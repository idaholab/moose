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
 * Implements a limiter which reproduces a central-differencing scheme, defined by
 * $\beta(r_f) = 1$
 */
class CentralDifferenceLimiter : public Limiter
{
public:
  ADReal operator()(const ADReal &,
                    const ADReal &,
                    const ADRealVectorValue *,
                    const RealVectorValue &) const override final
  {
    return 1;
  }
  bool constant() const override final { return true; }

  CentralDifferenceLimiter() = default;
};
}
}
