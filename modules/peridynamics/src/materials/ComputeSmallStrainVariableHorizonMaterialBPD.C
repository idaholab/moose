//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeSmallStrainVariableHorizonMaterialBPD.h"

registerMooseObject("PeridynamicsApp", ComputeSmallStrainVariableHorizonMaterialBPD);

InputParameters
ComputeSmallStrainVariableHorizonMaterialBPD::validParams()
{
  InputParameters params = ComputeSmallStrainMaterialBaseBPD::validParams();
  params.addClassDescription("Class for computing peridynamic micro elastic modulus for bond-based "
                             "model using irregular mesh");

  return params;
}

ComputeSmallStrainVariableHorizonMaterialBPD::ComputeSmallStrainVariableHorizonMaterialBPD(
    const InputParameters & parameters)
  : ComputeSmallStrainMaterialBaseBPD(parameters)
{
}

void
ComputeSmallStrainVariableHorizonMaterialBPD::computePeridynamicsParams()
{
  _Cij = _dim * _dim * _bulk_modulus * (1.0 / _horizon_vol[0] + 1.0 / _horizon_vol[1]) /
         _origin_vec.norm();
}
