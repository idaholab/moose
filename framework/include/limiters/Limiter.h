//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADReal.h"
#include "MooseTypes.h"
#include "HasMembers.h"
#include <memory>

class MooseEnum;

namespace Moose
{
namespace FV
{
enum class InterpMethod;

enum class LimiterType : int
{
  VanLeer = 0,
  Upwind,
  CentralDifference,
  MinMod,
  SOU,
  QUICK
};
extern const MooseEnum moose_limiter_type;

template <typename T, typename Enable = void>
struct LimiterValueType;

template <>
struct LimiterValueType<Real>
{
  typedef Real value_type;
};
template <>
struct LimiterValueType<ADReal>
{
  typedef ADReal value_type;
};
template <typename T>
struct LimiterValueType<T, typename std::enable_if<HasMemberType_value_type<T>::value>::type>
{
  typedef typename T::value_type value_type;
};

/**
 * Base class for defining slope limiters for finite volume or potentially reconstructed
 * Discontinuous-Galerkin applications
 */
template <typename T>
class Limiter
{
public:
  /**
   * Defines the slope limiter function $\beta(r_f)$ where $r_f$ represents the ratio of upstream to
   * downstream gradients
   */
  virtual T operator()(const T & phi_upwind,
                       const T & phi_downwind,
                       const VectorValue<T> * grad_phi_upwind,
                       const RealVectorValue & dCD) const = 0;
  virtual bool constant() const = 0;
  virtual InterpMethod interpMethod() const = 0;

  Limiter() = default;

  virtual ~Limiter() = default;

  static std::unique_ptr<Limiter> build(LimiterType limiter);
};

/**
 * Return the limiter type associated with the supplied interpolation method
 */
LimiterType limiterType(InterpMethod interp_method);
}
}
