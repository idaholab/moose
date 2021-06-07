//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Limiter.h"

namespace Moose
{
namespace FV
{
/**
 * Implements the Van Leer limiter, defined by
 * $\beta(r_f) = \frac{r_f + \text{abs}(r_f)}{1 + \text{abs}(r_f)}$
 */
class VanLeerLimiter : public Limiter
{
public:
  ADReal operator()(const ADReal & r_f) const override final
  {
    return (r_f + std::abs(r_f)) / (1. + std::abs(r_f));
  }

  VanLeerLimiter() = default;
};
}
}
