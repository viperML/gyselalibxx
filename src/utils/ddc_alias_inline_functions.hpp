// SPDX-License-Identifier: MIT
#pragma once
#include <type_traits>
#include <utility>

#include <ddc/ddc.hpp>

#include <sll/polar_spline.hpp>

#include "derivative_field_common.hpp"
#include "vector_field_common.hpp"

namespace detail {

/*
 * A class using template magic to determine if a type has an idx_range function.
 */
template <typename Type>
class HasIdxRange
{
private:
    // Class for SFINAE deduction
    template <typename U>
    class check
    {
    };

    // Function that will be chosen if ClassType has a method called idx_range with 0 arguments
    template <typename ClassType>
    static char f(check<decltype(std::declval<ClassType>().idx_range())>*);

    // Function that will be chosen by default
    template <typename ClassType>
    static long f(...);

public:
    static constexpr bool value = (sizeof(f<Type>(0)) == sizeof(char));
};

/*
 * A class using template magic to determine if a type is a kind of Field defined in Gyselalibxx.
 * Such a field should define a function get_const_field.
 */
template <typename Type>
class IsGslxField
{
private:
    // Class for SFINAE deduction
    template <typename U>
    class check
    {
    };

    // Function that will be chosen if ClassType has a method called get_const_field with 0 arguments
    template <typename ClassType>
    static char f(check<decltype(std::declval<ClassType>().get_const_field())>*);

    // Function that will be chosen by default
    template <typename ClassType>
    static long f(...);

public:
    static constexpr bool value = (sizeof(f<Type>(0)) == sizeof(char));
};

} // namespace detail

template <typename Type>
static constexpr bool has_idx_range_v = detail::HasIdxRange<Type>::value;

template <typename Type>
static constexpr bool is_gslx_field_v = detail::IsGslxField<Type>::value;

/**
 * A function to get the range of valid indices that can be used to index this field.
 *
 * @param[in] field The field whose indices are of interest.
 *
 * @returns The index range.
 */
template <class... QueryGrids, class FieldType>
KOKKOS_INLINE_FUNCTION auto get_idx_range(FieldType const& field) noexcept
{
    static_assert(
            ddc::is_chunk_v<FieldType> || has_idx_range_v<FieldType>,
            "Not a DDC field (ddc::ChunkSpan) type");
    if constexpr (ddc::is_chunk_v<FieldType>) {
        if constexpr (sizeof...(QueryGrids) == 0) {
            return field.domain();
        } else {
            return field.template domain<QueryGrids...>();
        }
    } else {
        if constexpr (sizeof...(QueryGrids) == 0) {
            return field.idx_range();
        } else {
            return field.template idx_range<QueryGrids...>();
        }
    }
}

/**
 * A function to get the range of valid indices that can be used to index a set of b-splines that are compatible with this spline builder.
 *
 * @param[in] builder The spline builder.
 *
 * @returns The index range.
 */
template <class SplineBuilder>
inline auto get_spline_idx_range(SplineBuilder const& builder) noexcept
{
    return builder.spline_domain();
}

/**
 * A helper function to get a modifiable field from a FieldMem without allocating additional memory.
 *
 * @param[in] field The field memory object.
 * @returns The modifiable field.
 */
template <class FieldType>
inline auto get_field(FieldType&& field)
{
    using Type = std::remove_cv_t<std::remove_reference_t<FieldType>>;
    static_assert(
            (ddc::is_chunk_v<Type>) || (is_field_v<Type>) || (is_deriv_field_v<Type>)
                    || (is_polar_spline_v<Type>),
            "Not a Field or FieldMem (ddc::Chunk or ddc::ChunkSpan) type");
    return field.span_view();
}

/**
 * A helper function to get a constant field from a FieldMem without allocating additional memory.
 *
 * @param[in] field The field memory object.
 * @returns The constant field.
 */
template <class FieldType>
inline auto get_const_field(FieldType&& field)
{
    using Type = std::remove_cv_t<std::remove_reference_t<FieldType>>;
    static_assert(
            (ddc::is_chunk_v<Type>) || (is_field_v<Type>) || (is_deriv_field_v<Type>)
                    || (is_polar_spline_v<Type>) || (is_gslx_field_v<Type>),
            "Not a Field or FieldMem (ddc::Chunk or ddc::ChunkSpan) type");
    return field.span_cview();
}
