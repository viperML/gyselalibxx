// SPDX-License-Identifier: MIT
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <ddc/ddc.hpp>

#include <paraconf.h>
#include <pdi.h>

#include "CollisionSpVparMu.hpp"
#include "collisioninfo_radial.hpp"
#include "ddc_alias_inline_functions.hpp"
#include "geometry.hpp"
#include "input.hpp"
#include "paraconfpp.hpp"
#include "pdi_out.yml.hpp"
#include "simpson_quadrature.hpp"
#include "testcollisions.yaml.hpp"

using std::cerr;
using std::cout;
using std::endl;
using std::chrono::steady_clock;
namespace fs = std::filesystem;

int main(int argc, char** argv)
{
    long int iter_start;
    PC_tree_t conf_gyselax;
    parse_executable_arguments(conf_gyselax, iter_start, argc, argv, params_yaml);
    PC_tree_t conf_pdi = PC_parse_string(PDI_CFG);
    PC_errhandler(PC_NULL_HANDLER);
    PDI_init(conf_pdi);

    Kokkos::ScopeGuard scope(argc, argv);
    ddc::ScopeGuard ddc_scope(argc, argv);

    ddc::expose_to_pdi("iter_start", iter_start);

    // Input and output file names info
    std::string readRestartFileName(PCpp_string(conf_gyselax, ".InputFileNames.read_restart"));
    cout << "Input read restart: " << readRestartFileName << endl;
    std::string writeRestartFileName(PCpp_string(conf_gyselax, ".InputFileNames.write_restart"));
    cout << "Input write restart: " << writeRestartFileName << endl;
    int64_t read_restart_filename_size = readRestartFileName.size();
    int64_t write_restart_filename_size = writeRestartFileName.size();
    PDI_multi_expose(
            "restartFile",
            "read_restart_filename_size",
            &read_restart_filename_size,
            PDI_OUT,
            "read_restart_filename",
            readRestartFileName.c_str(),
            PDI_OUT,
            "write_restart_filename_size",
            &write_restart_filename_size,
            PDI_OUT,
            "write_restart_filename",
            writeRestartFileName.c_str(),
            PDI_OUT,
            NULL);

    std::vector<size_t> grid_tor1_extents(1);
    std::vector<size_t> grid_tor2_extents(1);
    std::vector<size_t> grid_tor3_extents(1);
    std::vector<size_t> grid_vpar_extents(1);
    std::vector<size_t> grid_mu_extents(1);
    std::vector<size_t> species_extents(1);
    std::vector<size_t> charges_extents(1);
    std::vector<size_t> masses_extents(1);
    PDI_multi_expose(
            "read_grid_extents",
            "grid_tor1_extents",
            grid_tor1_extents.data(),
            PDI_INOUT,
            "grid_tor2_extents",
            grid_tor2_extents.data(),
            PDI_INOUT,
            "grid_tor3_extents",
            grid_tor3_extents.data(),
            PDI_INOUT,
            "grid_vpar_extents",
            grid_vpar_extents.data(),
            PDI_INOUT,
            "grid_mu_extents",
            grid_mu_extents.data(),
            PDI_INOUT,
            "species_extents",
            species_extents.data(),
            PDI_INOUT,
            "charges_extents",
            charges_extents.data(),
            PDI_INOUT,
            "masses_extents",
            masses_extents.data(),
            PDI_INOUT,
            NULL);
    std::vector<double> grid_tor1(grid_tor1_extents[0]);
    std::vector<double> grid_tor2(grid_tor2_extents[0]);
    std::vector<double> grid_tor3(grid_tor3_extents[0]);
    std::vector<double> grid_vpar(grid_vpar_extents[0]);
    std::vector<double> grid_mu(grid_mu_extents[0]);
    std::vector<int> species(species_extents[0]);
    std::vector<double> charges(charges_extents[0]);
    std::vector<double> masses(masses_extents[0]);
    PDI_multi_expose(
            "read_grid",
            "grid_tor1",
            grid_tor1.data(),
            PDI_INOUT,
            "grid_tor2",
            grid_tor2.data(),
            PDI_INOUT,
            "grid_tor3",
            grid_tor3.data(),
            PDI_INOUT,
            "grid_vpar",
            grid_vpar.data(),
            PDI_INOUT,
            "grid_mu",
            grid_mu.data(),
            PDI_INOUT,
            "species",
            species.data(),
            PDI_INOUT,
            "charges",
            charges.data(),
            PDI_INOUT,
            "masses",
            masses.data(),
            PDI_INOUT,
            NULL);
    ddc::init_discrete_space<GridTor1>(grid_tor1);
    IdxRangeTor1 idx_range_tor1(IdxTor1(0), IdxStepTor1(grid_tor1.size()));
    ddc::init_discrete_space<GridTor2>(grid_tor2);
    IdxRangeTor2 idx_range_tor2(IdxTor2(0), IdxStepTor2(grid_tor2.size()));
    ddc::init_discrete_space<GridTor3>(grid_tor3);
    IdxRangeTor3 idx_range_tor3(IdxTor3(0), IdxStepTor3(grid_tor3.size()));
    ddc::init_discrete_space<GridVpar>(grid_vpar);
    IdxRangeVpar idx_range_vpar(IdxVpar(0), IdxStepVpar(grid_vpar.size()));
    ddc::init_discrete_space<GridMu>(grid_mu);
    IdxRangeMu idx_range_mu(IdxMu(0), IdxStepMu(grid_mu.size()));
    IdxStepSp const kinspecies(charges.size());
    IdxRangeSp const idx_range_kinsp(IdxSp(0), kinspecies);

    DConstFieldTor1 field_grid_tor1_host(grid_tor1.data(), idx_range_tor1);
    auto field_grid_tor1 = ddc::
            create_mirror_view_and_copy(Kokkos::DefaultExecutionSpace(), field_grid_tor1_host);
    DConstFieldTor2 field_grid_tor2_host(grid_tor2.data(), idx_range_tor2);
    auto field_grid_tor2 = ddc::
            create_mirror_view_and_copy(Kokkos::DefaultExecutionSpace(), field_grid_tor2_host);
    DConstFieldTor3 field_grid_tor3_host(grid_tor3.data(), idx_range_tor3);
    auto field_grid_tor3 = ddc::
            create_mirror_view_and_copy(Kokkos::DefaultExecutionSpace(), field_grid_tor3_host);
    DConstFieldVpar field_grid_vpar_host(grid_vpar.data(), idx_range_vpar);
    auto field_grid_vpar = ddc::
            create_mirror_view_and_copy(Kokkos::DefaultExecutionSpace(), field_grid_vpar_host);
    DConstFieldMu field_grid_mu_host(grid_mu.data(), idx_range_mu);
    auto field_grid_mu
            = ddc::create_mirror_view_and_copy(Kokkos::DefaultExecutionSpace(), field_grid_mu_host);
    host_t<ConstFieldSp<int>> field_species_host(species.data(), idx_range_kinsp);
    auto field_species
            = ddc::create_mirror_view_and_copy(Kokkos::DefaultExecutionSpace(), field_species_host);
    host_t<DConstFieldSp> field_charges_host(charges.data(), idx_range_kinsp);
    auto field_charges
            = ddc::create_mirror_view_and_copy(Kokkos::DefaultExecutionSpace(), field_charges_host);
    host_t<DConstFieldSp> field_masses_host(masses.data(), idx_range_kinsp);
    auto field_masses
            = ddc::create_mirror_view_and_copy(Kokkos::DefaultExecutionSpace(), field_masses_host);

    // Algorithm Info
    double const deltat = PCpp_double(conf_gyselax, ".Algorithm.deltat");
    int const nbiter = static_cast<int>(PCpp_int(conf_gyselax, ".Algorithm.nbiter"));

    // Output info
    double const time_diag = PCpp_double(conf_gyselax, ".Output.time_diag");
    int const nbstep_diag = int(time_diag / deltat);

    cout << "nbiter = " << nbiter << " nbstep_diag = " << nbstep_diag << endl;

    // Poloidal cross-section of the 3 moments: density, temperature and Upar
    IdxRangeSpTorCS const idx_range_sp_torCS(idx_range_kinsp, idx_range_tor2, idx_range_tor1);
    DFieldSpTorCS_host density_torCS_host(idx_range_sp_torCS);
    DFieldSpTorCS_host temperature_torCS_host(idx_range_sp_torCS);
    DFieldSpTorCS_host Upar_torCS_host(idx_range_sp_torCS);
    ddc::PdiEvent("read_profiles")
            .with("densityTorCS", density_torCS_host)
            .with("temperatureTorCS", temperature_torCS_host)
            .with("UparTorCS", Upar_torCS_host);

    // fdistribu
    IdxRangeSpTor3DV2D const idx_range_sp_tor3D_v2D(
            idx_range_kinsp,
            idx_range_tor3,
            idx_range_tor2,
            idx_range_tor1,
            idx_range_vpar,
            idx_range_mu);
    DFieldSpTor3DV2D_host allfdistribu_host(idx_range_sp_tor3D_v2D);
    double time_saved;
    ddc::PdiEvent("read_fdistribu")
            .with("time_saved", time_saved)
            .and_with("fdistribu", allfdistribu_host);
    cout << "Reading of time " << time_saved << endl;
    auto allfdistribu_alloc = ddc::create_mirror_view_and_copy(
            Kokkos::DefaultExecutionSpace(),
            get_field(allfdistribu_host));
    auto allfdistribu = get_field(allfdistribu_alloc);

    // Collision operator initialisation
    double const nustar0_rpeak = 1.;
    std::int8_t const collisions_interspecies = true;
    double const rpeak = 1.;
    double const q_rpeak = 1.;
    DFieldMemTor1 safety_factor(idx_range_tor1);
    ddc::parallel_fill(safety_factor, 1.0); //ATTENTION: Must be changed
    IdxRangeTorCS idx_range_tor1_tor2(idx_range_tor2, idx_range_tor1);
    DFieldMemTorCS B_norm(idx_range_tor1_tor2);
    ddc::parallel_fill(B_norm, 1.0);

    DFieldMemVpar const coeff_intdvpar
            = simpson_quadrature_coefficients_1d<Kokkos::DefaultExecutionSpace>(
                    get_idx_range<GridVpar>(allfdistribu));
    DFieldMemMu const coeff_intdmu
            = simpson_quadrature_coefficients_1d<Kokkos::DefaultExecutionSpace>(
                    get_idx_range<GridMu>(allfdistribu));

    CollisionInfoRadial<GridTor1> collision_info(
            nustar0_rpeak,
            collisions_interspecies,
            rpeak,
            q_rpeak,
            get_const_field(field_grid_tor1),
            get_const_field(safety_factor));
    CollisionSpVparMu<
            CollisionInfoRadial<GridTor1>,
            IdxRangeSpTor3DV2D,
            GridVpar,
            GridMu,
            DConstFieldTorCS>
            collision_operator(
                    collision_info,
                    idx_range_sp_tor3D_v2D,
                    get_const_field(coeff_intdmu),
                    get_const_field(coeff_intdvpar),
                    get_const_field(B_norm));

    steady_clock::time_point const start = steady_clock::now();

    collision_operator(allfdistribu, deltat);

    long int iter_saved(iter_start + 1);
    int iter = 1;
    time_saved = time_saved + iter * deltat;
    cout << "iter_saved = " << iter_saved << " ; time_saved = " << time_saved << endl;
    ddc::parallel_deepcopy(allfdistribu_host, allfdistribu);
    ddc::PdiEvent("write_restart")
            .with("iter_saved", iter_saved)
            .with("time_saved", time_saved)
            .with("grid_tor1", field_grid_tor1_host)
            .with("grid_tor2", field_grid_tor2_host)
            .with("grid_tor3", field_grid_tor3_host)
            .with("grid_vpar", field_grid_vpar_host)
            .with("grid_mu", field_grid_mu_host)
            .with("species", field_species_host)
            .with("masses", field_masses_host)
            .with("charges", field_charges_host)
            .with("densityTorCS", density_torCS_host)
            .with("temperatureTorCS", temperature_torCS_host)
            .with("UparTorCS", Upar_torCS_host)
            .with("fdistribu", allfdistribu_host);

    steady_clock::time_point const end = steady_clock::now();

    double const simulation_time = std::chrono::duration<double>(end - start).count();
    std::cout << "Simulation time: " << simulation_time << "s\n";

    PDI_finalize();

    PC_tree_destroy(&conf_pdi);

    PC_tree_destroy(&conf_gyselax);

    return EXIT_SUCCESS;
}
