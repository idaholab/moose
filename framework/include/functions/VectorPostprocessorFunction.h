//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VectorPostprocessorFunction_H
#define VectorPostprocessorFunction_H

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
  VectorPostprocessorFunction(const InputParameters & parameters);
  virtual Real value(Real /*t*/, const Point & pt) override;

protected:
  std::unique_ptr<LinearInterpolation> _linear_interp;
  const unsigned int _component;
  const VectorPostprocessorValue & _argument_column;
  const VectorPostprocessorValue & _value_column;
};

#endif
