// SPDX-License-Identifier: MIT

#pragma once
#include <ddc/ddc.hpp>

#include "ddc_aliases.hpp"
#include "vector_field_common.hpp"

/**
 * @brief Pre-declaration of VectorFieldMem.
 */
template <
        class ElementType,
        class IdxRangeType,
        class,
        class Allocator = ddc::HostAllocator<ElementType>>
class VectorFieldMem;


template <class ElementType, class IdxRangeType, class DimSeq, class Allocator>
inline constexpr bool
        enable_field<VectorFieldMem<ElementType, IdxRangeType, DimSeq, Allocator>> = true;

/**
 * @brief Pre-declaration of VectorField.
 */
template <class, class, class, class, class>
class VectorField;

/**
 * @brief A class which describes the storage for a vector field.
 *
 * A class which describes the storage for a vector field. In other words a class which maps a position
 * on an index range to a vector (x,y,z,...). This is done by storing the values at the positions in
 * individual FieldMems.
 *
 * @tparam ElementType The data type of a scalar element of the vector field.
 * @tparam IdxRangeType
 * @tparam NDTag A NDTag describing the dimensions described by the scalar elements of a vector field element.
 * @tparam Allocator The type describing how/where memory is allocated. See DDC.
 */
template <class ElementType, class IdxRangeType, class NDTag, class Allocator>
class VectorFieldMem
    : public VectorFieldCommon<FieldMem<ElementType, IdxRangeType, Allocator>, NDTag>
{
public:
    /**
     * @brief Type describing the object which can be extracted from this VectorFieldMem using the get<> function.
     *
     * This is a DDC keyword used to make this class interchangeable with Field.
     * In DDC FieldMem types are referred to as Chunk types and Field types are
     * referred to as ChunkSpan/ChunkView.
     */
    using chunk_type = FieldMem<ElementType, IdxRangeType, Allocator>;

private:
    using base_type = VectorFieldCommon<chunk_type, NDTag>;

public:
    /// The type of an element in one of the FieldMems comprising the VectorFieldMem
    using element_type = typename base_type::element_type;

public:
    /**
     * @brief A type which can hold a reference to this VectorFieldMem.
     *
     * This is a DDC keyword used to make this class interchangeable with Field.
     */
    using span_type = VectorField<
            ElementType,
            IdxRangeType,
            NDTag,
            std::experimental::layout_right,
            typename Allocator::memory_space>;

    /**
     * @brief A type which can hold a constant reference to this VectorFieldMem.
     *
     * This is a DDC keyword used to make this class interchangeable with Field.
     */
    using view_type = VectorField<
            const ElementType,
            IdxRangeType,
            NDTag,
            std::experimental::layout_right,
            typename Allocator::memory_space>;

    /**
     * @brief The type of the index range on which the field is defined.
     * This is a DDC keyword used to make this class interchangeable with Field.
     * In DDC IdxRange types are referred to as DiscreteDomain types.
     */
    using discrete_domain_type = typename base_type::discrete_domain_type;
    /// @brief The IdxRange on which the fields in this object are defined.
    using index_range_type = discrete_domain_type;

    /**
     * @brief The type of the memory space where the field is saved (CPU vs GPU).
     */
    using memory_space = typename chunk_type::memory_space;

private:
    /// Construct a VectorFieldMem on an index range with uninitialized values
    template <std::size_t... Is>
    explicit VectorFieldMem(
            index_range_type const& idx_range,
            Allocator allocator,
            std::index_sequence<Is...> const&)
        : base_type(((void)Is, chunk_type(idx_range, allocator))...)
    {
    }

    /** Element access using a multi-dimensional Idx
     * @param delems discrete coordinates
     * @return copy of this element
     */
    template <class... ODDims, typename T, T... ints>
    element_type operator()(Idx<ODDims...> const& delems, std::integer_sequence<T, ints...>)
            const noexcept
    {
        return element_type((base_type::m_values[ints](delems))...);
    }

public:
    /// Empty VectorFieldMem
    VectorFieldMem() = default;

    /**
     * Construct a VectorFieldMem on an index range with uninitialized values
     *
     * @param[in] idx_range The index range on which the chunk will be defined.
     * @param[in] allocator An optional allocator used to create the chunks.
     */
    explicit VectorFieldMem(index_range_type const& idx_range, Allocator allocator = Allocator())
        : VectorFieldMem(idx_range, allocator, std::make_index_sequence<base_type::NDims> {})
    {
    }

    /// Deleted: use deepcopy instead
    VectorFieldMem(VectorFieldMem const& other) = delete;

    /**
     * Constructs a new VectorFieldMem by move
     * @param other the VectorFieldMem to move
     */
    VectorFieldMem(VectorFieldMem&& other) = default;

    /// Deleted: use deepcopy instead
    VectorFieldMem& operator=(VectorFieldMem const& other) = delete;

    /**
     * Move-assigns a new value to this VectorField
     * @param other the VectorField to move
     * @return *this
     */
    VectorFieldMem& operator=(VectorFieldMem&& other) = default;

    ~VectorFieldMem() = default;

    /**
     * Get a constant reference to this vector field.
     *
     * This function is designed to match the equivalent function in DDC. In Gysela it should
     * not be called directly. Instead the global function get_const_field should be used.
     *
     * @return A constant reference to this vector field.
     */
    view_type span_cview() const
    {
        return view_type(*this);
    }

    /**
     * Get a constant reference to this vector field.
     *
     * This function is designed to match the equivalent function in DDC. In Gysela it should
     * not be called directly. Instead the global function get_field should be used.
     *
     * @return A constant reference to this vector field.
     */
    view_type span_view() const
    {
        return view_type(*this);
    }

    /**
     * Get a modifiable reference to this vector field.
     *
     * This function is designed to match the equivalent function in DDC. In Gysela it should
     * not be called directly. Instead the global function get_field should be used.
     *
     * @return A modifiable reference to this vector field.
     */
    span_type span_view()
    {
        return span_type(*this);
    }

    /** Element access using a list of Idxs
     * @param delems 1D discrete coordinates
     * @return copy of this element
     */
    template <class... ODDims>
    element_type operator()(Idx<ODDims> const&... delems) const noexcept
    {
        Idx<ODDims...> delem_idx(delems...);
        return this->
        operator()(delem_idx, std::make_integer_sequence<int, element_type::size()> {});
    }

    /** Element access using a multi-dimensional Idx
     * @param delems discrete coordinates
     * @return copy of this element
     */
    template <class... ODDims, class = std::enable_if_t<sizeof...(ODDims) != 1>>
    element_type operator()(Idx<ODDims...> const& delems) const noexcept
    {
        return this->operator()(delems, std::make_integer_sequence<int, element_type::size()> {});
    }


    /**
     * @brief Slice out some dimensions.
     *
     * Get the VectorFieldMem on the reduced index range which is obtained by indexing
     * the dimensions QueryDDims at the position slice_spec.
     *
     * @param[in] slice_spec The slice describing the index range of interest.
     *
     * @return A constant reference to the vector field on the sliced index range.
     */
    template <class... QueryDDims>
    auto operator[](Idx<QueryDDims...> const& slice_spec) const
    {
        return span_cview()[slice_spec];
    }

    /**
     * @brief Slice out some dimensions.
     *
     * Get the VectorFieldMem on the reduced index range which is obtained by indexing
     * the dimensions QueryDDims at the position slice_spec.
     *
     * @param[in] slice_spec The slice describing the index range of interest.
     *
     * @return A modifiable reference to the vector field on the sliced index range.
     */
    template <class... QueryDDims>
    auto operator[](Idx<QueryDDims...> const& slice_spec)
    {
        return span_view()[slice_spec];
    }

    /**
     * @brief Slice out some dimensions.
     *
     * Get the VectorFieldMem on the reduced index range passed as an argument.
     *
     * @param[in] oidx_range The index range of interest.
     *
     * @return A modifiable reference to the vector field on the sliced index range.
     */
    template <class... QueryDDims>
    auto operator[](IdxRange<QueryDDims...> const& oidx_range) const
    {
        return span_cview()[oidx_range];
    }

    /**
     * @brief Slice out some dimensions.
     *
     * Get the VectorFieldMem on the reduced index range passed as an argument.
     *
     * @param[in] oidx_range The index range of interest.
     *
     * @return A modifiable reference to the vector field on the sliced index range.
     */
    template <class... QueryDDims>
    auto operator[](IdxRange<QueryDDims...> const& oidx_range)
    {
        return span_view()[oidx_range];
    }
};
