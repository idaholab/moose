//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
    _F(getMaterialPropertyByName<RankTwoTensor>(_base_name + "deformation_gradient"))
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

    auto f = _inv_df[_qp].inverse();
    usingTensorIndices(i_, j_, k_, l_);
    _cauchy_jacobian[_qp] = _cauchy_stress[_qp].times<i_, l_, j_, k_>(f) -
                            _cauchy_stress[_qp].times<i_, j_, k_, l_>(f.transpose());
    _cauchy_jacobian[_qp] +=
        _pk1_jacobian[_qp].tripleProductJkl(_F[_qp], f.transpose(), _F[_qp]) / _F[_qp].det();
  }
  // Small deformations these are the same
  else
  {
    _cauchy_stress[_qp] = _pk1_stress[_qp];
    _cauchy_jacobian[_qp] = _pk1_jacobian[_qp];
  }
}
