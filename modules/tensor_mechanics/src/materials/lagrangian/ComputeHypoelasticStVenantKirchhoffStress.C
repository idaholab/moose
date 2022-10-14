//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeHypoelasticStVenantKirchhoffStress.h"

registerMooseObject("TensorMechanicsApp", ComputeHypoelasticStVenantKirchhoffStress);

InputParameters
ComputeHypoelasticStVenantKirchhoffStress::validParams()
{
  InputParameters params = ComputeLagrangianObjectiveStress::validParams();
  params.addClassDescription(
      "Calculate a small strain elastic stress that is equivalent to the hyperelastic St. "
      "Venant-Kirchhoff model if integrated using the Truesdell rate.");

  params.addParam<MaterialPropertyName>(
      "elasticity_tensor", "elasticity_tensor", "The name of the elasticity tensor.");

  return params;
}

ComputeHypoelasticStVenantKirchhoffStress::ComputeHypoelasticStVenantKirchhoffStress(
    const InputParameters & parameters)
  : ComputeLagrangianObjectiveStress(parameters),
    _elasticity_tensor(getMaterialProperty<RankFourTensor>(
        getParam<MaterialPropertyName>(_base_name + "elasticity_tensor"))),
    _def_grad(getMaterialProperty<RankTwoTensor>(_base_name + "deformation_gradient"))
{
}

void
ComputeHypoelasticStVenantKirchhoffStress::computeQpSmallStress()
{
  usingTensorIndices(i, j, k, l);

  // The increment in the spatial velocity gradient
  RankTwoTensor dL = _strain_increment[_qp] + _vorticity_increment[_qp];

  // If small kinematics, it falls back to the grade-zero hypoelasticity
  if (!_large_kinematics)
  {
    _small_stress[_qp] = _small_stress_old[_qp] + _elasticity_tensor[_qp] * dL;
    _small_jacobian[_qp] = _elasticity_tensor[_qp];
    return;
  }

  // Large kinematics:
  // First push forward the elasticity tensor
  const RankTwoTensor F = _def_grad[_qp];
  const Real J = F.det();
  const RankFourTensor FF = F.times<i, k, j, l>(F);
  const RankFourTensor FtFt = F.times<k, i, l, j>(F);
  const RankFourTensor C0 = _elasticity_tensor[_qp];
  const RankFourTensor C = FF * C0 * FtFt / J;

  // Update the small stress
  const RankTwoTensor dS = C * dL;
  _small_stress[_qp] = _small_stress_old[_qp] + dS;

  // Compute the small Jacobian
  if (_fe_problem.currentlyComputingJacobian())
  {
    const RankFourTensor dFddL = _inv_df[_qp].inverse().times<i, k, l, j>(F);
    _small_jacobian[_qp] =
        C +
        (dFddL.singleProductJ((C0.tripleProductJkl(F, F, F) * dL).transpose()) +
         dFddL.singleProductJ(C0.tripleProductIkl(F, F, F) * dL).transposeIj() +
         FF * C0.singleProductL(F * dL).transposeKl() * dFddL +
         FF * C0.singleProductK(dL.transpose() * F) * dFddL) /
            J -
        dS.outerProduct(_inv_def_grad[_qp].transpose().initialContraction(dFddL));
  }
}
