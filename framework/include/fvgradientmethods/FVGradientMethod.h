//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "MooseTypes.h"
#include "GradientLimiterType.h"

#include <memory>
#include <unordered_set>
#include <vector>

class SystemBase;

namespace libMesh
{
template <typename T>
class NumericVector;
}

/**
 * Registered base class for linear FV gradient methods.
 */
class FVGradientMethod : public MooseObject
{
public:
  using GradientContainer = std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>>;

  static InputParameters validParams();

  FVGradientMethod(const InputParameters & params);

  /// Compute unlimited gradients; the system applies limiterType() afterward if requested.
  virtual void computeGradient(SystemBase & system,
                               GradientContainer & output_gradient,
                               const std::unordered_set<unsigned int> & variable_numbers) const = 0;

  Moose::FV::GradientLimiterType limiterType() const { return _limiter_type; }

private:
  const Moose::FV::GradientLimiterType _limiter_type;
};
