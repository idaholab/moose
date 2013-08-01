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

#include "SplineFFn.h"

template<>
InputParameters validParams<SplineFFn>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<FunctionName>("function", "The name of the spline function");

  return params;
}

SplineFFn::SplineFFn(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _fn(dynamic_cast<SplineFunction &>(getFunction("function")))
{
}

SplineFFn::~SplineFFn()
{
}

Real
SplineFFn::computeQpResidual()
{
  return _test[_i][_qp] * _fn.secondDerivative(_q_point[_qp]);
}
