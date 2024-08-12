// SPDX-License-Identifier: MIT
/** @file trapezoid_quadrature.hpp
 * File providing quadrature coefficients via the trapezoidal method.
 */
#pragma once
#include <ddc/ddc.hpp>

#include "ddc_aliases.hpp"
#include "quadrature_coeffs_nd.hpp"

/**
 * @brief Get the trapezoid coefficients in 1D.
 *
 * Calculate the quadrature coefficients for the trapezoid method defined on the provided index range.
 *
 * @tparam ExecSpace Execution space, depends on Kokkos.
 *
 * @param[in] idx_range
 * 	The idx_range on which the quadrature will be carried out.
 *
 * @return The quadrature coefficients for the trapezoid method defined on the provided index range.
 */
template <class ExecSpace, class Grid1D>
FieldMem<double, IdxRange<Grid1D>, ddc::KokkosAllocator<double, typename ExecSpace::memory_space>>
trapezoid_quadrature_coefficients_1d(IdxRange<Grid1D> const& idx_range)
{
    DFieldMem<IdxRange<Grid1D>, ddc::KokkosAllocator<double, typename ExecSpace::memory_space>>
            coefficients_alloc(idx_range);
    DField<IdxRange<Grid1D>,
           std::experimental::layout_right,
           typename ExecSpace::memory_space> const coefficients
            = get_field(coefficients_alloc);
    IdxRange<Grid1D> middle_domain = idx_range.remove(IdxStep<Grid1D>(1), IdxStep<Grid1D>(1));

    ddc::parallel_for_each(
            ExecSpace(),
            middle_domain,
            KOKKOS_LAMBDA(Idx<Grid1D> const idx) {
                coefficients(idx) = 0.5 * (distance_at_left(idx) + distance_at_right(idx));
            });
    double const dx_l = distance_at_left(idx_range.back());
    double const dx_r = distance_at_right(idx_range.front());
    Kokkos::parallel_for(
            "bounds",
            Kokkos::RangePolicy<ExecSpace>(0, 1),
            KOKKOS_LAMBDA(const int i) {
                coefficients(idx_range.front()) = 0.5 * dx_r;
                coefficients(idx_range.back()) = 0.5 * dx_l;
                if constexpr (Grid1D::continuous_dimension_type::PERIODIC) {
                    coefficients(idx_range.front()) += 0.5 * dx_l;
                    coefficients(idx_range.back()) += 0.5 * dx_r;
                }
            });

    return coefficients_alloc;
}

/**
 * @brief Get the trapezoid coefficients in ND.
 *
 * Calculate the quadrature coefficients for the trapezoid method defined on the provided index range.
 *
 * @tparam ExecSpace Execution space, depends on Kokkos.
 *
 * @param[in] idx_range
 * 	The idx_range on which the quadrature will be carried out.
 *
 * @return The quadrature coefficients for the trapezoid method defined on the provided idx_range.
 *         The allocation place (host or device ) will depend on the ExecSpace.
 */
template <class ExecSpace, class... ODims>
FieldMem<double, IdxRange<ODims...>, ddc::KokkosAllocator<double, typename ExecSpace::memory_space>>
trapezoid_quadrature_coefficients(IdxRange<ODims...> const& idx_range)
{
    return quadrature_coeffs_nd<ExecSpace, ODims...>(
            idx_range,
            (std::function<FieldMem<
                     double,
                     IdxRange<ODims>,
                     ddc::KokkosAllocator<double, typename ExecSpace::memory_space>>(
                     IdxRange<ODims>)>(trapezoid_quadrature_coefficients_1d<ExecSpace, ODims>))...);
}
