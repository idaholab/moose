//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMatInterfaceReaction.h"

registerMooseObject("MooseApp", ADMatInterfaceReaction);

InputParameters
ADMatInterfaceReaction::validParams()

{
  InputParameters params = ADInterfaceKernel::validParams();
  params.addParam<MaterialPropertyName>("forward_rate", "kf", "Forward reaction rate coefficient.");
  params.addParam<MaterialPropertyName>(
      "backward_rate", "kb", "Backward reaction rate coefficient.");
  params.addClassDescription("Implements a reaction to establish ReactionRate=k_f*u-k_b*v "
                             "at interface.");
  return params;
}

ADMatInterfaceReaction::ADMatInterfaceReaction(const InputParameters & parameters)
  : ADInterfaceKernel(parameters),
    _kf(getADMaterialProperty<Real>("forward_rate")),
    _kb(getNeighborADMaterialProperty<Real>("backward_rate"))
{
}

ADReal
ADMatInterfaceReaction::computeQpResidual(Moose::DGResidualType type)
{
  ADReal r = 0;
  switch (type)
  {
    // Move all the terms to the LHS to get residual, for primary domain
    // Residual = kf*u - kb*v = kf*u - kb*v
    // Weak form for primary domain is: (test, kf*u - kb*v)
    case Moose::Element:
      r = _test[_i][_qp] * (_kf[_qp] * _u[_qp] - _kb[_qp] * _neighbor_value[_qp]);
      break;

    // Similarly, weak form for secondary domain is: -(test, kf*u - kb*v),
    // flip the sign because the direction is opposite.
    case Moose::Neighbor:
      r = -_test_neighbor[_i][_qp] * (_kf[_qp] * _u[_qp] - _kb[_qp] * _neighbor_value[_qp]);
      break;
  }
  return r;
}
