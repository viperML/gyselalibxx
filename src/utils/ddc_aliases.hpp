#pragma once

#include <ddc/ddc.hpp>

/**
 * This file contains aliases for DDC. The documentation for DDC can be found at <https://ddc.mdls.fr/>.
 * The names used in the DDC project are not always intuitive for mathematicians/physicists therefore
 * Gysela has chosen to introduce this file to provide names that are hopefully more intuitive.
 * The documentation for these concepts can be found in the Gysela documentation at:
 * <https://gyselax.github.io/gyselalibxx/docs_DDC_in_gyselalibxx.html>
 */

/// An alias describing the type of a coordinate (e.g. a coordinate in phase-space (x, vx)).
template <class... Dims>
using Coord = ddc::Coordinate<Dims...>;

/// An alias describing the type of an index that is used to access the values of a field defined on a grid.
template <class... GridTypes>
using Idx = ddc::DiscreteElement<GridTypes...>;

/// An alias describing the type of a distance between two indexes.
template <class... GridTypes>
using IdxStep = ddc::DiscreteVector<GridTypes...>;

/// An alias describing the type of an index range describing the subsection of a grid on which a field is defined.
template <class... GridTypes>
using IdxRange = ddc::DiscreteDomain<GridTypes...>;

/// An alias describing the type of an object which will allocate memory for a field when it is created.
template <
        class ElementType,
        class IdxRange,
        class Allocator
        = ddc::KokkosAllocator<ElementType, Kokkos::DefaultExecutionSpace::memory_space>>
using FieldMem = ddc::Chunk<ElementType, IdxRange, Allocator>;

/// An alias describing the type of an object which will allocate memory for a field of doubles when it is created.
template <
        class IdxRange,
        class Allocator = ddc::KokkosAllocator<double, Kokkos::DefaultExecutionSpace::memory_space>>
using DFieldMem = FieldMem<double, IdxRange, Allocator>;

/// An alias describing the type of a field defined on a grid (e.g. the electric field defined on the grid @f${x_0, x_1, .., x_N}@f$)
template <
        class ElementType,
        class IdxRange,
        class LayoutStridedPolicy = std::experimental::layout_right,
        class MemorySpace = Kokkos::DefaultExecutionSpace::memory_space>
using Field = ddc::ChunkSpan<ElementType, IdxRange, LayoutStridedPolicy, MemorySpace>;

/// An alias describing the type of a field of doubles defined on a grid (e.g. the electric field defined on the grid @f${x_0, x_1, .., x_N}@f$)
template <
        class IdxRange,
        class LayoutStridedPolicy = std::experimental::layout_right,
        class MemorySpace = Kokkos::DefaultExecutionSpace::memory_space>
using DField = Field<double, IdxRange, LayoutStridedPolicy, MemorySpace>;

/// An alias describing the type of a constant field defined on a grid (e.g. the electric field defined on the grid @f${x_0, x_1, .., x_N}@f$)
template <
        class ElementType,
        class IdxRange,
        class LayoutStridedPolicy = std::experimental::layout_right,
        class MemorySpace = Kokkos::DefaultExecutionSpace::memory_space>
using ConstField = ddc::ChunkView<ElementType, IdxRange, LayoutStridedPolicy, MemorySpace>;

/// An alias describing the type of a constant field of doubles defined on a grid (e.g. the electric field defined on the grid @f${x_0, x_1, .., x_N}@f$)
template <
        class IdxRange,
        class LayoutStridedPolicy = std::experimental::layout_right,
        class MemorySpace = Kokkos::DefaultExecutionSpace::memory_space>
using DConstField = ConstField<double, IdxRange, LayoutStridedPolicy, MemorySpace>;

/// An alias describing the type from which a uniform grid must inherit.
template <class GridType>
using UniformGridBase = ddc::UniformPointSampling<GridType>;

/// An alias describing the type from which a non-uniform grid must inherit.
template <class GridType>
using NonUniformGridBase = ddc::NonUniformPointSampling<GridType>;
