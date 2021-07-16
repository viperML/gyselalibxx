#pragma once

#include "blockview.h"

template <class, class>
class Block;

template <class... Meshes, class ElementType>
class Block<ProductMDomain<Meshes...>, ElementType>
    : public BlockView<ProductMDomain<Meshes...>, ElementType>
{
public:
    /// ND view on this block
    using block_view_type = BlockView<ProductMDomain<Meshes...>, ElementType>;

    using block_span_type = BlockView<ProductMDomain<Meshes...>, ElementType const>;

    /// ND memory view
    using raw_view_type = typename block_view_type::raw_view_type;

    using mdomain_type = typename block_view_type::mdomain_type;

    using mesh_type = ProductMesh<Meshes...>;

    using mcoord_type = typename mdomain_type::mcoord_type;

    using extents_type = typename block_view_type::extents_type;

    using layout_type = typename block_view_type::layout_type;

    using mapping_type = typename block_view_type::mapping_type;

    using element_type = typename block_view_type::element_type;

    using value_type = typename block_view_type::value_type;

    using index_type = typename block_view_type::index_type;

    using difference_type = typename block_view_type::difference_type;

    using pointer = typename block_view_type::pointer;

    using reference = typename block_view_type::reference;

    template <class, class, bool>
    friend class BlockView;

public:
    /** Construct a Block on a domain with uninitialized values
     */
    explicit inline constexpr Block(mdomain_type const& domain)
        : block_view_type(
                domain,
                raw_view_type(
                        new (std::align_val_t(64)) value_type[domain.size()],
                        ExtentsND<sizeof...(Meshes)>(::get<Meshes>(domain).size()...)))
    {
    }

    /** Constructs a new Block by copy
     * 
     * This is deleted, one should use deepcopy
     * @param other the Block to copy
     */
    inline constexpr Block(Block const& other) = delete;

    /** Constructs a new Block by move
     * @param other the Block to move
     */
    inline constexpr Block(Block&& other) = default;

    inline ~Block()
    {
        if (this->raw_view().data()) {
            operator delete(this->raw_view().data(), std::align_val_t(64));
        }
    }

    /** Copy-assigns a new value to this field
     * @param other the Block to copy
     * @return *this
     */
    inline constexpr Block& operator=(Block const& other) = default;

    /** Move-assigns a new value to this field
     * @param other the Block to move
     * @return *this
     */
    inline constexpr Block& operator=(Block&& other) = default;

    /** Copy-assigns a new value to this field
     * @param other the Block to copy
     * @return *this
     */
    template <class... OMeshes, class OElementType>
    inline Block& operator=(Block<ProductMDomain<OMeshes...>, OElementType>&& other)
    {
        copy(*this, other);
        return *this;
    }

    /** Swaps this field with another
     * @param other the Block to swap with this one
     */
    inline constexpr void swap(Block& other)
    {
        Block tmp = std::move(other);
        other = std::move(*this);
        *this = std::move(tmp);
    }

    template <
            class... IndexType,
            std::enable_if_t<(... && std::is_convertible_v<IndexType, std::size_t>), int> = 0>
    inline constexpr element_type const& operator()(IndexType&&... indices) const noexcept
    {
        return this->m_raw(std::forward<IndexType>(indices)...);
    }

    template <
            class... IndexType,
            std::enable_if_t<(... && std::is_convertible_v<IndexType, std::size_t>), int> = 0>
    inline constexpr element_type& operator()(IndexType&&... indices) noexcept
    {
        return this->m_raw(std::forward<IndexType>(indices)...);
    }

    inline constexpr element_type const& operator()(mcoord_type const& indices) const noexcept
    {
        return this->m_raw(indices.array());
    }

    inline constexpr element_type& operator()(mcoord_type const& indices) noexcept
    {
        return this->m_raw(indices.array());
    }

    inline constexpr block_view_type cview() const
    {
        return *this;
    }

    inline constexpr block_view_type cview()
    {
        return *this;
    }

    inline constexpr block_view_type view() const
    {
        return *this;
    }

    inline constexpr block_span_type view()
    {
        return *this;
    }
};
