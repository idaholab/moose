//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmallStrainVariableHorizonBPD.h"

registerMooseObject("PeridynamicsApp", SmallStrainVariableHorizonBPD);

template <>
InputParameters
validParams<SmallStrainVariableHorizonBPD>()
{
  InputParameters params = validParams<SmallStrainMaterialBaseBPD>();
  params.addClassDescription("Class for computing peridynamic micro elastic modulus for bond-based "
                             "model using irregular mesh");

  return params;
}

SmallStrainVariableHorizonBPD::SmallStrainVariableHorizonBPD(const InputParameters & parameters)
  : SmallStrainMaterialBaseBPD(parameters)
{
}

void
SmallStrainVariableHorizonBPD::computePDMicroModuli()
{
  _Cij = _dim * _dim * _bulk_modulus * (1.0 / _nvsum[0] + 1.0 / _nvsum[1]) / _origin_length;
}
