#ifndef BSPLINES_NON_UNIFORM_H
#define BSPLINES_NON_UNIFORM_H
#include "bsplines.h"

class BSplines_non_uniform: public BSplines
{
    public:
        BSplines_non_uniform(int degree, bool periodic, std::vector<double> breaks);
        virtual void eval_basis(double x, mdspan_1d& values, int& jmin) const override;
        virtual void eval_deriv(double x, mdspan_1d& derivs, int& jmin) const override;
        virtual void eval_basis_and_n_derivs(double x, int n, mdspan_2d& derivs, int& jmin) const override;
        virtual void integrate(mdspan_1d& int_vals) const override;

        virtual double get_knot(int break_idx) const {
            // TODO: assert break_idx >= 1 - degree
            // TODO: assert break_idx <= npoints + degree
            return knots[break_idx+degree];
        }
        ~BSplines_non_uniform();
    protected:
        int find_cell(double x) const;

        inline double& get_knot(int break_idx) {
            // TODO: assert break_idx >= 1 - degree
            // TODO: assert break_idx <= npoints + degree
            return knots[break_idx+degree];
        }

        double* knots;
        int npoints;
        friend class Spline_interpolator_1D;
};
#endif //BSPLINES_NON_UNIFORM_H
