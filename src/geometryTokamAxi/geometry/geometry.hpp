// SPDX-License-Identifier: MIT
#pragma once
#include <ddc/ddc.hpp>
#include <ddc/kernels/splines.hpp>

#include "ddc_alias_inline_functions.hpp"
#include "ddc_aliases.hpp"
#include "ddc_helper.hpp"
#include "species_info.hpp"

/*
 * @file geometry.hpp
 *
 */

/**
 * @brief Define non periodic real R dimension.
 */
struct R
{
    /**
     * @brief Define periodicity of the dimension.
     * Here, not periodic.
     */
    static bool constexpr PERIODIC = false;
};
/**
 * @brief Define periodic real Theta dimension.
 */
struct Theta
{
    /**
     * @brief Define periodicity of the dimension.
     * Here, periodic.
     */
    static bool constexpr PERIODIC = true;
};
/**
 * @brief Define non periodic parallel velocity @f$v_\parallel@f$.
 */
struct Vpar
{
    /**
     * The periodicity of the parallel velocity.
     * This is a compile time constant which allows the code to avoid
     * unnecessary if conditions.
     */
    static bool constexpr PERIODIC = false;
};
/**
 * @brief Define non periodic magnetic momentum @f$\mu@f$.
 */
struct Mu
{
    /**
     * The periodicity of the magnetic momentum.
     * This is a compile time constant which allows the code to avoid
     * unnecessary if conditions.
     */
    static bool constexpr PERIODIC = false;
};

// Coord = position of a coordinate in the vector space
using CoordR = Coord<R>;
using CoordTheta = Coord<Theta>;
using CoordVpar = Coord<Vpar>;
using CoordMu = Coord<Mu>;

// Splines definition
int constexpr BSDegreeR = 3;
int constexpr BSDegreeTheta = 3;
int constexpr BSDegreeVpar = 3;
int constexpr BSDegreeMu = 3;

bool constexpr BsplineOnUniformCellsR = true;
bool constexpr BsplineOnUniformCellsTheta = true;
bool constexpr BsplineOnUniformCellsVpar = true;
bool constexpr BsplineOnUniformCellsMu = true;

struct BSplinesR
    : std::conditional_t<
              BsplineOnUniformCellsR,
              ddc::UniformBSplines<R, BSDegreeR>,
              ddc::NonUniformBSplines<R, BSDegreeR>>
{
};
struct BSplinesTheta
    : std::conditional_t<
              BsplineOnUniformCellsTheta,
              ddc::UniformBSplines<Theta, BSDegreeTheta>,
              ddc::NonUniformBSplines<Theta, BSDegreeTheta>>
{
};
struct BSplinesVpar
    : std::conditional_t<
              BsplineOnUniformCellsVpar,
              ddc::UniformBSplines<Vpar, BSDegreeVpar>,
              ddc::NonUniformBSplines<Vpar, BSDegreeVpar>>
{
};
struct BSplinesMu
    : std::conditional_t<
              BsplineOnUniformCellsMu,
              ddc::UniformBSplines<Mu, BSDegreeMu>,
              ddc::NonUniformBSplines<Mu, BSDegreeMu>>
{
};
ddc::BoundCond constexpr SplineRBoundary = ddc::BoundCond::HERMITE;
ddc::BoundCond constexpr SplineThetaBoundary = ddc::BoundCond::PERIODIC;
ddc::BoundCond constexpr SplineVparBoundary = ddc::BoundCond::HERMITE;
ddc::BoundCond constexpr SplineMuBoundary = ddc::BoundCond::HERMITE;

using SplineInterpPointsR
        = ddc::GrevilleInterpolationPoints<BSplinesR, SplineRBoundary, SplineRBoundary>;
using SplineInterpPointsTheta
        = ddc::GrevilleInterpolationPoints<BSplinesTheta, SplineThetaBoundary, SplineThetaBoundary>;
using SplineInterpPointsVpar
        = ddc::GrevilleInterpolationPoints<BSplinesVpar, SplineVparBoundary, SplineVparBoundary>;
using SplineInterpPointsMu
        = ddc::GrevilleInterpolationPoints<BSplinesMu, SplineMuBoundary, SplineMuBoundary>;

