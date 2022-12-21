#pragma once

#include <geometry.hpp>

#include "irighthandside.hpp"

class KrookSourceConstant : public IRightHandSide
{
private:
    RhsType m_type;
    double m_extent;
    double m_stiffness;
    double m_amplitude;
    double m_density;
    double m_temperature;
    DFieldX m_mask;
    DFieldVx m_ftarget;

public:
    KrookSourceConstant(
            IDomainX const& gridx,
            IDomainVx const& gridv,
            RhsType const type,
            double const extent,
            double const stiffness,
            double const amplitude,
            double const density,
            double const temperature);

    KrookSourceConstant(KrookSourceConstant&&) = default;

    ~KrookSourceConstant() override = default;

    DSpanSpXVx operator()(DSpanSpXVx allfdistribu, double dt) const override;
};