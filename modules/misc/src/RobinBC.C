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

#include "RobinBC.h"

template<>
InputParameters validParams<RobinBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addParam<Real>("value", 0.0, "The value of the gradient on the boundary.");
  return params;
}

RobinBC::RobinBC(const std::string & name, InputParameters parameters) :
  IntegratedBC(name, parameters),
  _value(parameters.get<Real>("value"))
{
}

Real
RobinBC::computeQpResidual()
{
  return (_grad_u[_qp])(1)*_test[_i][_qp] + _u[_qp] - _value;
}

