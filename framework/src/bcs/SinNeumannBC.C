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

#include "SinNeumannBC.h"

template <>
InputParameters
validParams<SinNeumannBC>()
{
  InputParameters params = validParams<IntegratedBC>();
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
