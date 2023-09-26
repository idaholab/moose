//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADJouleHeatingSource.h"

registerMooseObject("HeatConductionApp", ADJouleHeatingSource);

InputParameters
ADJouleHeatingSource::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addCoupledVar("elec", "Electrostatic potential for joule heating.");
  params.addParam<MaterialPropertyName>(
      "electrical_conductivity",
      "electrical_conductivity",
      "Material property providing electrical conductivity of the material.");
  params.addClassDescription("Calculates the heat source term corresponding to electrostatic Joule "
                             "heating, with Jacobian contributions calculated using the automatic "
                             "differentiation system.");
  return params;
}

ADJouleHeatingSource::ADJouleHeatingSource(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _grad_elec(adCoupledGradient("elec")),
    _elec_cond(getADMaterialProperty<Real>("electrical_conductivity"))
{
}

ADReal
ADJouleHeatingSource::precomputeQpResidual()
{
  return -_elec_cond[_qp] * _grad_elec[_qp] * _grad_elec[_qp];
}
