//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalVariableHorizonBPD.h"

registerMooseObject("PeridynamicsApp", ThermalVariableHorizonBPD);

template <>
InputParameters
validParams<ThermalVariableHorizonBPD>()
{
  InputParameters params = validParams<ThermalMaterialBaseBPD>();
  params.addClassDescription("Class for computing peridynamic micro conductivity for bond-based "
                             "model using irregular mesh");

  return params;
}

ThermalVariableHorizonBPD::ThermalVariableHorizonBPD(const InputParameters & parameters)
  : ThermalMaterialBaseBPD(parameters)
{
}

void
ThermalVariableHorizonBPD::computePDMicroConductivity(const Real ave_thermal_conductivity)
{
  _Kij = _dim * ave_thermal_conductivity * (1.0 / _nvsum[0] + 1.0 / _nvsum[1]) / _origin_length;
}
