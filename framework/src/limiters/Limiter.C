//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseEnum.h"
#include "VanLeerLimiter.h"
#include "UpwindLimiter.h"
#include "CentralDifferenceLimiter.h"
#include "MooseError.h"

#include "libmesh/auto_ptr.h"

namespace Moose
{
namespace FV
{
const MooseEnum moose_limiter_type("vanLeer=0 upwind=1 central_difference=2");

std::unique_ptr<Limiter>
Limiter::build(const LimiterType limiter)
{
  switch (limiter)
  {
    case LimiterType::VanLeer:
      return libmesh_make_unique<VanLeerLimiter>();

    case LimiterType::Upwind:
      return libmesh_make_unique<UpwindLimiter>();

    case LimiterType::CentralDifference:
      return libmesh_make_unique<CentralDifferenceLimiter>();

    default:
      mooseError("Unrecognized limiter type ", unsigned(limiter));
  }
}
}
}
