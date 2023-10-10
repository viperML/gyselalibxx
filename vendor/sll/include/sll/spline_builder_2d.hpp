#pragma once
#include <sll/spline_builder.hpp>


/**
 * @brief A class for creating a 2D spline approximation of a function.
 *
 * A class which contains an operator () which can be used to build a 2D spline approximation
 * of a function. A 2D spline approximation uses a cross-product between two 1D spline builder.
 *
 * @see SplineBuilder
 */
template <class SplineBuilder1, class SplineBuilder2>
class SplineBuilder2D
{
private:
    /**
     * @brief Tag the dimension of the first 1D SplineBuilder.
     */
    using tag_type1 = typename SplineBuilder1::bsplines_type::tag_type;
    /**
     * @brief Tag the dimension of the second 1D SplineBuilder.
     */
    using tag_type2 = typename SplineBuilder2::bsplines_type::tag_type;

public:
    /**
     * @brief The type of the BSplines in the first dimension which are compatible with this class.
     */
    using bsplines_type1 = typename SplineBuilder1::bsplines_type;
    /**
     * @brief The type of the BSplines in the second dimension which are compatible with this class.
     */
    using bsplines_type2 = typename SplineBuilder2::bsplines_type;

    /**
     * @brief Tag the type of the Bsplines in the first dimension.
     */
    using builder_type1 = SplineBuilder1;
    /**
     * @brief Tag the type of the Bsplines in the second dimension.
     */
    using builder_type2 = SplineBuilder2;

    /**
     * @brief The type of the interpolation mesh in the first dimension used by this class.
     */
    using interpolation_mesh_type1 = typename SplineBuilder1::mesh_type;
    /**
     * @brief The type of the interpolation mesh in the second dimension used by this class.
     */
    using interpolation_mesh_type2 = typename SplineBuilder2::mesh_type;

    /**
     * @brief The type of the domain for the interpolation mesh is the first dimension used by this class.
     */
    using interpolation_domain_type1 = ddc::DiscreteDomain<interpolation_mesh_type1>;
    /**
     * @brief The type of the domain for the interpolation mesh is the second dimension used by this class.
     */
    using interpolation_domain_type2 = ddc::DiscreteDomain<interpolation_mesh_type2>;
    /**
     * @brief The type of the domain for the interpolation mesh is the 2D dimension used by this class.
     */
    using interpolation_domain_type
            = ddc::DiscreteDomain<interpolation_mesh_type1, interpolation_mesh_type2>;

    /**
     * @brief The boundary condition implemented at the lower bound in the first dimension.
     */
    static constexpr BoundCond BcXmin1 = SplineBuilder1::s_bc_xmin;
    /**
     * @brief The boundary condition implemented at the upper bound in the first dimension.
     */
    static constexpr BoundCond BcXmax1 = SplineBuilder1::s_bc_xmax;
    /**
     * @brief The boundary condition implemented at the lower bound in the second dimension.
     */
    static constexpr BoundCond BcXmin2 = SplineBuilder2::s_bc_xmin;
    /**
     * @brief The boundary condition implemented at the upper bound in the second dimension.
     */
    static constexpr BoundCond BcXmax2 = SplineBuilder2::s_bc_xmax;

private:
    builder_type1 spline_builder1;
    builder_type2 spline_builder2;
    interpolation_domain_type m_interpolation_domain;

public:
    /**
     * @brief Create a new SplineBuilder2D.
     *
     * @param interpolation_domain
     *      The 2D domain on which points will be provided in order to
     *      create the 2D spline approximation.
     */
    SplineBuilder2D(interpolation_domain_type const& interpolation_domain)
        : spline_builder1(ddc::select<interpolation_mesh_type1>(interpolation_domain))
        , spline_builder2(ddc::select<interpolation_mesh_type2>(interpolation_domain))
        , m_interpolation_domain(interpolation_domain)
    {
    }

