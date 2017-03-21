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
#include "TEJumpBC.h"

template <>
InputParameters
validParams<TEJumpBC>()
{
  InputParameters params = validParams<NodalBC>();
  params.addParam<Real>("value", 0.0, "The value the variable should have on the boundary");
  params.addParam<double>("t_jump", 1.0, "Time when the jump occurs");
  params.addParam<double>("slope", 1.0, "How steep the jump is");
  return params;
}

TEJumpBC::TEJumpBC(const InputParameters & parameters)
  : NodalBC(parameters), _t_jump(getParam<Real>("t_jump")), _slope(getParam<Real>("slope"))
{
}

Real
TEJumpBC::computeQpResidual()
{
  return _u[_qp] - (atan((_t - _t_jump) * libMesh::pi * _slope));
}
