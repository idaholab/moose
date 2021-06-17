//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalConstantHorizonMaterialBPD.h"
#include "MathUtils.h"

registerMooseObject("PeridynamicsApp", ThermalConstantHorizonMaterialBPD);

InputParameters
ThermalConstantHorizonMaterialBPD::validParams()
{
  InputParameters params = ThermalMaterialBaseBPD::validParams();
  params.addClassDescription(
      "Class for computing peridynamic micro conductivity for bond-based model "
      "using regular uniform mesh");

  return params;
}

ThermalConstantHorizonMaterialBPD::ThermalConstantHorizonMaterialBPD(
    const InputParameters & parameters)
  : ThermalMaterialBaseBPD(parameters)
{
}

void
ThermalConstantHorizonMaterialBPD::computePeridynamicsParams(const Real ave_thermal_conductivity)
{
  _Kij = 3.0 * ave_thermal_conductivity / M_PI *
         (1.0 / (MathUtils::pow(_horizon_radius[0], _dim + 1)) +
          1.0 / (MathUtils::pow(_horizon_radius[1], _dim + 1)));
}
