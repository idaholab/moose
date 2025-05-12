//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADJouleHeatingSource.h"

registerMooseObject("HeatTransferApp", ADJouleHeatingSource);

InputParameters
ADJouleHeatingSource::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addCoupledVar("elec", "Electrostatic potential for joule heating.");
  params.addParam<MaterialPropertyName>(
      "heating_term", "electric_field_heating", "Material property providing the Joule heating.");
  params.addParam<MaterialPropertyName>(
      "electrical_conductivity",
      "electrical_conductivity",
      "Material property providing electrical conductivity of the material.");
  params.addClassDescription(
      "Calculates the heat source term corresponding to electrostatic or electromagnetic Joule "
      "heating, with Jacobian contributions calculated using the automatic "
      "differentiation system.");
  return params;
}

ADJouleHeatingSource::ADJouleHeatingSource(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _supplied_potential(isParamValid("elec")),

    _grad_potential(_supplied_potential ? adCoupledGradient("elec") : _ad_grad_zero),
    _elec_cond(_supplied_potential ? getADMaterialProperty<Real>("electrical_conductivity")
                                   : getGenericZeroMaterialProperty<Real, true>()),

    _heating_residual(_supplied_potential ? getGenericZeroMaterialProperty<Real, true>()
                                          : getADMaterialProperty<Real>("heating_term"))
{
  if (_supplied_potential)
    mooseDeprecated(
        "Directly coupling an electrostatic potential will be deprecated in the near future "
        "(10/01/2025). Please use the material object 'ElectromagneticHeatingMaterial' to couple "
        "either the electrostatic or electromagnetic field for Joule heating.");
}

ADReal
ADJouleHeatingSource::precomputeQpResidual()
{
  /*
   * NOTE: Coupling in the gradient of the potential will be deprecated in the
   *       near future (10/01/2025). After the deprecation, the residual contribution of this kernel will
   *       be solely provided by the 'ElectromagneticHeatingMaterial' material object.
   */
  if (_supplied_potential)
    return -_elec_cond[_qp] * _grad_potential[_qp] * _grad_potential[_qp];
  else
    return -_heating_residual[_qp];
}
