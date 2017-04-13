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
#include "DirichletBCfuncXYZ0.h"

template <>
InputParameters
validParams<DirichletBCfuncXYZ0>()
{
  InputParameters params = validParams<NodalBC>();

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
