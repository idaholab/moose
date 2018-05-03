//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmallStrainConstantHorizonBPD.h"

registerMooseObject("PeridynamicsApp", SmallStrainConstantHorizonBPD);

template <>
InputParameters
validParams<SmallStrainConstantHorizonBPD>()
{
  InputParameters params = validParams<SmallStrainMaterialBaseBPD>();
  params.addClassDescription("Class for computing peridynamic micro elastic modulus for bond-based "
                             "model using regular uniform mesh");

  return params;
}

SmallStrainConstantHorizonBPD::SmallStrainConstantHorizonBPD(const InputParameters & parameters)
  : SmallStrainMaterialBaseBPD(parameters)
{
}

void
SmallStrainConstantHorizonBPD::computePDMicroModuli()
{
  _Cij = 0.5 * (6.0 * _dim * _bulk_modulus / (3.14159265358 * std::pow(_horizon[0], _dim + 1)) +
                6.0 * _dim * _bulk_modulus / (3.14159265358 * std::pow(_horizon[1], _dim + 1)));
}
