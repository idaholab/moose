//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialReaction.h"

#include "Material.h"

registerMooseObject("StochasticToolsTestApp", MaterialReaction);

InputParameters
MaterialReaction::validParams()
{
  InputParameters params = Reaction::validParams();
  params.addParam<MaterialPropertyName>(
      "coefficient", 1.0, "Name of the material property acting as reaction coefficient.");
  return params;
}

MaterialReaction::MaterialReaction(const InputParameters & parameters)
  : Reaction(parameters), _coeff(getMaterialProperty<Real>("coefficient"))
{
}

Real
MaterialReaction::computeQpResidual()
{
  return _coeff[_qp] * Reaction::computeQpResidual();
}

Real
MaterialReaction::computeQpJacobian()
{
  return _coeff[_qp] * Reaction::computeQpJacobian();
}