    /**
     * @brief Create a new SplineBuilder2D by copy
     *
     * @param x
     *      The SplineBuilder2D being copied.
     */
    SplineBuilder2D(SplineBuilder2D const& x) = delete;

    /**
     * @brief Create a new SplineBuilder2D by copy
     *
     * @param x
     *      The temporary SplineBuilder2D being copied.
     */
    SplineBuilder2D(SplineBuilder2D&& x) = default;

    ~SplineBuilder2D() = default;

    SplineBuilder2D& operator=(SplineBuilder2D const& x) = delete;


    /**
     * @brief Copy a SplineBuilder2D.
     *
     * @param x
     *      The temporary SplineBuilder2D being copied.
     * @returns A reference to this object.
     */
    SplineBuilder2D& operator=(SplineBuilder2D&& x) = default;

    /**
     * @brief Build a 2D spline approximation of a function.
     *
     * Use the values of a function at known grid points (as specified by
     * SplineBuilder2D::interpolation_domain_type) and the derivatives of the
     * function at the boundaries (if necessary for the chosen boundary
     * conditions) to calculate a 2D spline approximation of a function.
     *
     * The spline approximation is stored as a ChunkSpan of coefficients
     * associated with basis-splines.
     *
     * @param[out] spline
     *      The coefficients of the spline calculated by the function.
     * @param[in] vals
     *      The values of the function at the grid points.
     * @param[in] derivs_xmin
     *      The values of the derivatives at the lower boundary in the first dimension.
     * @param[in] derivs_xmax
     *      The values of the derivatives at the upper boundary in the first dimension.
     * @param[in] derivs_ymin
     *      The values of the derivatives at the lower boundary in the second dimension.
     * @param[in] derivs_ymax
     *      The values of the derivatives at the upper boundary in the second dimension.
     * @param[in] mixed_derivs_xmin_ymin
     *      The values of the the cross-derivatives at the lower boundary in the first dimension
     *      and the lower boundary in the second dimension.
     * @param[in] mixed_derivs_xmax_ymin
     *      The values of the the cross-derivatives at the upper boundary in the first dimension
     *      and the lower boundary in the second dimension.
     * @param[in] mixed_derivs_xmin_ymax
     *      The values of the the cross-derivatives at the lower boundary in the first dimension
     *      and the upper boundary in the second dimension.
     * @param[in] mixed_derivs_xmax_ymax
     *      The values of the the cross-derivatives at the upper boundary in the first dimension
     *      and the upper boundary in the second dimension.
     */
    void operator()(
            ddc::ChunkSpan<double, ddc::DiscreteDomain<bsplines_type1, bsplines_type2>> spline,
            ddc::ChunkSpan<double const, interpolation_domain_type> vals,
            std::optional<CDSpan2D> const derivs_xmin = std::nullopt,
            std::optional<CDSpan2D> const derivs_xmax = std::nullopt,
            std::optional<CDSpan2D> const derivs_ymin = std::nullopt,
            std::optional<CDSpan2D> const derivs_ymax = std::nullopt,
            std::optional<CDSpan2D> const mixed_derivs_xmin_ymin = std::nullopt,
            std::optional<CDSpan2D> const mixed_derivs_xmax_ymin = std::nullopt,
            std::optional<CDSpan2D> const mixed_derivs_xmin_ymax = std::nullopt,
            std::optional<CDSpan2D> const mixed_derivs_xmax_ymax = std::nullopt) const;

    /**
     * @brief Get the first dimension domain from which the approximation is defined.
     *
     * Get the first dimension  domain on which values of the function must be provided in order
     * to build a spline approximation of the function.
     *
     * @return The first dimension domain for the grid points.
     */
    interpolation_domain_type1 const& interpolation_domain1() const noexcept
    {
        return spline_builder1.interpolation_domain();
    }

    /**
     * @brief Get the second dimension domain from which the approximation is defined.
     *
     * Get the second dimension  domain on which values of the function must be provided in order
     * to build a spline approximation of the function.
     *
     * @return The second dimension domain for the grid points.
     */
    interpolation_domain_type2 const& interpolation_domain2() const noexcept
    {
        return spline_builder2.interpolation_domain();
    }

