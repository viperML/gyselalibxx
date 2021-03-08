#ifndef BSPLINES_UNIFORM_H
#define BSPLINES_UNIFORM_H
#include <vector>
#include "bsplines.h"

class BSplines_uniform: public BSplines
{
    public:
        BSplines_uniform(int degree, bool periodic, double xmin, double xmax, int ncells);
        virtual inline void eval_basis(double x, mdspan_1d& values, int& jmin) const override
            { return eval_basis(x, values, jmin, degree); }
        virtual void eval_deriv(double x, mdspan_1d& derivs, int& jmin) const override;
        virtual void eval_basis_and_n_derivs(double x, int n, mdspan_2d& derivs, int& jmin) const override;
        virtual void integrate(mdspan_1d& int_vals) const override;
        virtual double get_knot(int idx) const { return xmin + idx * dx; }
        ~BSplines_uniform();
    protected:
        void eval_basis(double x, mdspan_1d& values, int& jmin, int degree) const;
        void get_icell_and_offset(double x, int& icell, double& offset) const;
        double inv_dx;
        double dx;
};
#endif //BSPLINES_UNIFORM_H
