//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PotentialToFieldAux.h"

registerMooseObject("ElectromagneticsApp", PotentialToFieldAux);

InputParameters
PotentialToFieldAux::validParams()
{
  InputParameters params = VariableGradientComponent::validParams();
  params.addClassDescription("An AuxKernel that calculates the electrostatic electric field given "
                             "the electrostatic potential.");
  MooseEnum sign("positive=1 negative=-1", "negative");
  params.addParam<MooseEnum>("sign", sign, "Sign of potential gradient.");
  return params;
}

PotentialToFieldAux::PotentialToFieldAux(const InputParameters & parameters)
  : VariableGradientComponent(parameters), _sign(getParam<MooseEnum>("sign"))
{
}

Real
PotentialToFieldAux::computeValue()
{
  return _sign * VariableGradientComponent::computeValue();
}
