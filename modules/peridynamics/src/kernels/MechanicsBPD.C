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

InputParameters
MechanicsBPD::validParams()
{
  InputParameters params = MechanicsBasePD::validParams();
  params.addClassDescription("Class for calculating the residual and Jacobian for the bond-based "
                             "peridynamic mechanics formulation");

  params.addRequiredParam<unsigned int>(
      "component",
      "An integer corresponding to the variable this kernel acts on (0 for x, 1 for y, 2 for z)");

  return params;
}

MechanicsBPD::MechanicsBPD(const InputParameters & parameters)
  : MechanicsBasePD(parameters),
    _bond_local_force(getMaterialProperty<Real>("bond_local_force")),
    _bond_local_dfdU(getMaterialProperty<Real>("bond_dfdU")),
    _bond_local_dfdT(getMaterialProperty<Real>("bond_dfdT")),
    _component(getParam<unsigned int>("component"))
{
}

void
MechanicsBPD::computeLocalResidual()
{
  _local_re(0) = -_bond_local_force[0] * _current_unit_vec(_component) * _bond_status;
  _local_re(1) = -_local_re(0);
}

void
MechanicsBPD::computeLocalJacobian()
{
  Real diag = _current_unit_vec(_component) * _current_unit_vec(_component) * _bond_local_dfdU[0] +
              _bond_local_force[0] *
                  (1.0 - _current_unit_vec(_component) * _current_unit_vec(_component)) /
                  _current_vec.norm();

  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < _phi.size(); _j++)
      _local_ke(_i, _j) += (_i == _j ? 1 : -1) * diag * _bond_status;
}

void
MechanicsBPD::computeLocalOffDiagJacobian(unsigned int coupled_component)
{
  if (coupled_component == 3)
  {
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        _local_ke(_i, _j) += (_i == 1 ? 1 : -1) * _current_unit_vec(_component) *
                             _bond_local_dfdT[_j] * _bond_status;
  }
  else
  {
    Real val =
        _current_unit_vec(_component) * _current_unit_vec(coupled_component) * _bond_local_dfdU[0] -
        _bond_local_force[0] * _current_unit_vec(_component) *
            _current_unit_vec(coupled_component) / _current_vec.norm();
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        _local_ke(_i, _j) += (_i == _j ? 1 : -1) * val * _bond_status;
  }
}