struct GridR : SplineInterpPointsR::interpolation_discrete_dimension_type
{
};
struct GridTheta : SplineInterpPointsTheta::interpolation_discrete_dimension_type
{
};
struct GridVpar : SplineInterpPointsVpar::interpolation_discrete_dimension_type
{
};
struct GridMu : SplineInterpPointsMu::interpolation_discrete_dimension_type
{
};

using SplineVparBuilder_1d = ddc::SplineBuilder<
        Kokkos::DefaultHostExecutionSpace,
        Kokkos::DefaultHostExecutionSpace::memory_space,
        BSplinesVpar,
        GridVpar,
        SplineVparBoundary,
        SplineVparBoundary,
        ddc::SplineSolver::LAPACK,
        GridVpar>;

// Idx = index of the point in the point sampling
using IdxR = Idx<GridR>;
using IdxTheta = Idx<GridTheta>;
using IdxVpar = Idx<GridVpar>;
using IdxMu = Idx<GridMu>;
using IdxTor2D = Idx<GridR, GridTheta>;
using IdxV2D = Idx<GridVpar, GridMu>;
using IdxV2DTor2D = Idx<GridVpar, GridMu, GridR, GridTheta>;
using IdxSpTor2D = Idx<Species, GridR, GridTheta>;
using IdxSpV2D = Idx<Species, GridVpar, GridMu>;
using IdxSpV2DTor2D = Idx<Species, GridVpar, GridMu, GridR, GridTheta>;

// IdxStep = number of grid points between points in a sampling
using IdxStepR = IdxStep<GridR>;
using IdxStepTheta = IdxStep<GridTheta>;
using IdxStepVpar = IdxStep<GridVpar>;
using IdxStepMu = IdxStep<GridMu>;
using IdxStepTor2D = IdxStep<GridR, GridTheta>;
using IdxStepV2D = IdxStep<GridVpar, GridMu>;
using IdxStepV2DTor2D = IdxStep<GridVpar, GridMu, GridR, GridTheta>;
using IdxStepSpTor2D = IdxStep<Species, GridR, GridTheta>;
using IdxStepSpV2D = IdxStep<Species, GridVpar, GridMu>;
using IdxStepSpV2DTor2D = IdxStep<Species, GridVpar, GridMu, GridR, GridTheta>;

// IdxRange = to describe the wole index range (or a sub-index range)
using IdxRangeR = IdxRange<GridR>;
using IdxRangeTheta = IdxRange<GridTheta>;
using IdxRangeVpar = IdxRange<GridVpar>;
using IdxRangeMu = IdxRange<GridMu>;
using IdxRangeTor2D = IdxRange<GridR, GridTheta>;
using IdxRangeV2D = IdxRange<GridVpar, GridMu>;
using IdxRangeV2DTor2D = IdxRange<GridVpar, GridMu, GridR, GridTheta>;
using IdxRangeSpTor2D = IdxRange<Species, GridR, GridTheta>;
using IdxRangeSpV2D = IdxRange<Species, GridVpar, GridMu>;
using IdxRangeSpV2DTor2D = IdxRange<Species, GridVpar, GridMu, GridR, GridTheta>;

// template for the fields
// --> For FieldMem template
template <class ElementType>
using FieldMemR = FieldMem<ElementType, IdxRangeR>;
using DFieldMemR = FieldMemR<double>;

template <class ElementType>
using FieldMemTheta = FieldMem<ElementType, IdxRangeTheta>;
using DFieldMemTheta = FieldMemTheta<double>;

template <class ElementType>
using FieldMemVpar = FieldMem<ElementType, IdxRangeVpar>;
using DFieldMemVpar = FieldMemVpar<double>;

template <class ElementType>
using FieldMemMu = FieldMem<ElementType, IdxRangeMu>;
using DFieldMemMu = FieldMemMu<double>;

template <class ElementType>
using FieldMemTor2D = FieldMem<ElementType, IdxRangeTor2D>;
using DFieldMemTor2D = FieldMemTor2D<double>;

template <class ElementType>
using FieldMemV2D = FieldMem<ElementType, IdxRangeV2D>;
using DFieldMemV2D = FieldMemV2D<double>;

