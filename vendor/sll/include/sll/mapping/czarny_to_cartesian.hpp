#pragma once

#include <cmath>

#include <ddc/ddc.hpp>

#include "analytical_invertible_curvilinear2d_to_cartesian.hpp"


/**
 * @brief A class for describing the Czarny 2D mapping.
 *
 * The mapping @f$ (r,\theta)\mapsto (x,y) @f$ is defined by
 *
 * @f$ x(r,\theta) = \frac{1}{\epsilon} \left( 1 - \sqrt{1 + \epsilon(\epsilon + 2 r \cos(\theta))} \right),@f$
 *
 * @f$ y(r,\theta) = \frac{e\xi r \sin(\theta)}{2 -\sqrt{1 + \epsilon(\epsilon + 2 r \cos(\theta))} },@f$
 *
 * with @f$ \xi = 1/\sqrt{1 - \epsilon^2 /4} @f$ and @f$ e @f$ and @f$ \epsilon @f$ given as parameters.
 * It and its Jacobian matrix are invertible everywhere except for @f$ r = 0 @f$.
 *
 * Its Jacobian coefficients are defined as follow
 *
 * @f$ J_{11}(r,\theta) = - \cos(\theta)\frac{1}{ \sqrt{1 + \epsilon(\epsilon + 2 r \cos(\theta))} } @f$
 *
 * @f$ J_{12}(r,\theta)  =  r\sin(\theta)\frac{1}{ \sqrt{1 + \epsilon(\epsilon + 2 r \cos(\theta))} } @f$
 *
 * @f$ J_{21}(r,\theta)  =  \cos(\theta)\frac{e\epsilon \xi r\sin(\theta)}{ \sqrt{1 + \epsilon(\epsilon + 2 r \cos(\theta))} \left(
 * 2 - \sqrt{1 + \epsilon(\epsilon + 2 r \cos(\theta))}  \right)^2 }
 * +  \sin(\theta)\frac{e\xi }{ 2- \sqrt{1 + \epsilon(\epsilon + 2 r \cos(\theta))} }@f$
 *
 * @f$ J_{22}(r,\theta)  =   r \sin(\theta)\frac{- e\epsilon \xi r \sin(\theta)}{ \sqrt{1 + \epsilon(\epsilon + 2 r \cos(\theta))} \left(
 * 2 - \sqrt{1 + \epsilon(\epsilon + 2 r \cos(\theta))} \right)^2 }
 * +  r\cos(\theta)\frac{e\xi }{ 2 -\sqrt{1 + \epsilon(\epsilon + 2 r \cos(\theta))} }@f$.
 *
 *
 * and
 * @f$ \det(J(r, \theta)) = \frac{- r}{ \sqrt{1 + \epsilon(\epsilon + 2 r \cos(\theta))}}
 *  \frac{e\xi}{2 -  \sqrt{1 + \epsilon(\epsilon + 2 r \cos(\theta))}}. @f$
 *
 *
 * @see AnalyticalInvertibleCurvilinear2DToCartesian
 */
template <class X, class Y, class R, class Theta>
class CzarnyToCartesian : public AnalyticalInvertibleCurvilinear2DToCartesian<X, Y, R, Theta>
{
public:
    /**
     * @brief Indicate the first physical coordinate.
     */
    using cartesian_tag_x = X;
    /**
     * @brief Indicate the second physical coordinate.
     */
    using cartesian_tag_y = Y;
    /**
     * @brief Indicate the first logical coordinate.
     */
    using circular_tag_r = R;
    /**
     * @brief Indicate the second logical coordinate.
     */
    using circular_tag_theta = Theta;
    /**
     * @brief Define a 2x2 matrix with an 2D array of an 2D array.
     */
    using Matrix_2x2 = std::array<std::array<double, 2>, 2>;

private:
    double m_epsilon;
    double m_e;

public:
    /**
     * @brief Instantiate a CzarnyToCartesian from parameters.
     *
     * @param[in] epsilon
     * 			The @f$ \epsilon @f$ parameter in the definition of the mapping CzarnyToCartesian.
     *
     * @param[in] e
     * 			The @f$ e @f$ parameter in the definition of the mapping CzarnyToCartesian.
     *
     * @see CzarnyToCartesian
     */
    CzarnyToCartesian(double epsilon, double e) : m_epsilon(epsilon), m_e(e) {}

