/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SplitCHWRes.h"
template<>
InputParameters validParams<SplitCHWRes>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<std::string>("mob_name", "mobtemp", "The mobility used with the kernel");
  return params;
}

SplitCHWRes::SplitCHWRes(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _mob_name(getParam<std::string>("mob_name")),
    _mob(getMaterialProperty<Real>(_mob_name))
{
}

Real
SplitCHWRes::computeQpResidual()
{
  return _mob[_qp] * _grad_u[_qp] * _grad_test[_i][_qp];
}

Real
SplitCHWRes::computeQpJacobian()
{
  return _mob[_qp] * _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}
