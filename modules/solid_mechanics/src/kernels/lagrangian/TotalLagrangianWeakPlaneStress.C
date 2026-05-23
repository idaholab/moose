//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TotalLagrangianWeakPlaneStress.h"

registerMooseObject("SolidMechanicsApp", TotalLagrangianWeakPlaneStress);

InputParameters
TotalLagrangianWeakPlaneStress::validParams()
{
  InputParameters params = TotalLagrangianStressDivergence::validParams();
  params.addClassDescription("Plane stress kernel to provide out-of-plane strain contribution.");
  params.set<unsigned int>("component") = 0;
  params.suppressParameter<unsigned int>("component");
  params.suppressParameter<std::vector<VariableName>>("temperature");
  params.suppressParameter<std::vector<MaterialPropertyName>>("eigenstrain_names");
  params.suppressParameter<std::vector<VariableName>>("out_of_plane_strain");
  return params;
}

TotalLagrangianWeakPlaneStress::TotalLagrangianWeakPlaneStress(const InputParameters & parameters)
  : TotalLagrangianStressDivergence(parameters)
{
}

Real
TotalLagrangianWeakPlaneStress::computeQpResidual()
{
  return _test[_i][_qp] * _pk1[_qp](2, 2);
}

Real
TotalLagrangianWeakPlaneStress::computeQpJacobian()
{
  // Diagonal Jacobian: d(R_zz)/d(strain_zz_j) = test · d(PK1_{2,2})/d(strain_zz_j).
  // strain_zz feeds `_F[(2,2)]` AFTER F-bar runs, so strain_zz perturbations bypass
  // F-bar's chain. Use `_dpk1_bypass_fbar` (pk1_jacobian with the F-bar
  // `_d_F_stab_d_F_ust` factor REPLACED by identity in the σ chain) for consistency.
  return _test[_i][_qp] * _dpk1_bypass_fbar[_qp](2, 2, 2, 2) * _phi[_j][_qp];
}

Real
TotalLagrangianWeakPlaneStress::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (auto beta : make_range(_ndisp))
    if (jvar == _disp_nums[beta])
    {
      // Local PK1_{2,2} Jacobian via _dpk1 (includes local F-bar through the σ chain).
      // Displacement perturbations DO go through F-bar's chain, so use the regular
      // `_dpk1` (= pk1_jacobian WITH the F-bar local correction).
      Real J = _test[_i][_qp] * _dpk1[_qp].contractionIj(2, 2, gradTrial(beta));

      // Non-local F-bar contribution to PK1_{2,2}; helper handles the σ-via-dL chain and
      // the wrap branch. Guarded on `_stabilize_strain` because `_avg_grad_trial` is only
      // populated when F-bar is on.
      if (_stabilize_strain)
      {
        const RankTwoTensor delta_F_avg = _d_F_d_grad_u[_qp] * _avg_grad_trial[beta][_j];
        J += _test[_i][_qp] * deltaPK1NonLocalFBar(delta_F_avg)(2, 2);
      }
      return J;
    }

  return 0;
}