    /**
     * @brief Get the 2D dimension domain from which the approximation is defined.
     *
     * Get the 2D dimension  domain on which values of the function must be provided in order
     * to build a spline approximation of the function.
     *
     * @return The 2D dimension domain for the grid points.
     */
    interpolation_domain_type const& interpolation_domain() const noexcept
    {
        return m_interpolation_domain;
    }

    /**
     * @brief Get the 2D domain on which the approximation is defined.
     *
     * Get the 2D domain of the basis-splines for which the coefficients of the spline
     * approximation must be calculated.
     *
     * @return The 2D domain for the splines.
     */
    ddc::DiscreteDomain<bsplines_type1, bsplines_type2> spline_domain() const noexcept
    {
        return ddc::DiscreteDomain<bsplines_type1, bsplines_type2>(
                ddc::DiscreteElement<bsplines_type1, bsplines_type2>(0, 0),
                ddc::DiscreteVector<bsplines_type1, bsplines_type2>(
                        ddc::discrete_space<bsplines_type1>().size(),
                        ddc::discrete_space<bsplines_type2>().size()));
    }

    /**
     * @brief Get the 1D spline builder in the first dimension.
     *
     * @return the 1D spline builder in the first dimension.
     */
    builder_type1 const& get_builder_1() const noexcept
    {
        return spline_builder1;
    }

    /**
     * @brief Get the 1D spline builder in the second dimension.
     *
     * @return the 1D spline builder in the second dimension.
     */
    builder_type2 const& get_builder_2() const noexcept
    {
        return spline_builder2;
    }
};


