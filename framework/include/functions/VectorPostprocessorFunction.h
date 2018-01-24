/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