    /**
     * @brief Instantiate a CzarnyToCartesian from another CzarnyToCartesian (lvalue).
     *
     * @param[in] other
     * 		CzarnyToCartesian mapping used to instantiate the new one.
     */
    KOKKOS_FUNCTION CzarnyToCartesian(CzarnyToCartesian const& other)
        : m_epsilon(other.epsilon())
        , m_e(other.e())
    {
    }

    /**
     * @brief Instantiate a CzarnyToCartesian from another temporary CzarnyToCartesian (rvalue).
     *
     * @param[in] x
     * 		CzarnyToCartesian mapping used to instantiate the new one.
     */
    CzarnyToCartesian(CzarnyToCartesian&& x) = default;

    ~CzarnyToCartesian() = default;

    /**
     * @brief Assign a CzarnyToCartesian from another CzarnyToCartesian (lvalue).
     *
     * @param[in] x
     * 		CzarnyToCartesian mapping used to assign.
     *
     * @return The CzarnyToCartesian assigned.
     */
    CzarnyToCartesian& operator=(CzarnyToCartesian const& x) = default;

    /**
     * @brief Assign a CzarnyToCartesian from another temporary CzarnyToCartesian (rvalue).
     *
     * @param[in] x
     * 		CzarnyToCartesian mapping used to assign.
     *
     * @return The CzarnyToCartesian assigned.
     */
    CzarnyToCartesian& operator=(CzarnyToCartesian&& x) = default;

    /**
     * @brief Return the @f$ \epsilon @f$ parameter.
     *
     * @return The value of @f$ \epsilon @f$.
     *
     * @see CzarnyToCartesian
     */
    KOKKOS_FUNCTION double epsilon() const
    {
        return m_epsilon;
    }

    /**
     * @brief Return the @f$ e @f$ parameter.
     *
     * @return The value of @f$ e @f$.
     *
     * @see CzarnyToCartesian
     */
    KOKKOS_FUNCTION double e() const
    {
        return m_e;
    }

    KOKKOS_FUNCTION ddc::Coordinate<X, Y> operator()(
            ddc::Coordinate<R, Theta> const& coord) const final
    {
        const double r = ddc::get<R>(coord);
        const double theta = ddc::get<Theta>(coord);
        const double tmp1
                = Kokkos::sqrt(m_epsilon * (m_epsilon + 2.0 * r * Kokkos::cos(theta)) + 1.0);

        const double x = (1.0 - tmp1) / m_epsilon;
        const double y = m_e * r * Kokkos::sin(theta)
                         / (Kokkos::sqrt(1.0 - 0.25 * m_epsilon * m_epsilon) * (2.0 - tmp1));

        return ddc::Coordinate<X, Y>(x, y);
    }

    KOKKOS_FUNCTION ddc::Coordinate<R, Theta> operator()(
            ddc::Coordinate<X, Y> const& coord) const final
    {
        const double x = ddc::get<X>(coord);
        const double y = ddc::get<Y>(coord);
        const double ex = 1. + m_epsilon * x;
        const double ex2 = (m_epsilon * x * x - 2. * x - m_epsilon);
        const double xi2 = 1. / (1. - m_epsilon * m_epsilon * 0.25);
        const double xi = Kokkos::sqrt(xi2);
        const double r = Kokkos::sqrt(y * y * ex * ex / (m_e * m_e * xi2) + ex2 * ex2 * 0.25);
        double theta
                = Kokkos::atan2(2. * y * ex, (m_e * xi * (m_epsilon * x * x - 2. * x - m_epsilon)));
        if (theta < 0) {
            theta = 2 * M_PI + theta;
        }
        return ddc::Coordinate<R, Theta>(r, theta);
    }

    double jacobian(ddc::Coordinate<R, Theta> const& coord) const final
    {
        const double r = ddc::get<R>(coord);
        const double theta = ddc::get<Theta>(coord);
        const double xi = std::sqrt(1. / (1. - m_epsilon * m_epsilon * 0.25));
        return -r / std::sqrt(1 + m_epsilon * (m_epsilon + 2.0 * r * std::cos(theta))) * m_e * xi
               / (2 - std::sqrt(1 + m_epsilon * (m_epsilon + 2.0 * r * std::cos(theta))));
    }