template <class ElementType>
using FieldMemV2DTor2D = FieldMem<ElementType, IdxRangeV2DTor2D>;
using DFieldMemV2DTor2D = FieldMemV2DTor2D<double>;

template <class ElementType>
using FieldMemSpTor2D = FieldMem<ElementType, IdxRangeSpTor2D>;
using DFieldMemSpTor2D = FieldMemSpTor2D<double>;

template <class ElementType>
using FieldMemSpV2D = FieldMem<ElementType, IdxRangeSpV2D>;
using DFieldMemSpV2D = FieldMemSpV2D<double>;

template <class ElementType>
using FieldMemSpV2DTor2D = FieldMem<ElementType, IdxRangeSpV2DTor2D>;
using DFieldMemSpV2DTor2D = FieldMemSpV2DTor2D<double>;

// --> For Field template
template <class ElementType>
using FieldR = Field<ElementType, IdxRangeR>;
using DFieldR = FieldR<double>;

template <class ElementType>
using FieldTheta = Field<ElementType, IdxRangeTheta>;
using DFieldTheta = FieldTheta<double>;

template <class ElementType>
using FieldVpar = Field<ElementType, IdxRangeVpar>;
using DFieldVpar = FieldVpar<double>;

template <class ElementType>
using FieldMu = Field<ElementType, IdxRangeMu>;
using DFieldMu = FieldMu<double>;

template <class ElementType>
using FieldTor2D = Field<ElementType, IdxRangeTor2D>;
using DFieldTor2D = FieldTor2D<double>;

template <class ElementType>
using FieldV2D = Field<ElementType, IdxRangeV2D>;
using DFieldV2D = FieldV2D<double>;

template <class ElementType>
using FieldV2DTor2D = Field<ElementType, IdxRangeV2DTor2D>;
using DFieldV2DTor2D = FieldV2DTor2D<double>;

template <class ElementType>
using FieldSpTor2D = Field<ElementType, IdxRangeSpTor2D>;
using DFieldSpTor2D = FieldSpTor2D<double>;

template <class ElementType>
using FieldSpV2D = Field<ElementType, IdxRangeSpV2D>;
using DFieldSpV2D = FieldSpV2D<double>;

template <class ElementType>
using FieldSpV2DTor2D = Field<ElementType, IdxRangeSpV2DTor2D>;
using DFieldSpV2DTor2D = FieldSpV2DTor2D<double>;

// --> For ConstField template
template <class ElementType>
using ConstFieldR = ConstField<ElementType, IdxRangeR>;
using DConstFieldR = ConstFieldR<double>;

template <class ElementType>
using ConstFieldTheta = ConstField<ElementType, IdxRangeTheta>;
using DConstFieldTheta = ConstFieldTheta<double>;

template <class ElementType>
using ConstFieldVpar = ConstField<ElementType, IdxRangeVpar>;
using DConstFieldVpar = ConstFieldVpar<double>;

template <class ElementType>
using ConstFieldMu = ConstField<ElementType, IdxRangeMu>;
using DConstFieldMu = ConstFieldMu<double>;

template <class ElementType>
using ConstFieldTor2D = ConstField<ElementType, IdxRangeTor2D>;
using DConstFieldTor2D = ConstFieldTor2D<double>;

template <class ElementType>
using ConstFieldV2D = ConstField<ElementType, IdxRangeV2D>;
using DConstFieldV2D = ConstFieldV2D<double>;

template <class ElementType>
using ConstFieldV2DTor2D = ConstField<ElementType, IdxRangeV2DTor2D>;
using DConstFieldV2DTor2D = ConstFieldV2DTor2D<double>;

template <class ElementType>
using ConstFieldSpTor2D = ConstField<ElementType, IdxRangeSpTor2D>;
using DConstFieldSpTor2D = ConstFieldSpTor2D<double>;

template <class ElementType>
using ConstFieldSpV2D = ConstField<ElementType, IdxRangeSpV2D>;
using DConstFieldSpV2D = ConstFieldSpV2D<double>;

template <class ElementType>
using ConstFieldSpV2DTor2D = ConstField<ElementType, IdxRangeSpV2DTor2D>;
using DConstFieldSpV2DTor2D = ConstFieldSpV2DTor2D<double>;
