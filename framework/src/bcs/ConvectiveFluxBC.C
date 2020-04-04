//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConvectiveFluxBC.h"

registerMooseObject("MooseApp", ConvectiveFluxBC);

InputParameters
ConvectiveFluxBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.set<Real>("rate") = 7500;
  params.set<Real>("initial") = 500;
  params.set<Real>("final") = 500;
  params.set<Real>("duration") = 0.0;
  params.addClassDescription(
      "Determines boundary values via the initial and final values, flux, and exposure duration");
  return params;
}

ConvectiveFluxBC::ConvectiveFluxBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _initial(getParam<Real>("initial")),
    _final(getParam<Real>("final")),
    _rate(getParam<Real>("rate")),
    _duration(getParam<Real>("duration"))
{
}

Real
ConvectiveFluxBC::computeQpResidual()
{
  Real value;

  if (_t < _duration)
    value = _initial + (_final - _initial) * std::sin((0.5 / _duration) * libMesh::pi * _t);
  else
    value = _final;

  return -(_test[_i][_qp] * _rate * (value - _u[_qp]));
}

Real
ConvectiveFluxBC::computeQpJacobian()
{
  return -(_test[_i][_qp] * _rate * (-_phi[_j][_qp]));
}
