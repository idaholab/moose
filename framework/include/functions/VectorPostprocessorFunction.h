//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"
#include "LinearInterpolation.h"
#include "VectorPostprocessorInterface.h"

/**
 * Function which provides a piecewise continuous linear interpolation
 * of a data set provided as two columns of a VectorPostprocessor.
 */
class VectorPostprocessorFunction : public Function, public VectorPostprocessorInterface
{
public:
  static InputParameters validParams();

  VectorPostprocessorFunction(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;
  virtual ADReal value(const ADReal & t, const ADPoint & p) const override;

protected:
  std::unique_ptr<LinearInterpolation> _linear_interp;
  const VectorPostprocessorValue & _argument_column;
  const VectorPostprocessorValue & _value_column;

  template <typename T, typename P>
  T valueInternal(const T & t, const P & p) const;

  const MooseEnum & _component;

  /// used to get the current linear/non-linear iteration number
  FEProblemBase & _fe_problem;

  /// last iteration during which the linear interpolation was rebuilt
  mutable std::tuple<Real, unsigned int, unsigned int> _last_update;
};
