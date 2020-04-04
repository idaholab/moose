//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeSmallStrainMaterialBaseBPD.h"

InputParameters
ComputeSmallStrainMaterialBaseBPD::validParams()
{
  InputParameters params = ParametricMaterialBasePD::validParams();
  params.addClassDescription("Base class for bond-based peridynamic mechanics models");

  return params;
}

ComputeSmallStrainMaterialBaseBPD::ComputeSmallStrainMaterialBaseBPD(
    const InputParameters & parameters)
  : ParametricMaterialBasePD(parameters)
{
}

void
ComputeSmallStrainMaterialBaseBPD::computeBondForce()
{
  if (_scalar_out_of_plane_strain_coupled)
  {
    _bond_local_force[_qp] =
        _Cij *
        (_mechanical_stretch[_qp] +
         _poissons_ratio * (_scalar_out_of_plane_strain[0] -
                            _alpha * (0.5 * (_temp[0] + _temp[1]) - _temp_ref))) *
        _node_vol[0] * _node_vol[1];
    _bond_local_dfdT[_qp] =
        -_Cij * (1.0 + _poissons_ratio) * 0.5 * _alpha * _node_vol[0] * _node_vol[1];
  }
  else
  {
    _bond_local_force[_qp] = _Cij * _mechanical_stretch[_qp] * _node_vol[0] * _node_vol[1];
    _bond_local_dfdT[_qp] = -_Cij * 0.5 * _alpha * _node_vol[0] * _node_vol[1];
  }

  _bond_local_dfdU[_qp] = _Cij / _origin_vec.norm() * _node_vol[0] * _node_vol[1];
  _bond_local_dfdE[_qp] = _Cij * _poissons_ratio * _node_vol[0] * _node_vol[1];
}
