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
#include "MinModLimiter.h"
#include "SOULimiter.h"
#include "QUICKLimiter.h"
#include "VenkatakrishnanLimiter.h"
#include "MooseError.h"
#include "MathFVUtils.h"

#include <memory>

namespace Moose
{
namespace FV
{
const MooseEnum moose_limiter_type(
    "vanLeer=0 upwind=1 central_difference=2 min_mod=3 sou=4 quick=5 venkatakrishnan=6", "upwind");

template <typename T>
std::unique_ptr<Limiter<T>>
Limiter<T>::build(const LimiterType limiter)
{
  switch (limiter)
  {
    case LimiterType::VanLeer:
      return std::make_unique<VanLeerLimiter<T>>();

    case LimiterType::Upwind:
      return std::make_unique<UpwindLimiter<T>>();

    case LimiterType::CentralDifference:
      return std::make_unique<CentralDifferenceLimiter<T>>();

    case LimiterType::MinMod:
      return std::make_unique<MinModLimiter<T>>();

    case LimiterType::SOU:
      return std::make_unique<SOULimiter<T>>();

    case LimiterType::QUICK:
      return std::make_unique<QUICKLimiter<T>>();

    case LimiterType::Venkatakrishnan:
      return std::make_unique<VenkatakrishnanLimiter<T>>();

    default:
      mooseError("Unrecognized limiter type ", unsigned(limiter));
  }
}

LimiterType
limiterType(const InterpMethod interp_method)
{
  switch (interp_method)
  {
    case InterpMethod::Average:
    case InterpMethod::SkewCorrectedAverage:
      return LimiterType::CentralDifference;

    case InterpMethod::Upwind:
      return LimiterType::Upwind;

    case InterpMethod::VanLeer:
      return LimiterType::VanLeer;

    case InterpMethod::MinMod:
      return LimiterType::MinMod;

    case InterpMethod::SOU:
      return LimiterType::SOU;

    case InterpMethod::QUICK:
      return LimiterType::QUICK;

    case InterpMethod::Venkatakrishnan:
      return LimiterType::Venkatakrishnan;

    default:
      mooseError("Unrecognized interpolation method type.");
  }
}

// instantiations we care about
template std::unique_ptr<Limiter<Real>> Limiter<Real>::build(const LimiterType limiter);
template std::unique_ptr<Limiter<ADReal>> Limiter<ADReal>::build(const LimiterType limiter);
}
}
