//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmallStrainMaterialBaseBPD.h"

template <>
InputParameters
validParams<SmallStrainMaterialBaseBPD>()
{
  InputParameters params = validParams<ParametricMaterialBasePD>();
  params.addClassDescription("Base class for bond-based peridynamic mechanics models");

  return params;
}

SmallStrainMaterialBaseBPD::SmallStrainMaterialBaseBPD(const InputParameters & parameters)
  : ParametricMaterialBasePD(parameters)
{
}

void
SmallStrainMaterialBaseBPD::computeBondForce()
{
  // compute the peridynamic micro-modulus: _Cij
  computePDMicroModuli();

  // residual terms
  if (_scalar_out_of_plane_strain_coupled)
    _bond_force_ij[_qp] = _Cij *
                          (_mechanical_stretch[_qp] +
                           _poissons_ratio * (_scalar_out_of_plane_strain[0] -
                                              _alpha * (0.5 * (_temp[0] + _temp[1]) - _temp_ref))) *
                          _nv[0] * _nv[1];
  else
    _bond_force_ij[_qp] = _Cij * _mechanical_stretch[_qp] * _nv[0] * _nv[1];

  // derivatives of residual terms
  _bond_dfdU_ij[_qp] = _Cij / _origin_length * _nv[0] * _nv[1];
  _bond_dfdE_ij[_qp] = _Cij * _poissons_ratio * _nv[0] * _nv[1];

  if (_scalar_out_of_plane_strain_coupled)
    _bond_dfdT_ij[_qp] = -_Cij * (1.0 + _poissons_ratio) * 0.5 * _alpha * _nv[0] * _nv[1];
  else
    _bond_dfdT_ij[_qp] = -_Cij * 0.5 * _alpha * _nv[0] * _nv[1];
}
