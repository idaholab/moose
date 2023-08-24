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
#include <Eigen/Dense>

/**
 * A two-component zero initialized vector used for tangential quantities
 */
template <typename T>
class GenericTwoVector : public Eigen::Matrix<T, 2, 1>
{
public:
  GenericTwoVector() { this->setZero(); }
  using Eigen::Matrix<T, 2, 1>::Matrix;
};

using TwoVector = GenericTwoVector<Real>;
using ADTwoVector = GenericTwoVector<ADReal>;

namespace MetaPhysicL
{
// raw_value AD->non-AD conversion for ADReal valued Eigen::Matrix objects
template <>
struct RawType<ADTwoVector>
{
  typedef TwoVector value_type;

  static value_type value(const ADTwoVector & in)
  {
    return value_type::NullaryExpr([&in](Eigen::Index i) { return raw_value(in(i)); });
  }
};
}
