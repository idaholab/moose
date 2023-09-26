//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JouleHeatingHeatGeneratedAux.h"

registerMooseObject("HeatConductionApp", JouleHeatingHeatGeneratedAux);

InputParameters
JouleHeatingHeatGeneratedAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Compute heat generated from Joule heating "
                             "$(d\\vec{P}/d\\vec{V} = \\vec{E}^2 \\sigma )$.");
  params.addRequiredCoupledVar("elec", "Electric potential for joule heating.");
  params.addParam<MaterialPropertyName>(
      "electrical_conductivity",
      "electrical_conductivity",
      "Material property providing electrical conductivity of the material.");
  return params;
}

JouleHeatingHeatGeneratedAux::JouleHeatingHeatGeneratedAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _grad_elec(coupledGradient("elec")),
    _elec_cond(getMaterialProperty<Real>("electrical_conductivity"))
{
}

Real
JouleHeatingHeatGeneratedAux::computeValue()
{
  return _elec_cond[_qp] * _grad_elec[_qp] * _grad_elec[_qp];
}
