//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADRealForward.h"
#include "DualRealOps.h"

namespace Eigen
{
namespace internal
{
template <typename V, typename D, bool asd>
inline bool
isinf_impl(const MetaPhysicL::DualNumber<V, D, asd> & a)
{
  return std::isinf(a);
}

template <typename V, typename D, bool asd>
inline bool
isnan_impl(const MetaPhysicL::DualNumber<V, D, asd> & a)
{
  return std::isnan(a);
}

template <typename V, typename D, bool asd>
inline MetaPhysicL::DualNumber<V, D, asd>
sqrt(const MetaPhysicL::DualNumber<V, D, asd> & a)
{
  return std::sqrt(a);
}

template <typename V, typename D, bool asd>
inline MetaPhysicL::DualNumber<V, D, asd>
abs(const MetaPhysicL::DualNumber<V, D, asd> & a)
{
  return std::abs(a);
}
}
} // namespace Eigen

// this include _must_ come after the Eigen::internal overloads above. We also ignore a warning
// about an Eigen internal use of a potentially uninitialized variable
#include "libmesh/ignore_warnings.h"
#include <Eigen/Core>
#include "libmesh/restore_warnings.h"

// Eigen needs this
namespace MetaPhysicL
{
// raw_value AD->non-AD conversion for ADReal valued Eigen::Matrix objects
template <typename T, int M, int N, int O, int M2, int N2>
struct RawType<Eigen::Matrix<T, M, N, O, M2, N2>>
{
  typedef Eigen::Matrix<typename RawType<T>::value_type, M, N, O, M2, N2> value_type;

  static value_type value(const Eigen::Matrix<T, M, N, O, M2, N2> & in)
  {
    return value_type::NullaryExpr([&in](Eigen::Index i) { return raw_value(in(i)); });
  }
};

// raw_value overload for Map type objects that forces evaluation
template <typename T>
auto
raw_value(const Eigen::Map<T> & in)
{
  return raw_value(in.eval());
}
} // namespace MetaPhysicL

namespace Eigen
{
// libEigen support for dual number types
template <typename V, typename D, bool asd>
struct NumTraits<MetaPhysicL::DualNumber<V, D, asd>>
  : NumTraits<V> // permits to get the epsilon, dummy_precision, lowest, highest functions
{
  typedef MetaPhysicL::DualNumber<V, D, asd> Real;
  typedef MetaPhysicL::DualNumber<V, D, asd> NonInteger;
  typedef MetaPhysicL::DualNumber<V, D, asd> Nested;

  enum
  {
    IsComplex = 0,
    IsInteger = 0,
    IsSigned = 1,
    RequireInitialization = 1,
    ReadCost = HugeCost,
    AddCost = HugeCost,
    MulCost = HugeCost
  };
};

template <typename BinaryOp, typename V, typename D, bool asd>
struct ScalarBinaryOpTraits<Real, MetaPhysicL::DualNumber<V, D, asd>, BinaryOp>
{
  typedef MetaPhysicL::DualNumber<V, D, asd> ReturnType;
};
template <typename BinaryOp, typename V, typename D, bool asd>
struct ScalarBinaryOpTraits<MetaPhysicL::DualNumber<V, D, asd>, Real, BinaryOp>
{
  typedef MetaPhysicL::DualNumber<V, D, asd> ReturnType;
};
} // namespace Eigen