    void jacobian_matrix(ddc::Coordinate<R, Theta> const& coord, Matrix_2x2& matrix) const final
    {
        const double r = ddc::get<R>(coord);
        const double theta = ddc::get<Theta>(coord);

        const double sin_theta = std::sin(theta);
        const double cos_theta = std::cos(theta);
        const double xi2 = 1. / (1. - m_epsilon * m_epsilon * 0.25);
        const double xi = std::sqrt(xi2);
        const double sqrt_eps = std::sqrt(m_epsilon * (m_epsilon + 2.0 * r * cos_theta) + 1.0);
        const double sqrt_eps_2 = 2.0 - sqrt_eps;

        matrix[0][0] = -cos_theta / sqrt_eps;
        matrix[0][1] = r * sin_theta / sqrt_eps;
        matrix[1][0] = m_e * m_epsilon * r * sin_theta * cos_theta * xi
                               / (sqrt_eps_2 * sqrt_eps_2 * sqrt_eps)
                       + m_e * sin_theta * xi / sqrt_eps_2;
        matrix[1][1] = r
                       * (-m_e * m_epsilon * r * sin_theta * sin_theta * xi
                                  / (sqrt_eps_2 * sqrt_eps_2 * sqrt_eps)
                          + m_e * cos_theta * xi / sqrt_eps_2);
    }

    double jacobian_11(ddc::Coordinate<R, Theta> const& coord) const final
    {
        const double r = ddc::get<R>(coord);
        const double theta = ddc::get<Theta>(coord);
        return -std::cos(theta)
               / std::sqrt(m_epsilon * (m_epsilon + 2.0 * r * std::cos(theta)) + 1.0);
    }

    double jacobian_12(ddc::Coordinate<R, Theta> const& coord) const final
    {
        const double r = ddc::get<R>(coord);
        const double theta = ddc::get<Theta>(coord);
        return r * std::sin(theta)
               / std::sqrt(m_epsilon * (m_epsilon + 2.0 * r * std::cos(theta)) + 1.0);
    }

    double jacobian_21(ddc::Coordinate<R, Theta> const& coord) const final
    {
        const double r = ddc::get<R>(coord);
        const double theta = ddc::get<Theta>(coord);

        const double sin_theta = std::sin(theta);
        const double cos_theta = std::cos(theta);
        const double xi2 = 1. / (1. - m_epsilon * m_epsilon * 0.25);
        const double xi = std::sqrt(xi2);
        const double tmp1 = std::sqrt(m_epsilon * (m_epsilon + 2.0 * r * cos_theta) + 1.0);
        const double tmp2 = 2.0 - tmp1;
        return m_e * m_epsilon * r * sin_theta * cos_theta * xi / (tmp2 * tmp2 * tmp1)
               + m_e * sin_theta * xi / tmp2;
    }

    double jacobian_22(ddc::Coordinate<R, Theta> const& coord) const final
    {
        const double r = ddc::get<R>(coord);
        const double theta = ddc::get<Theta>(coord);

        const double sin_theta = std::sin(theta);
        const double cos_theta = std::cos(theta);
        const double xi2 = 1. / (1. - m_epsilon * m_epsilon * 0.25);
        const double xi = std::sqrt(xi2);
        const double tmp1 = std::sqrt(m_epsilon * (m_epsilon + 2.0 * r * cos_theta) + 1.0);
        const double tmp2 = 2.0 - tmp1;
        return r
               * (-m_e * m_epsilon * r * sin_theta * sin_theta * xi / (tmp2 * tmp2 * tmp1)
                  + m_e * cos_theta * xi / tmp2);
    }

