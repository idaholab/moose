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

#include "BicubicSplineFFn.h"

template<>
InputParameters validParams<BicubicSplineFFn>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<FunctionName>("function", "The name of the bicubic spline function");

  return params;
}

BicubicSplineFFn::BicubicSplineFFn(const InputParameters & parameters) :
    Kernel(parameters),
    _fn(dynamic_cast<BicubicSplineFunction &>(getFunction("function")))
{
}

BicubicSplineFFn::~BicubicSplineFFn()
{
}

Real
BicubicSplineFFn::computeQpResidual()
{
  return _test[_i][_qp] * (_fn.secondDerivative(_q_point[_qp], 1)
                           + _fn.secondDerivative(_q_point[_qp], 2));
}
