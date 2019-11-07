//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DirichletBCfuncXYZ0.h"

registerMooseObject("MooseTestApp", DirichletBCfuncXYZ0);

InputParameters
DirichletBCfuncXYZ0::validParams()
{
  InputParameters params = NodalBC::validParams();

  params.set<Real>("A0") = 0.;
  params.set<Real>("B0") = 0.;
  params.set<Real>("C0") = 0.;
  params.set<Real>("omega0") = 1.;

  params.set<bool>("_integrated") = false;
  return params;
}

DirichletBCfuncXYZ0::DirichletBCfuncXYZ0(const InputParameters & parameters)
  : NodalBC(parameters),
    _A0(getParam<Real>("A0")),
    _B0(getParam<Real>("B0")),
    _C0(getParam<Real>("C0")),
    _omega0(getParam<Real>("omega0"))
{
}

Real
DirichletBCfuncXYZ0::computeQpResidual()
{
  return _u[_qp] - ManSol4ADR1(*_current_node, _A0, _B0, _C0, _omega0, _t, _is_transient);
}
