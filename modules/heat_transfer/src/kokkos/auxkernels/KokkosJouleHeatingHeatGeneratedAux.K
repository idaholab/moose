//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosJouleHeatingHeatGeneratedAux.h"

registerKokkosAuxKernel("HeatTransferApp", KokkosJouleHeatingHeatGeneratedAux);

InputParameters
KokkosJouleHeatingHeatGeneratedAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Compute heat generated from Joule heating.");
  params.addParam<MaterialPropertyName>(
      "heating_term", "electric_field_heating", "Material property providing the Joule heating.");
  return params;
}

KokkosJouleHeatingHeatGeneratedAux::KokkosJouleHeatingHeatGeneratedAux(
    const InputParameters & parameters)
  : AuxKernel(parameters), _heating_residual(getKokkosMaterialProperty<Real>("heating_term"))
{
}
