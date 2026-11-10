//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosSinDirichletBC.h"

registerKokkosResidualObject("MooseApp", KokkosSinDirichletBC);

InputParameters
KokkosSinDirichletBC::validParams()
{
  InputParameters params = NodalBC::validParams();
  params.addParam<Real>("initial", 0, "Initial value");
  params.addParam<Real>("final", 0, "Final value");
  params.addParam<Real>("duration", 0, "Duration over which the value is changing");
  params.addClassDescription(
      "Imposes a time-varying essential boundary condition $u=g(t)$, where $g(t)$ "
      "varies from an given initial value at time $t=0$ to a given final value over a specified "
      "duration.");
  return params;
}

KokkosSinDirichletBC::KokkosSinDirichletBC(const InputParameters & parameters)
  : NodalBC(parameters),
    _initial(getParam<Real>("initial")),
    _final(getParam<Real>("final")),
    _duration(getParam<Real>("duration")),
    _omega((0.5 / _duration) * libMesh::pi)
{
}
