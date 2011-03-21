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

#include "TEJumpFFN.h"

template<>
InputParameters validParams<TEJumpFFN>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<double>("t_jump", 1.0, "Time when the jump occurs");
  params.addParam<double>("slope", 1.0, "How steep the jump is");
  return params;
}

TEJumpFFN::TEJumpFFN(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _t_jump(getParam<Real>("t_jump")),
    _slope(getParam<Real>("slope"))
{
}

Real
TEJumpFFN::computeQpResidual()
{
  return -_test[_i][_qp] * (_slope * libMesh::pi)/(1 + _slope*_slope*libMesh::pi*libMesh::pi*std::pow(_t_jump - _t, 2));
}

Real
TEJumpFFN::computeQpJacobian()
{
  return 0;
}