    void inv_jacobian_matrix(ddc::Coordinate<R, Theta> const& coord, Matrix_2x2& matrix) const final
    {
        const double r = ddc::get<R>(coord);
        const double theta = ddc::get<Theta>(coord);

        assert(r >= 1e-15);

        const double sin_theta = std::sin(theta);
        const double cos_theta = std::cos(theta);
        const double xi = std::sqrt(1. / (1. - m_epsilon * m_epsilon * 0.25));
        const double divisor = 2 - std::sqrt(1 + m_epsilon * (m_epsilon + 2.0 * r * cos_theta));

        const double fact_1 = 1 / std::sqrt(1 + m_epsilon * (m_epsilon + 2.0 * r * cos_theta));
        const double fact_2 = m_e * m_epsilon * xi * r * sin_theta * fact_1 / divisor / divisor;
        const double fact_3 = m_e * xi / divisor;

        matrix[0][0] = -1 / fact_1 * (-sin_theta * fact_2 + cos_theta * fact_3) / fact_3;
        matrix[0][1] = sin_theta / fact_3;
        matrix[1][0] = 1 / r / fact_1 * (cos_theta * fact_2 + sin_theta * fact_3) / fact_3;
        matrix[1][1] = 1 / r * cos_theta / fact_3;
    }



    double inv_jacobian_11(ddc::Coordinate<R, Theta> const& coord) const final
    {
        const double r = ddc::get<R>(coord);
        const double theta = ddc::get<Theta>(coord);

        const double sin_theta = std::sin(theta);
        const double cos_theta = std::cos(theta);
        const double xi = std::sqrt(1. / (1. - m_epsilon * m_epsilon * 0.25));
        const double divisor = 2 - std::sqrt(1 + m_epsilon * (m_epsilon + 2.0 * r * cos_theta));

        const double fact_1 = 1 / std::sqrt(1 + m_epsilon * (m_epsilon + 2.0 * r * cos_theta));
        const double fact_2 = m_e * m_epsilon * xi * r * sin_theta * fact_1 / divisor / divisor;
        const double fact_3 = m_e * xi / divisor;

        return -1 / fact_1 * (-sin_theta * fact_2 + cos_theta * fact_3) / fact_3;
    }

    double inv_jacobian_12(ddc::Coordinate<R, Theta> const& coord) const final
    {
        const double r = ddc::get<R>(coord);
        const double theta = ddc::get<Theta>(coord);

        const double sin_theta = std::sin(theta);
        const double cos_theta = std::cos(theta);
        const double xi = std::sqrt(1. / (1. - m_epsilon * m_epsilon * 0.25));
        const double divisor = 2 - std::sqrt(1 + m_epsilon * (m_epsilon + 2.0 * r * cos_theta));

        const double fact_3 = m_e * xi / divisor;
        return sin_theta / fact_3;
    }

    double inv_jacobian_21(ddc::Coordinate<R, Theta> const& coord) const final
    {
        const double r = ddc::get<R>(coord);
        const double theta = ddc::get<Theta>(coord);

        assert(r >= 1e-15);

        const double sin_theta = std::sin(theta);
        const double cos_theta = std::cos(theta);
        const double xi = std::sqrt(1. / (1. - m_epsilon * m_epsilon * 0.25));
        const double divisor = 2 - std::sqrt(1 + m_epsilon * (m_epsilon + 2.0 * r * cos_theta));

        const double fact_1 = 1 / std::sqrt(1 + m_epsilon * (m_epsilon + 2.0 * r * cos_theta));
        const double fact_2 = m_e * m_epsilon * xi * r * sin_theta * fact_1 / divisor / divisor;
        const double fact_3 = m_e * xi / divisor;

        return 1 / r / fact_1 * (cos_theta * fact_2 + sin_theta * fact_3) / fact_3;
    }

    double inv_jacobian_22(ddc::Coordinate<R, Theta> const& coord) const final
    {
        const double r = ddc::get<R>(coord);
        const double theta = ddc::get<Theta>(coord);

        assert(r >= 1e-15);

        const double cos_theta = std::cos(theta);
        const double xi = std::sqrt(1. / (1. - m_epsilon * m_epsilon * 0.25));
        const double divisor = 2 - std::sqrt(1 + m_epsilon * (m_epsilon + 2.0 * r * cos_theta));

        const double fact_3 = m_e * xi / divisor;
        return 1 / r * cos_theta / fact_3;
    }


