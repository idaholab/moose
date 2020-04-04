//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SinNeumannBC.h"

registerMooseObject("MooseApp", SinNeumannBC);

InputParameters
SinNeumannBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addParam<Real>("initial", 0.0, "The initial value of the gradient on the boundary");
  params.addParam<Real>("final", 0.0, "The final value of the gradient on the boundary");
  params.addParam<Real>("duration", 0.0, "The duration of the ramp");
  params.addClassDescription("Imposes a time-varying flux boundary condition $\\frac{\\partial "
                             "u}{\\partial n}=g(t)$, where $g(t)$ "
                             "varies from an given initial value at time $t=0$ to a given final "
                             "value over a specified duration.");
  return params;
}

SinNeumannBC::SinNeumannBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _initial(getParam<Real>("initial")),
    _final(getParam<Real>("final")),
    _duration(getParam<Real>("duration"))
{
}

Real
SinNeumannBC::computeQpResidual()
{
  Real value;

  if (_t < _duration)
    value = _initial + (_final - _initial) * std::sin((0.5 / _duration) * libMesh::pi * _t);
  else
    value = _final;

  return -_test[_i][_qp] * value;
}
