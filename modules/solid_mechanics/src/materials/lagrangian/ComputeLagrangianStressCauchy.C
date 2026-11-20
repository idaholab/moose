//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianStressCauchy.h"

InputParameters
ComputeLagrangianStressCauchy::validParams()
{
  InputParameters params = ComputeLagrangianStressBase::validParams();

  params.addClassDescription("Stress update based on the Cauchy stress");

  return params;
}

ComputeLagrangianStressCauchy::ComputeLagrangianStressCauchy(const InputParameters & parameters)
  : ComputeLagrangianStressBase(parameters),
    _inv_df(getMaterialPropertyByName<RankTwoTensor>(_base_name +
                                                     "inverse_incremental_deformation_gradient")),
    _inv_def_grad(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "inverse_deformation_gradient")),
    _stabilized_deformation_gradient(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "deformation_gradient"))
{
}

void
ComputeLagrangianStressCauchy::computeQpStressUpdate()
{
  computeQpCauchyStress();
  computeQpPK1Stress(); // This could be "switched"
}

void
ComputeLagrangianStressCauchy::computeQpPK1Stress()
{
  // Wrap sigma -> PK1 using the *unstabilized* F (= F_actual at alpha = 1). F-bar enters
  // only through sigma via the strain calc's F-bar'd `_f_inv`, NOT through this kinematic
  // wrap -- this matches OLD's `StressDivergenceTensors`-on-displaced-mesh residual,
  // which integrates `gradTest_spatial * sigma dV` on the actual deformed mesh (so the
  // implicit "wrap F" is F_actual, NOT the F-bar'd F).
  pullBackCauchyStress();

  // The R4 algebra for `_pk1_jacobian` and `_pk1_jacobian_bypass_fbar` is Jacobian-only;
  // the wrap to `_pk1_stress` is not. Gate accordingly.
  if (_fe_problem.currentlyComputingJacobian() ||
      _fe_problem.currentlyComputingResidualAndJacobian())
    pullBackCauchyJacobian(_cauchy_jacobian[_qp] * _d_deformation_gradient_increment_d_F[_qp]);
}
