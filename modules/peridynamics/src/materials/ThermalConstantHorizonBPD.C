//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalConstantHorizonBPD.h"

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
ThermalConstantHorizonBPD::computePDMicroConductivity()
{
  _Kij = (6.0 * _kappa / (3.14159265358 * std::pow(_horizon[0], _dim + 1)) +
          6.0 * _kappa / (3.14159265358 * std::pow(_horizon[1], _dim + 1))) /
         2.0;
}
