//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SinDirichletBC.h"

registerMooseObject("MooseApp", SinDirichletBC);

InputParameters
SinDirichletBC::validParams()
{
  InputParameters params = NodalBC::validParams();
  params.set<Real>("initial") = 0.0;
  params.set<Real>("final") = 0.0;
  params.set<Real>("duration") = 0.0;
  params.addClassDescription(
      "Imposes a time-varying essential boundary condition $u=g(t)$, where $g(t)$ "
      "varies from an given initial value at time $t=0$ to a given final value over a specified "
      "duration.");
  return params;
}

SinDirichletBC::SinDirichletBC(const InputParameters & parameters)
  : NodalBC(parameters),
    _initial(getParam<Real>("initial")),
    _final(getParam<Real>("final")),
    _duration(getParam<Real>("duration"))
{
}

Real
SinDirichletBC::computeQpResidual()
{
  Real value;

  if (_t < _duration)
    value = _initial + (_final - _initial) * std::sin((0.5 / _duration) * libMesh::pi * _t);
  else
    value = _final;

  return _u[_qp] - value;
}
