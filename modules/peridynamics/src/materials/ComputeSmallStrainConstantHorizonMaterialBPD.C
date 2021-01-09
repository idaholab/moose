//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeSmallStrainConstantHorizonMaterialBPD.h"

registerMooseObject("PeridynamicsApp", ComputeSmallStrainConstantHorizonMaterialBPD);

InputParameters
ComputeSmallStrainConstantHorizonMaterialBPD::validParams()
{
  InputParameters params = ComputeSmallStrainMaterialBaseBPD::validParams();
  params.addClassDescription("Class for computing peridynamic micro elastic modulus for bond-based "
                             "model using regular uniform mesh");

  return params;
}

ComputeSmallStrainConstantHorizonMaterialBPD::ComputeSmallStrainConstantHorizonMaterialBPD(
    const InputParameters & parameters)
  : ComputeSmallStrainMaterialBaseBPD(parameters)
{
}

void
ComputeSmallStrainConstantHorizonMaterialBPD::computePeridynamicsParams()
{
  _Cij = 0.5 * (6.0 * _dim * _bulk_modulus / (M_PI * std::pow(_horizon_radius[0], _dim + 1)) +
                6.0 * _dim * _bulk_modulus / (M_PI * std::pow(_horizon_radius[1], _dim + 1)));
}