template <class SplineBuilder1, class SplineBuilder2>
void SplineBuilder2D<SplineBuilder1, SplineBuilder2>::operator()(
        ddc::ChunkSpan<double, ddc::DiscreteDomain<bsplines_type1, bsplines_type2>> spline,
        ddc::ChunkSpan<double const, interpolation_domain_type> vals,
        std::optional<CDSpan2D> const derivs_xmin,
        std::optional<CDSpan2D> const derivs_xmax,
        std::optional<CDSpan2D> const derivs_ymin,
        std::optional<CDSpan2D> const derivs_ymax,
        std::optional<CDSpan2D> const mixed_derivs_xmin_ymin,
        std::optional<CDSpan2D> const mixed_derivs_xmax_ymin,
        std::optional<CDSpan2D> const mixed_derivs_xmin_ymax,
        std::optional<CDSpan2D> const mixed_derivs_xmax_ymax) const
{
    const std::size_t nbc_xmin = spline_builder1.s_nbc_xmin;
    const std::size_t nbc_xmax = spline_builder1.s_nbc_xmax;
    const std::size_t nbc_ymin = spline_builder2.s_nbc_xmin;
    const std::size_t nbc_ymax = spline_builder2.s_nbc_xmax;

    assert((BcXmin1 == BoundCond::HERMITE)
           != (!derivs_xmin.has_value() || derivs_xmin->extent(0) == 0));
    assert((BcXmax1 == BoundCond::HERMITE)
           != (!derivs_xmax.has_value() || derivs_xmax->extent(0) == 0));
    assert((BcXmin2 == BoundCond::HERMITE)
           != (!derivs_ymin.has_value() || derivs_ymin->extent(0) == 0));
    assert((BcXmax2 == BoundCond::HERMITE)
           != (!derivs_ymax.has_value() || derivs_ymax->extent(0) == 0));
    assert((BcXmin1 == BoundCond::HERMITE && BcXmin2 == BoundCond::HERMITE)
           != (!mixed_derivs_xmin_ymin.has_value()
               || mixed_derivs_xmin_ymin->extent(0) != nbc_xmin));
    assert((BcXmax1 == BoundCond::HERMITE && BcXmin2 == BoundCond::HERMITE)
           != (!mixed_derivs_xmax_ymin.has_value()
               || mixed_derivs_xmax_ymin->extent(0) != nbc_xmax));
    assert((BcXmin2 == BoundCond::HERMITE && BcXmax2 == BoundCond::HERMITE)
           != (!mixed_derivs_xmin_ymax.has_value()
               || mixed_derivs_xmin_ymax->extent(0) != nbc_xmin));
    assert((BcXmax2 == BoundCond::HERMITE && BcXmax2 == BoundCond::HERMITE)
           != (!mixed_derivs_xmax_ymax.has_value()
               || mixed_derivs_xmax_ymax->extent(0) != nbc_xmax));

    ddc::Chunk<double, ddc::DiscreteDomain<bsplines_type1>> spline1(
            spline_builder1.spline_domain());
    ddc::Chunk<double, ddc::DiscreteDomain<bsplines_type2>> spline2(
            spline_builder2.spline_domain());

    using IMesh1 = ddc::DiscreteElement<interpolation_mesh_type1>;
    using IMesh2 = ddc::DiscreteElement<interpolation_mesh_type2>;

    /******************************************************************
    *  Cycle over x1 position (or order of x1-derivative at boundary)
    *  and interpolate f along x2 direction.
    *******************************************************************/
    if constexpr (BcXmin2 == BoundCond::HERMITE) {
        assert((long int)(derivs_ymin->extent(0))
                       == spline_builder1.interpolation_domain().extents()
               && derivs_ymin->extent(1) == nbc_ymin);
        if constexpr (BcXmin1 == BoundCond::HERMITE) {
            assert(mixed_derivs_xmin_ymin->extent(0) == nbc_xmin
                   && mixed_derivs_xmin_ymin->extent(1) == nbc_ymin);
        }
        if constexpr (BcXmax1 == BoundCond::HERMITE) {
            assert(mixed_derivs_xmax_ymin->extent(0) == nbc_xmax
                   && mixed_derivs_xmax_ymin->extent(1) == nbc_ymin);
        }
        // In the boundary region we interpolate the derivatives
        for (int i = nbc_ymin; i > 0; --i) {
            const ddc::DiscreteElement<bsplines_type2> spl_idx(i - 1);

            // Get interpolated values
            ddc::Chunk<double, interpolation_domain_type1> vals1(
                    spline_builder1.interpolation_domain());
            ddc::for_each(spline_builder1.interpolation_domain(), [&](IMesh1 const j) {
                vals1(j) = (*derivs_ymin)(j.uid(), i - 1);
            });

            // Get interpolated derivatives
            std::vector<double> l_derivs(nbc_xmin);
            if constexpr (BcXmin1 == BoundCond::HERMITE) {
                for (std::size_t j(0); j < nbc_xmin; ++j)
                    l_derivs[j] = (*mixed_derivs_xmin_ymin)(j, i - 1);
            }
            const std::optional<CDSpan1D> deriv_l(
                    BcXmin1 == BoundCond::HERMITE
                            ? std::optional(CDSpan1D(l_derivs.data(), nbc_xmin))
                            : std::nullopt);

            std::vector<double> r_derivs(nbc_xmax);
            if constexpr (BcXmax1 == BoundCond::HERMITE) {
                for (std::size_t j(0); j < nbc_xmax; ++j)
                    r_derivs[j] = (*mixed_derivs_xmax_ymin)(j, i - 1);
            }
            const std::optional<CDSpan1D> deriv_r(
                    BcXmax1 == BoundCond::HERMITE
                            ? std::optional(CDSpan1D(r_derivs.data(), nbc_xmax))
                            : std::nullopt);

            // Interpolate derivatives
            spline_builder1(spline1, vals1, deriv_l, deriv_r);

            // Save result into 2d spline structure
            ddc::for_each(
                    ddc::get_domain<bsplines_type1>(spline),
                    [&](ddc::DiscreteElement<bsplines_type1> const j) {
                        spline(spl_idx, j) = spline1(j);
                    });
        }
    }

    if (BcXmin1 == BoundCond::HERMITE) {
        assert((long int)(derivs_xmin->extent(0))
                       == spline_builder2.interpolation_domain().extents()
               && derivs_xmin->extent(1) == nbc_xmin);
    }
    if (BcXmax1 == BoundCond::HERMITE) {
        assert((long int)(derivs_xmax->extent(0))
                       == spline_builder2.interpolation_domain().extents()
               && derivs_xmax->extent(1) == nbc_xmax);
    }
    ddc::for_each(spline_builder2.interpolation_domain(), [&](IMesh2 const i) {
        const std::size_t ii = i.uid();
        const ddc::DiscreteElement<bsplines_type2> spl_idx(nbc_ymin + ii);

        // Get interpolated values
        ddc::Chunk<double, interpolation_domain_type1> vals1(
                spline_builder1.interpolation_domain());
        ddc::deepcopy(vals1, vals[i]);

        // Get interpolated derivatives
        const std::optional<CDSpan1D> deriv_l(
                BcXmin1 == BoundCond::HERMITE ? std::optional(
                        CDSpan1D(derivs_xmin->data_handle() + ii * nbc_xmin, nbc_xmin))
                                              : std::nullopt);
        const std::optional<CDSpan1D> deriv_r(
                BcXmax1 == BoundCond::HERMITE ? std::optional(
                        CDSpan1D(derivs_xmax->data_handle() + ii * nbc_xmax, nbc_xmax))
                                              : std::nullopt);

        // Interpolate values
        spline_builder1(spline1, vals1, deriv_l, deriv_r);

        // Save result into 2d spline structure
        ddc::for_each(
                ddc::get_domain<bsplines_type1>(spline),
                [&](ddc::DiscreteElement<bsplines_type1> const j) {
                    spline(spl_idx, j) = spline1(j);
                });
    });

    if constexpr (BcXmax2 == BoundCond::HERMITE) {
        assert((long int)(derivs_ymax->extent(0))
                       == spline_builder1.interpolation_domain().extents()
               && derivs_ymax->extent(1) == nbc_ymax);
        if constexpr (BcXmin2 == BoundCond::HERMITE) {
            assert(mixed_derivs_xmin_ymax->extent(0) == nbc_xmin
                   && mixed_derivs_xmin_ymax->extent(1) == nbc_ymax);
        }
        if constexpr (BcXmax2 == BoundCond::HERMITE) {
            assert(mixed_derivs_xmax_ymax->extent(0) == nbc_xmax
                   && mixed_derivs_xmax_ymax->extent(1) == nbc_ymax);
        }
        for (int i = nbc_ymax; i > 0; --i) {
            // In the boundary region we interpolate the derivatives
            const ddc::DiscreteElement<bsplines_type2> spl_idx(
                    i + ddc::discrete_space<bsplines_type2>().nbasis() - nbc_ymax - 1);

            // Get interpolated values
            ddc::Chunk<double, interpolation_domain_type1> vals1(
                    spline_builder1.interpolation_domain());
            ddc::for_each(spline_builder1.interpolation_domain(), [&](IMesh1 const j) {
                vals1(j) = (*derivs_ymax)(j.uid(), i - 1);
            });

            // Get interpolated derivatives
            std::vector<double> l_derivs(nbc_xmin);
            if constexpr (BcXmin1 == BoundCond::HERMITE) {
                for (std::size_t j(0); j < nbc_xmin; ++j)
                    l_derivs[j] = (*mixed_derivs_xmin_ymax)(j, i - 1);
            }
            const std::optional<CDSpan1D> deriv_l(
                    BcXmin1 == BoundCond::HERMITE
                            ? std::optional(CDSpan1D(l_derivs.data(), nbc_xmin))
                            : std::nullopt);

            std::vector<double> r_derivs(nbc_xmax);
            if constexpr (BcXmax1 == BoundCond::HERMITE) {
                for (std::size_t j(0); j < nbc_xmax; ++j)
                    r_derivs[j] = (*mixed_derivs_xmax_ymax)(j, i - 1);
            }
            const std::optional<CDSpan1D> deriv_r(
                    BcXmax1 == BoundCond::HERMITE
                            ? std::optional(CDSpan1D(r_derivs.data(), nbc_xmax))
                            : std::nullopt);

            // Interpolate derivatives
            spline_builder1(spline1, vals1, deriv_l, deriv_r);

            // Save result into 2d spline structure
            ddc::for_each(
                    ddc::get_domain<bsplines_type1>(spline),
                    [&](ddc::DiscreteElement<bsplines_type1> const j) {
                        spline(spl_idx, j) = spline1(j);
                    });
        }
    }

    using IMeshV2 = ddc::DiscreteVector<bsplines_type2>;

    /******************************************************************
    *  Cycle over x1 position (or order of x1-derivative at boundary)
    *  and interpolate x2 cofficients along x2 direction.
    *******************************************************************/
    const ddc::DiscreteDomain<bsplines_type1> spline_basis_domain
            = ddc::DiscreteDomain<bsplines_type1>(
                    ddc::DiscreteElement<bsplines_type1>(0),
                    ddc::DiscreteVector<bsplines_type1>(
                            ddc::discrete_space<bsplines_type1>().nbasis()));

    ddc::for_each(spline_basis_domain, [&](ddc::DiscreteElement<bsplines_type1> const i) {
        const ddc::ChunkSpan<double, ddc::DiscreteDomain<bsplines_type2>> line_2 = spline[i];
        const ddc::DiscreteDomain<bsplines_type2> whole_line_dom
                = ddc::get_domain<bsplines_type2>(spline);
        // Get interpolated values
        ddc::ChunkSpan<double, ddc::DiscreteDomain<bsplines_type2>> const vals2(
                line_2[whole_line_dom.remove(IMeshV2(nbc_ymin), IMeshV2(nbc_ymax))]);
        // Get interpolated values acting as derivatives
        ddc::ChunkSpan<double, ddc::DiscreteDomain<bsplines_type2>> const l_derivs(
                line_2[whole_line_dom.take_first(IMeshV2(nbc_ymin))]);
        ddc::ChunkSpan<double, ddc::DiscreteDomain<bsplines_type2>> const r_derivs(
                line_2[whole_line_dom.take_last(IMeshV2(nbc_ymax))]);
        const std::optional<CDSpan1D> deriv_l(
                BcXmin2 == BoundCond::HERMITE ? std::optional(l_derivs.allocation_mdspan())
                                              : std::nullopt);
        const std::optional<CDSpan1D> deriv_r(
                BcXmax2 == BoundCond::HERMITE ? std::optional(r_derivs.allocation_mdspan())
                                              : std::nullopt);

        ddc::ChunkSpan<double const, interpolation_domain_type2>
                vals2_i(vals2.data_handle(), spline_builder2.interpolation_domain());

        // Interpolate coefficients
        spline_builder2(spline2, vals2_i, deriv_l, deriv_r);

        // Re-write result into 2d spline structure
        ddc::for_each(
                ddc::get_domain<bsplines_type2>(spline),
                [&](ddc::DiscreteElement<bsplines_type2> const j) { spline(i, j) = spline2(j); });
    });

    if (bsplines_type1::is_periodic()) {
        for (std::size_t i(0); i < bsplines_type1::degree(); ++i) {
            const ddc::DiscreteElement<bsplines_type1> i_start(i);
            const ddc::DiscreteElement<bsplines_type1> i_end(
                    ddc::discrete_space<bsplines_type1>().nbasis() + i);
            ddc::for_each(
                    ddc::get_domain<bsplines_type2>(spline),
                    [&](ddc::DiscreteElement<bsplines_type2> const j) {
                        spline(i_end, j) = spline(i_start, j);
                    });
        }
    }
}
