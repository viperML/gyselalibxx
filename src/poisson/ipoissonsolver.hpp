#pragma once

#include <ddc/BlockSpan>

#include <geometry.h>

class IPoissonSolver
{
public:
    virtual DSpanX operator()(DSpanX electric_potential, DViewSpXVx fdistribu) const = 0;
};