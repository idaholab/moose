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
class MinModLimiter : public Limiter
{
public:
  ADReal operator()(const ADReal & r_f) const override final
  {
    // Dummy addition to avoid new nonzeros
    return 0 * r_f + std::max(0, std::min(1, r_f));
  }

  MinModLimiter() = default;
};
}
}
