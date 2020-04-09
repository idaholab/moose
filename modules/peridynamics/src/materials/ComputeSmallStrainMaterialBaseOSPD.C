//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeSmallStrainMaterialBaseOSPD.h"

InputParameters
ComputeSmallStrainMaterialBaseOSPD::validParams()
{
  InputParameters params = ParametricMaterialBasePD::validParams();
  params.addClassDescription("Base class for ordinary state-based peridynamic mechanics models");

  return params;
}

ComputeSmallStrainMaterialBaseOSPD::ComputeSmallStrainMaterialBaseOSPD(
    const InputParameters & parameters)
  : ParametricMaterialBasePD(parameters),
    _bond_nonlocal_force(declareProperty<Real>("bond_nonlocal_force")),
    _bond_nonlocal_dfdU(declareProperty<Real>("bond_nonlocal_dfdU")),
    _bond_nonlocal_dfdT(declareProperty<Real>("bond_nonlocal_dfdT")),
    _bond_nonlocal_dfdE(declareProperty<Real>("bond_nonlocal_dfdE")),
    _d(2)
{
}

void
ComputeSmallStrainMaterialBaseOSPD::computeBondForce()
{
  if (_scalar_out_of_plane_strain_coupled)
  {
    _bond_local_force[_qp] =
        2.0 * _b *
        (_mechanical_stretch[_qp] +
         _poissons_ratio * (_scalar_out_of_plane_strain[0] -
                            _alpha * (0.5 * (_temp[0] + _temp[1]) - _temp_ref))) /
        _origin_vec.norm() * _node_vol[0] * _node_vol[1];
    _bond_local_dfdT[_qp] = -2.0 * _b * (1.0 + _poissons_ratio) * 0.5 * _alpha /
                            _origin_vec.norm() * _node_vol[0] * _node_vol[1];

    _bond_nonlocal_force[_qp] =
        2.0 * _a * _d[_qp] * _d[_qp] *
        (_mechanical_stretch[_qp] + _alpha * (0.5 * (_temp[0] + _temp[1]) - _temp_ref) -
         _alpha * (_temp[_qp] - _temp_ref) +
         _poissons_ratio * (_scalar_out_of_plane_strain[0] - _alpha * (_temp[_qp] - _temp_ref))) *
        _node_vol[0] * _node_vol[1];
    _bond_nonlocal_dfdT[_qp] = -2.0 * _a * _d[_qp] * _d[_qp] * (1.0 + _poissons_ratio) * _alpha *
                               _node_vol[0] * _node_vol[1];
  }
  else
  {
    _bond_local_force[_qp] =
        2.0 * _b * _mechanical_stretch[_qp] / _origin_vec.norm() * _node_vol[0] * _node_vol[1];
    _bond_local_dfdT[_qp] =
        -2.0 * _b * 0.5 * _alpha / _origin_vec.norm() * _node_vol[0] * _node_vol[1];

    _bond_nonlocal_force[_qp] =
        2.0 * _a * _d[_qp] * _d[_qp] *
        (_mechanical_stretch[_qp] + _alpha * (0.5 * (_temp[0] + _temp[1]) - _temp_ref) -
         _alpha * (_temp[_qp] - _temp_ref)) *
        _node_vol[0] * _node_vol[1];
    _bond_nonlocal_dfdT[_qp] = -2.0 * _a * _d[_qp] * _d[_qp] * _alpha * _node_vol[0] * _node_vol[1];
  }

  _bond_local_dfdU[_qp] =
      2.0 * _b / _origin_vec.norm() / _origin_vec.norm() * _node_vol[0] * _node_vol[1];
  _bond_nonlocal_dfdU[_qp] =
      2.0 * _a * _d[_qp] * _d[_qp] / _origin_vec.norm() * _node_vol[0] * _node_vol[1];

  _bond_local_dfdE[_qp] =
      2.0 * _b * _poissons_ratio / _origin_vec.norm() * _node_vol[0] * _node_vol[1];
  _bond_nonlocal_dfdE[_qp] =
      2.0 * _a * _d[_qp] * _d[_qp] * _poissons_ratio * _node_vol[0] * _node_vol[1];
}
