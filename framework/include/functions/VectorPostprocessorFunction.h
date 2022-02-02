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

// Forward declarations
class VectorPostprocessorFunction;

template <>
InputParameters validParams<VectorPostprocessorFunction>();

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

  /// if the "component" parameter is specified, its value is assigned here and
  /// function values are interpolated W.R.T. spatial coordinates in that direction,
  /// otherwise, they are interpolated W.R.T time
  MooseEnum _deprecated; // index based access 0,1,2
  const MooseEnum & _component;
};
