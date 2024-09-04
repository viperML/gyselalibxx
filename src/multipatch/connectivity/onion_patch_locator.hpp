// SPDX-License-Identifier: MIT

#pragma once
#include <stdexcept>

#include <ddc/ddc.hpp>

#include "ddc_aliases.hpp"
#include "ipatch_locator.hpp"
#include "multipatch_type.hpp"
#include "types.hpp"



/**
 * @brief Patch locator specialised for "onion" geometry. 
 * 
 * We define an "onion" geometry a set of patches mapping to the 
 * physical domain in a shape of concentrical rings. The first patch
 * is supposed to be a disk containing the O-point. The other patches 
 * are ordered as concentrical rings drawing away from the O-point. 
 * The order of patches is made by MultipatchIdxRanges and it is important 
 * for the dichotomy method.
 * 
 * We also suppose that we can define a global logical grid that we can split 
 * into the different logical grids of the patches. 
 * 
 * This operator locates on which patch a given physical coordinate is. 
 * 
 * @warning The operator can works on GPU or CPU according to the given 
 * execution  space ExecSpace. The ExecSpace by default is device. 
 * The constructor  will still need to be called from CPU, but the operator() 
 * needs to be called from the given ExecSpace. 
 * 
 * @anchor OnionPatchLocatorImplementation
 * 
 * @tparam MultipatchIdxRanges A MultipatchType type containing the 2D index ranges 
 *          on each patch. 
 * @tparam Mapping A mapping type for all the patches. 
 * @tparam ExecSpace The space (CPU/GPU) where the calculations are carried out.
 *          By default it is on device. 
 */
template <class MultipatchIdxRanges, class Mapping, class ExecSpace = Kokkos::DefaultExecutionSpace>
class OnionPatchLocator;


/**
 * @brief Patch locator specialised for "onion" geometry. 
 * 
 * See @ref OnionPatchLocatorImplementation
 * 
 * @tparam ExecSpace The space (CPU/GPU) where the calculations are carried out.
 * @tparam Patches Patch types. Their order is important.  
 * @tparam Mapping A mapping type for all the patches. 
 */
template <class... Patches, class Mapping, class ExecSpace>
class OnionPatchLocator<MultipatchType<IdxRangeOnPatch, Patches...>, Mapping, ExecSpace>
    : public IPatchLocator
{
    using X = typename Mapping::cartesian_tag_x;
    using Y = typename Mapping::cartesian_tag_y;
    using R = typename Mapping::curvilinear_tag_r;
    using Theta = typename Mapping::curvilinear_tag_theta;

    using MultipatchIdxRanges = MultipatchType<IdxRangeOnPatch, Patches...>;
    using PatchOrdering = ddc::detail::TypeSeq<Patches...>;

    static constexpr std::size_t n_patches = ddc::type_seq_size_v<PatchOrdering>;

    static_assert(
            std::is_invocable_r_v<Coord<R, Theta>, Mapping, Coord<X, Y>>,
            "The mapping has to contain an operator from the physical domain to the logical "
            "domain.");
    static_assert(
            (((std::is_same_v<typename Patches::Dim1, R>)&&(
                     std::is_same_v<typename Patches::Dim2, Theta>))
             && ...),
            "The mappings and the patches have to be defined on the same dimensions.");

    Mapping const m_mapping;
    MultipatchIdxRanges const m_all_idx_ranges;

    Kokkos::View<Coord<R>*, typename ExecSpace::memory_space> m_radii;

public:
    /** 
     * @brief Instantiante the operator with MultipatchType of index ranges and 
     * a mapping on all the patches. 
     * 
     * The order of the elements in the tuple or the MultipatchType doesn't matter.
     * 
     * @param all_idx_ranges A MultipatchType of index ranges defined on the logical domain of each patch.
     * @param mapping Mapping from the logical domains of every patch to the global physical domain. 
    */
    OnionPatchLocator(MultipatchIdxRanges const& all_idx_ranges, Mapping const& mapping)
        : m_mapping(mapping)
        , m_all_idx_ranges(all_idx_ranges)
        , m_radii("m_radii", n_patches + 1)
    {
        set_and_check_radii();
    }


    ~OnionPatchLocator() = default;

    /**
     * @brief Get the patch where the given physical coordinate is. 
     * 
     * We use a dichotomy method to find the patch the physical coordinate is on. 
     * 
     * Each logical grid of the patches are defined on the same dimensions. 
     * Knowing that, we can compare the logical coordinates between the patches
     * in the dichotomy. 
     * 
     * @param coord [in] The given physical coordinate. 
     * @return [int] The patch index where the physical coordinate. If the coordinate
     *              is outside of the domain, it returns outside_domain value 
     *              (defined in IPatchLocator). 
     * 
     * @see IPatchLocator
     */
    KOKKOS_INLINE_FUNCTION int operator()(Coord<X, Y> const coord) const
    {
        int patch_index_min = 0;
        int patch_index_max = n_patches - 1;

        Coord<R> r_max = m_radii(patch_index_max + 1);
        Coord<R> r(m_mapping(coord));
        if (r == r_max) {
            return patch_index_max;
        }

        while (patch_index_max >= patch_index_min) {
            int patch_index_mid = (patch_index_min + patch_index_max) / 2;
            Coord<R> r_min = m_radii(patch_index_mid);
            r_max = m_radii(patch_index_mid + 1);

            if (r_min <= r && r < r_max) {
                return patch_index_mid;
            } else if (r < r_min) {
                patch_index_max = patch_index_mid - 1;
            } else if (r_max < r) {
                patch_index_min = patch_index_mid + 1;
            }
        };
        return IPatchLocator::outside_domain;
    }


    /**
     * @brief Get the mapping on the given Patch. 
     * The function can run on device and host. 
     * @tparam Patch Patch type. 
     * @return The mapping on the given Patch. 
     */
    template <class Patch>
    KOKKOS_FUNCTION Mapping get_mapping_on_patch() const
    {
        return m_mapping;
    }

    /// @brief Get the type of the mapping on the given Patch.
    template <class Patch>
    using get_mapping_on_patch_t = Mapping;


private:
    /** @brief Set the m_radii array containing all the boundary radial coordinates.
     *         Check if the patches are well ordered by comparing the radii. 
     */
    void set_and_check_radii()
    {
        Kokkos::View<Coord<R>*, Kokkos::HostSpace> radii_host("radii_host", n_patches + 1);

        std::array<Coord<R>, n_patches> r_min {
                (Coord<R>(ddc::coordinate(m_all_idx_ranges.template get<Patches>().front())))...};
        std::array<Coord<R>, n_patches> r_max {
                (Coord<R>(ddc::coordinate(m_all_idx_ranges.template get<Patches>().back())))...};

        radii_host(0) = r_min[0];
        for (std::size_t i(0); i < n_patches - 1; i++) {
            if (abs(double(r_min[i + 1] - r_max[i])) > 1e-14) {
                throw std::invalid_argument("The patches listed in PatchOrdering must be ordered.");
            }
            radii_host(i + 1) = r_max[i];
        }
        radii_host(n_patches) = r_max[n_patches - 1];

        Kokkos::deep_copy(m_radii, radii_host);
    }
};


// To help the template deduction.
template <class MultipatchIdxRanges, class Mapping, class ExecSpace = Kokkos::DefaultExecutionSpace>
OnionPatchLocator(MultipatchIdxRanges const& all_idx_ranges, Mapping const& mapping)
        -> OnionPatchLocator<MultipatchIdxRanges, Mapping, ExecSpace>;
