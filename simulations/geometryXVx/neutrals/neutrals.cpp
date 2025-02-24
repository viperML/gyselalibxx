// SPDX-License-Identifier: MIT
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <ddc/ddc.hpp>
#include <ddc/kernels/splines.hpp>

#include <paraconf.h>
#include <pdi.h>

#include "bsl_advection_vx.hpp"
#include "bsl_advection_x.hpp"
#include "charge_exchange.hpp"
#include "chargedensitycalculator.hpp"
#include "collisions_inter.hpp"
#include "collisions_intra.hpp"
#include "constantfluidinitialization.hpp"
#include "constantrate.hpp"
#include "ddc_alias_inline_functions.hpp"
#include "diffusiveneutralsolver.hpp"
#include "fem_1d_poisson_solver.hpp"
#include "fft_poisson_solver.hpp"
#include "geometry.hpp"
#include "input.hpp"
#include "ionization.hpp"
#include "irighthandside.hpp"
#include "kinetic_fluid_coupling_source.hpp"
#include "kinetic_source.hpp"
#include "krook_source_adaptive.hpp"
#include "krook_source_constant.hpp"
#include "maxwellianequilibrium.hpp"
#include "neumann_spline_quadrature.hpp"
#include "neutrals.yml.hpp"
#include "output.hpp"
#include "paraconfpp.hpp"
#include "pdi_out_neutrals.yml.hpp"
#include "predcorr_hybrid.hpp"
#include "qnsolver.hpp"
#include "recombination.hpp"
#include "restartinitialization.hpp"
#include "singlemodeperturbinitialization.hpp"
#include "species_info.hpp"
#include "species_init.hpp"
#include "spline_interpolator.hpp"
#include "splitrighthandsidesolver.hpp"
#include "splitvlasovsolver.hpp"

using std::cerr;
using std::endl;
using std::chrono::steady_clock;
namespace fs = std::filesystem;

