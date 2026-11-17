//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  // Green-Lagrange strain uses the F-bar-stabilized F (`_F` from the strain calc) so
  // the constitutive law receives the stabilized strain -- F-bar's purpose is precisely
  // to feed a volumetrically-corrected strain into the constitutive update.
  _E[_qp] = 0.5 * (_F[_qp].transpose() * _F[_qp] - RankTwoTensor::Identity());

  // PK2 update (constitutive). This populates `_S` (always needed) and `_C` (= dPK2/dE,
  // Jacobian-only). PK2-subclass implementers that want to skip `_C` on residual sweeps
  // can check `_fe_problem.currentlyComputingJacobian()` themselves; the wrap below
  // doesn't read `_C` in the residual path.
  computeQpPK2Stress();

  const bool need_jacobian = _fe_problem.currentlyComputingJacobian() ||
                             _fe_problem.currentlyComputingResidualAndJacobian();

  // The constitutive PK1 stress Pbar = Fbar Sbar and dPbar/dFbar, with Fbar = `_F`. The PK1 base
  // pushes them forward with Fbar to the Cauchy stress.
  if (_large_kinematics)
  {
    const RankTwoTensor & F_bar = _F[_qp];
    const RankTwoTensor & S_bar = _S[_qp];
    _pk1_stress[_qp] = F_bar * S_bar;
    if (!need_jacobian)
      return;

    usingTensorIndices(i_, j_, k_, l_);
    // dE/dFbar
    const RankFourTensor dE_dF_bar =
        0.5 * (RankTwoTensor::Identity().times<i_, l_, j_, k_>(F_bar.transpose()) +
               F_bar.transpose().times<i_, k_, j_, l_>(RankTwoTensor::Identity()));

    // dPbar/dFbar = dFbar/dFbar * Sbar + Fbar * dSbar/dE * dE/dFbar
    //   dFbar/dFbar * Sbar gives I x Sbar^T (per the existing template, with Sbar^T because
    //   Pbar = Fbar * Sbar and we differentiate Fbar).
    _pk1_jacobian[_qp] = RankTwoTensor::Identity().times<i_, k_, j_, l_>(S_bar.transpose()) +
                         (_C[_qp] * dE_dF_bar).singleProductI(F_bar);
  }
  // Small deformations: PK1 = PK2 = sigma
  else
  {
    _pk1_stress[_qp] = _S[_qp];
    if (!need_jacobian)
      return;
    _pk1_jacobian[_qp] = _C[_qp];
  }
}
