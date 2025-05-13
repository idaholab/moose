//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JouleHeatingHeatGeneratedAux.h"

registerMooseObject("HeatTransferApp", JouleHeatingHeatGeneratedAux);

InputParameters
JouleHeatingHeatGeneratedAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Compute heat generated from Joule heating.");
  params.addParam<MaterialPropertyName>(
      "heating_term", "electric_field_heating", "Material property providing the Joule heating.");
  params.addCoupledVar("elec", "Electric potential for Joule heating.");
  params.addParam<MaterialPropertyName>(
      "electrical_conductivity",
      "electrical_conductivity",
      "Material property providing electrical conductivity of the material.");
  return params;
}

JouleHeatingHeatGeneratedAux::JouleHeatingHeatGeneratedAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _supplied_potential(isParamValid("elec")),

    _grad_elec(coupledGradient("elec")),
    _elec_cond(_supplied_potential ? (hasMaterialProperty<Real>("electrical_conductivity")
                                          ? &getMaterialProperty<Real>("electrical_conductivity")
                                          : nullptr)
                                   : nullptr),
    _ad_elec_cond(
        _supplied_potential
            ? (!_elec_cond ? &getADMaterialProperty<Real>("electrical_conductivity") : nullptr)
            : nullptr),

    _heating_residual(_supplied_potential ? getGenericZeroMaterialProperty<Real, true>()
                                          : getADMaterialProperty<Real>("heating_term"))
{
  if (_supplied_potential)
    mooseDeprecated(
        "Directly coupling an electrostatic potential will be deprecated in the near future "
        "(10/01/2025). Please use the material object 'ElectromagneticHeatingMaterial' to coupled "
        "either the electrostatic or electromagnetic field for Joule heating.");
}

Real
JouleHeatingHeatGeneratedAux::computeValue()
{
  /*
   * NOTE: Coupling in the gradient of the potential will be deprecated in the
   *       near future (10/01/2025). After the deprecation, the residual contribution of this kernel
   * will be solely provided by the 'ElectromagneticHeatingMaterial' material object.
   */
  if (_supplied_potential)
  {
    const Real elec_coef =
        _elec_cond ? (*_elec_cond)[_qp] : MetaPhysicL::raw_value((*_ad_elec_cond)[_qp]);
    return elec_coef * _grad_elec[_qp] * _grad_elec[_qp];
  }
  else
    return MetaPhysicL::raw_value(_heating_residual[_qp]);
}