int main(int argc, char** argv)
{
    // Environments variables for profiling
    setenv("KOKKOS_TOOLS_LIBS", KP_KERNEL_TIMER_PATH, false);
    setenv("KOKKOS_TOOLS_TIMER_JSON", "true", false);

    long int iter_start;
    PC_tree_t conf_voicexx;
    parse_executable_arguments(conf_voicexx, iter_start, argc, argv, params_yaml);
    PC_tree_t conf_pdi = PC_parse_string(PDI_CFG);
    PC_errhandler(PC_NULL_HANDLER);
    PDI_init(conf_pdi);

    Kokkos::ScopeGuard kokkos_scope(argc, argv);
    ddc::ScopeGuard ddc_scope(argc, argv);

    // Reading config
    // --> Mesh info
    IdxRangeX const mesh_x = init_spline_dependent_idx_range<
            GridX,
            BSplinesX,
            SplineInterpPointsX>(conf_voicexx, "x");
    IdxRangeVx const mesh_vx = init_spline_dependent_idx_range<
            GridVx,
            BSplinesVx,
            SplineInterpPointsVx>(conf_voicexx, "vx");
    IdxRangeXVx const meshXVx(mesh_x, mesh_vx);

    SplineXBuilder const builder_x(meshXVx);
#ifndef PERIODIC_RDIMX
    SplineXBuilder_1d const builder_x_poisson(mesh_x);
#endif
    SplineVxBuilder const builder_vx(meshXVx);
    SplineVxBuilder_1d const builder_vx_poisson(mesh_vx);

    IdxRangeSp idx_range_kinsp;
    IdxRangeSp idx_range_fluidsp;
    init_species_withfluid(idx_range_kinsp, idx_range_fluidsp, conf_voicexx);

    // Initialization of kinetic species distribution function
    IdxRangeSpVx const meshSpVx(idx_range_kinsp, mesh_vx);
    DFieldMemSpVx allfequilibrium(meshSpVx);
    MaxwellianEquilibrium const init_fequilibrium
            = MaxwellianEquilibrium::init_from_input(idx_range_kinsp, conf_voicexx);
    init_fequilibrium(allfequilibrium);

    ddc::expose_to_pdi("iter_start", iter_start);

    IdxRangeSpXVx const meshSpXVx(idx_range_kinsp, meshXVx);
    DFieldMemSpXVx allfdistribu(meshSpXVx);
    double time_start(0);
    if (iter_start == 0) {
        SingleModePerturbInitialization const init = SingleModePerturbInitialization::
                init_from_input(allfequilibrium, idx_range_kinsp, conf_voicexx);
        init(allfdistribu);
    } else {
        RestartInitialization const restart(iter_start, time_start);
        restart(allfdistribu);
    }
    auto allfequilibrium_host = ddc::create_mirror_view_and_copy(get_field(allfequilibrium));

    // Moments index range initialization
    IdxStepMom const nb_fluid_moments(1);
    IdxRangeMom const meshM(IdxMom(0), nb_fluid_moments);
    ddc::init_discrete_space<GridMom>();

    // Neutral species initialization
    DFieldMemSpMomX neutrals_alloc(IdxRangeSpMomX(idx_range_fluidsp, meshM, mesh_x));
    auto neutrals = get_field(neutrals_alloc);
    host_t<DFieldMemSpMom> moments_init_host(IdxRangeSpMom(idx_range_fluidsp, meshM));

    for (IdxSp const isp : idx_range_fluidsp) {
        PC_tree_t const conf_nisp = PCpp_get(
                conf_voicexx,
                ".NeutralSpeciesInfo[%d]",
                (isp - idx_range_fluidsp.front()).value());
        ddc::parallel_fill(moments_init_host[isp], PCpp_double(conf_nisp, ".density_eq"));
    }
    ConstantFluidInitialization fluid_init(get_const_field(moments_init_host));
    fluid_init(neutrals);

    // --> Algorithm info
    double const deltat = PCpp_double(conf_voicexx, ".Algorithm.deltat");
    int const nbiter = static_cast<int>(PCpp_int(conf_voicexx, ".Algorithm.nbiter"));

    // --> Output info
    double const time_diag = PCpp_double(conf_voicexx, ".Output.time_diag");
    int const nbstep_diag = int(time_diag / deltat);

#ifdef PERIODIC_RDIMX
    ddc::PeriodicExtrapolationRule<X> bv_x_min;
    ddc::PeriodicExtrapolationRule<X> bv_x_max;
#else
    ddc::ConstantExtrapolationRule<X> bv_x_min(ddc::coordinate(mesh_x.front()));
    ddc::ConstantExtrapolationRule<X> bv_x_max(ddc::coordinate(mesh_x.back()));
#endif

    ddc::ConstantExtrapolationRule<Vx> bv_vx_min(ddc::coordinate(mesh_vx.front()));
    ddc::ConstantExtrapolationRule<Vx> bv_vx_max(ddc::coordinate(mesh_vx.back()));

    // Creating operators
    SplineXEvaluator const spline_x_evaluator(bv_x_min, bv_x_max);
    SplineVxEvaluator const spline_vx_evaluator(bv_vx_min, bv_vx_max);
#ifndef PERIODIC_RDIMX
    SplineXEvaluator_1d const spline_x_evaluator_poisson(bv_x_min, bv_x_max);
#endif
    PreallocatableSplineInterpolator const spline_x_interpolator(builder_x, spline_x_evaluator);
    PreallocatableSplineInterpolator const spline_vx_interpolator(builder_vx, spline_vx_evaluator);

    BslAdvectionSpatial<GeometryXVx, GridX> const advection_x(spline_x_interpolator);
    BslAdvectionVelocity<GeometryXVx, GridVx> const advection_vx(spline_vx_interpolator);

    // list of rhs operators
    std::vector<std::reference_wrapper<IRightHandSide const>> rhs_operators;
    std::vector<KrookSourceConstant> krook_source_constant_vector;
    std::vector<KrookSourceAdaptive> krook_source_adaptive_vector;
    // Krook operators initialization
    int const nb_rhsKrook(PCpp_len(conf_voicexx, ".Krook"));
    for (int ik = 0; ik < nb_rhsKrook; ++ik) {
        // --> Krook info
        PC_tree_t const conf_krook = PCpp_get(conf_voicexx, ".Krook[%d]", ik);

        static std::map<std::string, RhsType>
                str2rhstype {{"source", RhsType::Source}, {"sink", RhsType::Sink}};
        RhsType type = str2rhstype[PCpp_string(conf_krook, ".type")];
        std::string const krook_name = PCpp_string(conf_krook, ".name");
        if (krook_name == "constant") {
            krook_source_constant_vector.emplace_back(
                    mesh_x,
                    mesh_vx,
                    type,
                    PCpp_double(conf_krook, ".extent"),
                    PCpp_double(conf_krook, ".stiffness"),
                    PCpp_double(conf_krook, ".amplitude"),
                    PCpp_double(conf_krook, ".density"),
                    PCpp_double(conf_krook, ".temperature"));
            rhs_operators.emplace_back(krook_source_constant_vector.back());

        } else if (krook_name == "adaptive") {
            krook_source_adaptive_vector.emplace_back(
                    mesh_x,
                    mesh_vx,
                    type,
                    PCpp_double(conf_krook, ".extent"),
                    PCpp_double(conf_krook, ".stiffness"),
                    PCpp_double(conf_krook, ".amplitude"),
                    PCpp_double(conf_krook, ".density"),
                    PCpp_double(conf_krook, ".temperature"));
            rhs_operators.emplace_back(krook_source_adaptive_vector.back());
        } else {
            throw std::invalid_argument(
                    "Invalid krook name, allowed values are: 'constant', or 'adaptive'.");
        }
    }

    // Kinetic source
    KineticSource const rhs_kinetic_source(
            mesh_x,
            mesh_vx,
            PCpp_double(conf_voicexx, ".KineticSource.extent"),
            PCpp_double(conf_voicexx, ".KineticSource.stiffness"),
            PCpp_double(conf_voicexx, ".KineticSource.amplitude"),
            PCpp_double(conf_voicexx, ".KineticSource.density"),
            PCpp_double(conf_voicexx, ".KineticSource.energy"),
            PCpp_double(conf_voicexx, ".KineticSource.temperature"));
    rhs_operators.emplace_back(rhs_kinetic_source);


    CollisionsIntra const
            collisions_intra(meshSpXVx, PCpp_double(conf_voicexx, ".CollisionsInfo.nustar0"));
    rhs_operators.emplace_back(collisions_intra);

    std::optional<CollisionsInter> collisions_inter;
    if (PCpp_bool(conf_voicexx, ".CollisionsInfo.enable_inter")) {
        collisions_inter.emplace(meshSpXVx, PCpp_double(conf_voicexx, ".CollisionsInfo.nustar0"));
        rhs_operators.emplace_back(*collisions_inter);
    }
    SplitVlasovSolver const vlasov(advection_x, advection_vx);
    SplitRightHandSideSolver const boltzmann(vlasov, rhs_operators);

    DFieldMemVx const quadrature_coeffs_alloc(
            neumann_spline_quadrature_coefficients<
                    Kokkos::DefaultExecutionSpace>(mesh_vx, builder_vx_poisson));
    ChargeDensityCalculator rhs(get_const_field(quadrature_coeffs_alloc));
#ifdef PERIODIC_RDIMX
    FFTPoissonSolver<IdxRangeX, IdxRangeX, Kokkos::DefaultExecutionSpace> poisson_solver(mesh_x);
#else
    FEM1DPoissonSolver poisson_solver(builder_x_poisson, spline_x_evaluator_poisson);
#endif
    QNSolver const poisson(poisson_solver, rhs);

    double const normalization_coeff
            = PCpp_double(conf_voicexx, ".DiffusiveNeutralSolver.normalization_coeff_neutrals");
    double const norm_coeff_rate
            = PCpp_double(conf_voicexx, ".DiffusiveNeutralSolver.norm_coeff_rate_neutrals");

    // The CX coefficient needs to be first constructed in order to write a correct initstate file. Check pdi_out_neutrals.yml.hpp for a closer look.
    ChargeExchangeRate charge_exchange(norm_coeff_rate);
    IonizationRate ionization(norm_coeff_rate);
    RecombinationRate recombination(norm_coeff_rate);

    SplineXBuilder_1d const spline_x_builder_neutrals(mesh_x);
    SplineXEvaluator_1d const spline_x_evaluator_neutrals(bv_x_min, bv_x_max);

    DFieldMemVx const quadrature_coeffs_neutrals(
            trapezoid_quadrature_coefficients<Kokkos::DefaultExecutionSpace>(mesh_vx));

    DiffusiveNeutralSolver const neutralsolver(
            charge_exchange,
            ionization,
            recombination,
            normalization_coeff,
            spline_x_builder_neutrals,
            spline_x_evaluator_neutrals,
            get_const_field(quadrature_coeffs_neutrals));

    KineticFluidCouplingSource const kineticfluidcoupling(
            PCpp_double(conf_voicexx, ".KineticFluidCouplingSource.density_coupling_coeff"),
            PCpp_double(conf_voicexx, ".KineticFluidCouplingSource.momentum_coupling_coeff"),
            PCpp_double(conf_voicexx, ".KineticFluidCouplingSource.energy_coupling_coeff"),
            ionization,
            recombination,
            normalization_coeff,
            get_const_field(quadrature_coeffs_alloc));

    PredCorrHybrid const predcorr(boltzmann, neutralsolver, poisson, kineticfluidcoupling);

    // Starting the code
    ddc::expose_to_pdi("Nx_spline_cells", ddc::discrete_space<BSplinesX>().ncells());
    ddc::expose_to_pdi("Nvx_spline_cells", ddc::discrete_space<BSplinesVx>().ncells());
    expose_mesh_to_pdi("MeshX", mesh_x);
    expose_mesh_to_pdi("MeshVx", mesh_vx);
    ddc::expose_to_pdi("Lx", ddcHelper::total_interval_length(mesh_x));
    ddc::expose_to_pdi("nbstep_diag", nbstep_diag);
    ddc::expose_to_pdi("Nkinspecies", idx_range_kinsp.size());
    ddc::expose_to_pdi(
            "fdistribu_charges",
            ddc::discrete_space<Species>().charges()[idx_range_kinsp]);
    ddc::expose_to_pdi(
            "fdistribu_masses",
            ddc::discrete_space<Species>().masses()[idx_range_kinsp]);
    ddc::expose_to_pdi(
            "neutrals_masses",
            ddc::discrete_space<Species>().masses()[idx_range_fluidsp]);
    ddc::expose_to_pdi("normalization_coeff_neutrals", normalization_coeff);
    ddc::expose_to_pdi("norm_coeff_rate_neutrals", norm_coeff_rate);
    ddc::PdiEvent("initial_state").with("fdistribu_eq", allfequilibrium_host);

    steady_clock::time_point const start = steady_clock::now();

    predcorr(allfdistribu, neutrals, time_start, deltat, nbiter);

    steady_clock::time_point const end = steady_clock::now();

    double const simulation_time = std::chrono::duration<double>(end - start).count();
    std::cout << "Simulation time: " << simulation_time << "s\n";

    PC_tree_destroy(&conf_pdi);

    PDI_finalize();

    PC_tree_destroy(&conf_voicexx);

    return EXIT_SUCCESS;
}
