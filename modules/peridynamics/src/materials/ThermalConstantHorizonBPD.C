//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalConstantHorizonBPD.h"
#include "MathUtils.h"

registerMooseObject("PeridynamicsApp", ThermalConstantHorizonBPD);

template <>
InputParameters
validParams<ThermalConstantHorizonBPD>()
{
  InputParameters params = validParams<ThermalMaterialBaseBPD>();
  params.addClassDescription(
      "Class for computing peridynamic micro conductivity for bond-based model "
      "using regular uniform mesh");

  return params;
}

ThermalConstantHorizonBPD::ThermalConstantHorizonBPD(const InputParameters & parameters)
  : ThermalMaterialBaseBPD(parameters)
{
}

void
ThermalConstantHorizonBPD::computePDMicroConductivity(const Real ave_thermal_conductivity)
{
  _Kij = 3.0 * ave_thermal_conductivity / M_PI *
         (1.0 / (MathUtils::pow(_horizon[0], _dim + 1)) +
          1.0 / (MathUtils::pow(_horizon[1], _dim + 1)));
}
