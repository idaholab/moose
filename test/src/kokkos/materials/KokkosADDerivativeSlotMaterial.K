//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADDerivativeSlotMaterial.h"
#include "KokkosADMaterialProperty.h"

registerKokkosMaterial("MooseTestApp", KokkosADDerivativeSlotMaterial);

InputParameters
KokkosADDerivativeSlotMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Test material copying one stored derivative of a pointwise AD property into a property.");
  params.addRequiredParam<MaterialPropertyName>("ad_property", "The pointwise AD property");
  params.addRequiredParam<unsigned int>("slot", "The slot of the copied derivative");
  params.addRequiredParam<MaterialPropertyName>("property_name",
                                                "The name of the property holding the derivative");
  return params;
}

KokkosADDerivativeSlotMaterial::KokkosADDerivativeSlotMaterial(const InputParameters & parameters)
  : Material(parameters),
    _derivatives(getKokkosMaterialPropertyByName<Real, 1>(
        Moose::Kokkos::adDerivativesPropertyName(getParam<MaterialPropertyName>("ad_property")))),
    _slot(getParam<unsigned int>("slot")),
    _derivative(declareKokkosProperty<Real>("property_name"))
{
}
