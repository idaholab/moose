//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosHeatConductionBC.h"

registerKokkosResidualObject("HeatTransferApp", KokkosHeatConductionBC);

InputParameters
KokkosHeatConductionBC::validParams()
{
  InputParameters params = IntegratedBCValue::validParams();
  params.addClassDescription("Boundary condition for a diffusive heat flux.");
  return params;
}

KokkosHeatConductionBC::KokkosHeatConductionBC(const InputParameters & parameters)
  : IntegratedBCValue(parameters), _k(getKokkosMaterialPropertyByName<Real>("thermal_conductivity"))
{
}
