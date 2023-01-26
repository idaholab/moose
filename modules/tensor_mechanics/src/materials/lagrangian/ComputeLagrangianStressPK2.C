//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianStressPK2.h"

InputParameters
ComputeLagrangianStressPK2::validParams()
{
  InputParameters params = ComputeLagrangianStressPK1::validParams();
  return params;
}

ComputeLagrangianStressPK2::ComputeLagrangianStressPK2(const InputParameters & parameters)
  : ComputeLagrangianStressPK1(parameters),
    _E(declareProperty<RankTwoTensor>(_base_name + "green_lagrange_strain")),
    _S(declareProperty<RankTwoTensor>(_base_name + "pk2_stress")),
    _C(declareProperty<RankFourTensor>(_base_name + "pk2_jacobian"))
{
}

void
ComputeLagrangianStressPK2::computeQpPK1Stress()
{
  // Calculate the green-lagrange strain for the benefit of the subclasses
  _E[_qp] = 0.5 * (_F[_qp].transpose() * _F[_qp] - RankTwoTensor::Identity());

  // PK2 update
  computeQpPK2Stress();

  // Complicated wrapping, see documentation
  if (_large_kinematics)
  {
    _pk1_stress[_qp] = _F[_qp] * _S[_qp];
    usingTensorIndices(i_, j_, k_, l_);
    RankFourTensor dE =
        0.5 * (RankTwoTensor::Identity().times<i_, l_, j_, k_>(_F[_qp].transpose()) +
               _F[_qp].transpose().times<i_, k_, j_, l_>(RankTwoTensor::Identity()));

    _pk1_jacobian[_qp] = RankTwoTensor::Identity().times<i_, k_, j_, l_>(_S[_qp].transpose()) +
                         (_C[_qp] * dE).singleProductI(_F[_qp]);
  }
  // Small deformations all are equivalent
  else
  {
    _pk1_stress[_qp] = _S[_qp];
    _pk1_jacobian[_qp] = _C[_qp];
  }
}