    /**
     * @brief  Compute the full Jacobian matrix from the mapping to the pseudo-Cartesian mapping at the central point.
     *
     *
     * The pseudo-Cartesian Jacobian matrix for a Czarny mapping is given by :
     * - @f$ (J_{\mathcal{F}}J_{\mathcal{G}}^{-1})^{-1}_{11}(0, \theta) = - \sqrt{1 + \varepsilon^2}, @f$
     * - @f$ (J_{\mathcal{F}}J_{\mathcal{G}}^{-1})^{-1}_{12}(0, \theta) = 0, @f$
     * - @f$ (J_{\mathcal{F}}J_{\mathcal{G}}^{-1})^{-1}_{21}(0, \theta) = 0, @f$
     * - @f$ (J_{\mathcal{F}}J_{\mathcal{G}}^{-1})^{-1}_{22}(0, \theta) = \frac{2 - \sqrt{1 + \varepsilon^2}}{e \xi}. @f$
     *
     *
     * @param[out] matrix
     *      The pseudo-Cartesian matrix at the central point evaluated.
     *
     *
     * @see DiscreteToCartesian
     * @see BslAdvection
     * @see AdvectionDomain
     */
    void to_pseudo_cartesian_jacobian_center_matrix(Matrix_2x2& matrix) const
    {
        const double xi = std::sqrt(1. / (1. - m_epsilon * m_epsilon * 0.25));
        const double sqrt_eps_2 = std::sqrt(1. + m_epsilon * m_epsilon);
        matrix[0][0] = -sqrt_eps_2;
        matrix[0][1] = 0.;
        matrix[1][0] = 0.;
        matrix[1][1] = (2 - sqrt_eps_2) / m_e / xi;
    }

    /**
     * @brief Compute the (1,1) coefficient of the pseudo-Cartesian Jacobian matrix at the central point.
     *
     * @f$ (J_{\mathcal{F}}J_{\mathcal{G}}^{-1})^{-1}_{11}(0, \theta) = - \sqrt{1 + \varepsilon^2}. @f$
     *
     * @return A double with the (1,1) coefficient of the pseudo-Cartesian Jacobian matrix at the central point.
     *
     * @see to_pseudo_cartesian_jacobian_center_matrix
     */
    double to_pseudo_cartesian_jacobian_11_center() const
    {
        return -std::sqrt(1 + m_epsilon * m_epsilon);
    }

    /**
     * @brief Compute the (1,2) coefficient of the pseudo-Cartesian Jacobian matrix at the central point.
     *
     * @f$ (J_{\mathcal{F}}J_{\mathcal{G}}^{-1})^{-1}_{12}(0, \theta) = 0. @f$
     *
     * @return A double with the (1,2) coefficient of the pseudo-Cartesian Jacobian matrix at the central point.
     *
     * @see to_pseudo_cartesian_jacobian_center_matrix
     */
    double to_pseudo_cartesian_jacobian_12_center() const
    {
        return 0;
    }

    /**
     * @brief Compute the (2,1) coefficient of the pseudo-Cartesian Jacobian matrix at the central point.
     *
     * @f$ (J_{\mathcal{F}}J_{\mathcal{G}}^{-1})^{-1}_{21}(0, \theta) = 0. @f$
     *
     * @return A double with the (2,1) coefficient of the pseudo-Cartesian Jacobian matrix at the central point.
     *
     * @see to_pseudo_cartesian_jacobian_center_matrix
     */
    double to_pseudo_cartesian_jacobian_21_center() const
    {
        return 0;
    }

    /**
     * @brief Compute the (2,2) coefficient of the pseudo-Cartesian Jacobian matrix at the central point.
     *
     * @f$ (J_{\mathcal{F}}J_{\mathcal{G}}^{-1})^{-1}_{22}(0, \theta) = \frac{2 - \sqrt{1 + \varepsilon^2}}{e \xi}. @f$
     *
     * @return A double with the (2,2) coefficient of the pseudo-Cartesian Jacobian matrix at the central point.
     *
     * @see to_pseudo_cartesian_jacobian_center_matrix
     */
    double to_pseudo_cartesian_jacobian_22_center() const
    {
        const double xi = std::sqrt(1. / (1. - m_epsilon * m_epsilon * 0.25));
        return (2 - std::sqrt(1 + m_epsilon * m_epsilon)) / m_e / xi;
    }
};
