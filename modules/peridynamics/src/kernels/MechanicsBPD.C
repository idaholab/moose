//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MechanicsBPD.h"

registerMooseObject("PeridynamicsApp", MechanicsBPD);

template <>
InputParameters
validParams<MechanicsBPD>()
{
  InputParameters params = validParams<MechanicsBasePD>();
  params.addClassDescription("Class for calculating residual and Jacobian for Bond-based "
                             "PeriDynamic mechanics formulation");

  params.addRequiredParam<unsigned int>(
      "component",
      "An integer corresponding to the variable this kernel acts on (0 for x, 1 for y, 2 for z)");

  return params;
}

MechanicsBPD::MechanicsBPD(const InputParameters & parameters)
  : MechanicsBasePD(parameters),
    _bond_force_ij(getMaterialProperty<Real>("bond_force_ij")),
    _bond_dfdU_ij(getMaterialProperty<Real>("bond_dfdU_ij")),
    _bond_dfdT_ij(getMaterialProperty<Real>("bond_dfdT_ij")),
    _component(getParam<unsigned int>("component"))
{
}

void
MechanicsBPD::computeLocalResidual()
{
  _local_re(0) = -_bond_force_ij[0] * _cur_ori_ij(_component) * _bond_status_ij;
  _local_re(1) = -_local_re(0);
}

void
MechanicsBPD::computeLocalJacobian()
{
  Real diag =
      _cur_ori_ij(_component) * _cur_ori_ij(_component) * _bond_dfdU_ij[0] +
      _bond_force_ij[0] * (1.0 - _cur_ori_ij(_component) * _cur_ori_ij(_component)) / _cur_len_ij;

  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < _phi.size(); _j++)
      _local_ke(_i, _j) += (_i == _j ? 1 : -1) * diag * _bond_status_ij;
}

void
MechanicsBPD::computeLocalOffDiagJacobian(unsigned int coupled_component)
{
  if (coupled_component == 3)
  {
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        _local_ke(_i, _j) +=
            (_i == 1 ? 1 : -1) * _cur_ori_ij(_component) * _bond_dfdT_ij[_j] * _bond_status_ij;
  }
  else
  {
    Real val =
        _cur_ori_ij(_component) * _cur_ori_ij(coupled_component) * _bond_dfdU_ij[0] -
        _bond_force_ij[0] * _cur_ori_ij(_component) * _cur_ori_ij(coupled_component) / _cur_len_ij;
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        _local_ke(_i, _j) += (_i == _j ? 1 : -1) * val * _bond_status_ij;
  }
}
