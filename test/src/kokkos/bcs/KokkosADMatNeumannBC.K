//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADMatNeumannBC.h"

registerKokkosADResidualObject("MooseTestApp", KokkosADMatNeumannBC);

InputParameters
KokkosADMatNeumannBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addClassDescription(
      "Test AD integrated BC with the flux k u given by a pointwise AD material property k.");
  params.addRequiredParam<MaterialPropertyName>(
      "property", "The pointwise AD material property multiplying the BC variable in the flux");
  return params;
}

KokkosADMatNeumannBC::KokkosADMatNeumannBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _u(adValue()), _k(getKokkosADMaterialProperty<Real>("property"))
{
}
