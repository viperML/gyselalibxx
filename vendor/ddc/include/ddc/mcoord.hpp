#pragma once

#include <cstddef>
#include <utility>

#include <experimental/mdspan>

#include "ddc/taggedvector.hpp"

/** A MCoordElement is a scalar that represents the coordinate of a mesh point in one mesh axis.
 */
using MCoordElement = std::size_t;

/** A MLengthElement is a scalar that represents the difference between two coordinates.
 */
using MLengthElement = std::ptrdiff_t;

/** A MCoord is a coordinate (a vector) in the mesh.
 * 
 * Each coordinate is tagged by its associated dimension.
 */
template <class... Tags>
using MCoord = TaggedVector<MCoordElement, Tags...>;

template <class... Tags>
using MLength = TaggedVector<MLengthElement, Tags...>;


namespace detail {

template <class, class>
struct ExtentToMCoord;

template <class MCoordType, std::size_t... Ints>
struct ExtentToMCoord<MCoordType, std::index_sequence<Ints...>>
{
    static_assert(MCoordType::size() == sizeof...(Ints));

    template <std::size_t... Extents>
    static inline constexpr MCoordType mcoord(
            const std::experimental::extents<Extents...>& extent) noexcept
    {
        return MCoordType(extent.extent(Ints)...);
    }
};

} // namespace detail

template <class MCoordType>
struct ExtentToMCoordEnd
    : detail::ExtentToMCoord<MCoordType, std::make_index_sequence<MCoordType::size()>>
{
};

template <class... Tags, class Extents>
inline constexpr MCoord<Tags...> mcoord_end(Extents extent) noexcept
{
    return ExtentToMCoordEnd<MCoord<Tags...>>::mcoord(extent);
}