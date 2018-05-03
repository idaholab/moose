//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmallStrainConstantHorizonOSPD.h"

registerMooseObject("PeridynamicsApp", SmallStrainConstantHorizonOSPD);

template <>
InputParameters
validParams<SmallStrainConstantHorizonOSPD>()
{
  InputParameters params = validParams<SmallStrainMaterialBaseOSPD>();
  params.addClassDescription(
      "Class for computing peridynamic micro elastic moduli for ordinary state-based model "
      "using regular uniform mesh");

  return params;
}

SmallStrainConstantHorizonOSPD::SmallStrainConstantHorizonOSPD(const InputParameters & parameters)
  : SmallStrainMaterialBaseOSPD(parameters)
{
}

void
SmallStrainConstantHorizonOSPD::computePDMicroModuli()
{
  _a = 0.5 * (_bulk_modulus - (8.0 - _dim) / 3.0 * _shear_modulus);

  // _b = 2 * _b * _horizon_(i/j) * _origin_length //_origin_length will be cancelled out in parent
  // material model
  _b = _origin_length * (3.0 * _dim + 6.0) * _shear_modulus / 3.1415926 /
       std::pow(_horizon[0], _dim + 1);

  // _d_i = _di * _horizon_(i/j)
  _d[0] = (_dim / 4.0 + 1.5) / 3.1415926 / std::pow(_horizon[0], _dim);
  _d[1] = _d[0];
}
