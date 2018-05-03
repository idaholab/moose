//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmallStrainMaterialBaseOSPD.h"

template <>
InputParameters
validParams<SmallStrainMaterialBaseOSPD>()
{
  InputParameters params = validParams<ParametricMaterialBasePD>();
  params.addClassDescription("Base class for ordinary state-based peridynamic mechanics models");

  return params;
}

SmallStrainMaterialBaseOSPD::SmallStrainMaterialBaseOSPD(const InputParameters & parameters)
  : ParametricMaterialBasePD(parameters),
    _bond_force_i_j(declareProperty<Real>("bond_force_i_j")),
    _bond_dfdU_i_j(declareProperty<Real>("bond_dfdU_i_j")),
    _bond_dfdT_i_j(declareProperty<Real>("bond_dfdT_i_j")),
    _bond_dfdE_i_j(declareProperty<Real>("bond_dfdE_i_j")),
    _d(2)
{
}

void
SmallStrainMaterialBaseOSPD::computeBondForce()
{
  // compute peridynamic micro-moduli: _a, _b, _d(2)
  computePDMicroModuli();

  // residual terms
  if (_scalar_out_of_plane_strain_coupled)
  {
    _bond_force_ij[_qp] = 2.0 * _b *
                          (_mechanical_stretch[_qp] +
                           _poissons_ratio * (_scalar_out_of_plane_strain[0] -
                                              _alpha * (0.5 * (_temp[0] + _temp[1]) - _temp_ref))) /
                          _origin_length * _nv[0] * _nv[1];
    _bond_force_i_j[_qp] =
        2.0 * _a * _d[_qp] * _d[_qp] *
        (_mechanical_stretch[_qp] + _alpha * (0.5 * (_temp[0] + _temp[1]) - _temp_ref) -
         _alpha * (_temp[_qp] - _temp_ref) +
         _poissons_ratio * (_scalar_out_of_plane_strain[0] - _alpha * (_temp[_qp] - _temp_ref))) *
        _nv[0] * _nv[1];
  }
  else
  {
    _bond_force_ij[_qp] = 2.0 * _b * _mechanical_stretch[_qp] / _origin_length * _nv[0] * _nv[1];
    _bond_force_i_j[_qp] =
        2.0 * _a * _d[_qp] * _d[_qp] *
        (_mechanical_stretch[_qp] + _alpha * (0.5 * (_temp[0] + _temp[1]) - _temp_ref) -
         _alpha * (_temp[_qp] - _temp_ref)) *
        _nv[0] * _nv[1];
  }

  // derivatives of residual terms
  _bond_dfdU_ij[_qp] = 2.0 * _b / _origin_length / _origin_length * _nv[0] * _nv[1];
  _bond_dfdU_i_j[_qp] = 2.0 * _a * _d[_qp] * _d[_qp] / _origin_length * _nv[0] * _nv[1];

  _bond_dfdE_ij[_qp] = 2.0 * _b * _poissons_ratio / _origin_length * _nv[0] * _nv[1];
  _bond_dfdE_i_j[_qp] = 2.0 * _a * _d[_qp] * _d[_qp] * _poissons_ratio * _nv[0] * _nv[1];

  if (_scalar_out_of_plane_strain_coupled)
  {
    _bond_dfdT_ij[_qp] =
        -2.0 * _b * (1.0 + _poissons_ratio) * 0.5 * _alpha / _origin_length * _nv[0] * _nv[1];
    _bond_dfdT_i_j[_qp] =
        -2.0 * _a * _d[_qp] * _d[_qp] * (1.0 + _poissons_ratio) * _alpha * _nv[0] * _nv[1];
  }
  else
  {
    _bond_dfdT_ij[_qp] = -2.0 * _b * 0.5 * _alpha / _origin_length * _nv[0] * _nv[1];
    _bond_dfdT_i_j[_qp] = -2.0 * _a * _d[_qp] * _d[_qp] * _alpha * _nv[0] * _nv[1];
  }
}
