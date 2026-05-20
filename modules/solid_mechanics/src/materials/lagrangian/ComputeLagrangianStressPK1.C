//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianStressPK1.h"

InputParameters
ComputeLagrangianStressPK1::validParams()
{
  InputParameters params = ComputeLagrangianStressBase::validParams();

  params.addClassDescription("Stress update based on the first Piola-Kirchhoff stress");

  return params;
}

ComputeLagrangianStressPK1::ComputeLagrangianStressPK1(const InputParameters & parameters)
  : ComputeLagrangianStressBase(parameters),
    _inv_df(getMaterialPropertyByName<RankTwoTensor>(_base_name +
                                                     "inverse_incremental_deformation_gradient")),
    _inv_def_grad(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "inverse_deformation_gradient")),
    _F(getMaterialPropertyByName<RankTwoTensor>(_base_name + "deformation_gradient")),
    _d_spatial_velocity_increment_d_F(getMaterialPropertyByName<RankFourTensor>(
        _base_name + "d_spatial_velocity_increment_d_deformation_gradient"))
{
}

void
ComputeLagrangianStressPK1::computeQpStressUpdate()
{
  computeQpPK1Stress();
  computeQpCauchyStress(); // This could be "switched"
}

void
ComputeLagrangianStressPK1::computeQpCauchyStress()
{
  // Actually do the (annoying) wrapping
  if (_large_kinematics)
  {
    _cauchy_stress[_qp] = _pk1_stress[_qp] * _F[_qp].transpose() / _F[_qp].det();

    // Build dsigma/dF from the three contributions arising from sigma = (1/J) P F^T,
    // then chain through dF/d(dL) (the inverse of the stored d(dL)/dF) to get
    // dsigma/d(dL). This replaces the previous form that baked in the linear
    // approximation factors f and f^T inline.
    usingTensorIndices(i_, j_, m_, n_);
    const Real J = _F[_qp].det();
    const auto I = RankTwoTensor::Identity();

    RankFourTensor dsigma_dF = _pk1_jacobian[_qp].singleProductJ(_F[_qp]) / J;
    dsigma_dF += _pk1_stress[_qp].times<i_, n_, j_, m_>(I) / J;
    dsigma_dF -= _cauchy_stress[_qp].times<i_, j_, n_, m_>(_inv_def_grad[_qp]);

    _cauchy_jacobian[_qp] = dsigma_dF * _d_spatial_velocity_increment_d_F[_qp].inverse();
  }
  // Small deformations these are the same
  else
  {
    _cauchy_stress[_qp] = _pk1_stress[_qp];
    _cauchy_jacobian[_qp] = _pk1_jacobian[_qp];
  }
}
