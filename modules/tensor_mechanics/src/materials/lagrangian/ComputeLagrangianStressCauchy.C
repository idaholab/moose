//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
    _inv_df(getMaterialPropertyByName<RankTwoTensor>(_base_name + "inv_inc_def_grad")),
    _inv_def_grad(getMaterialPropertyByName<RankTwoTensor>(_base_name + "inv_def_grad")),
    _detJ(getMaterialPropertyByName<Real>(_base_name + "detJ"))
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
  // Actually do the (annoying) wrapping
  if (_large_kinematics)
  {
    _pk1_stress[_qp] = _detJ[_qp] * _cauchy_stress[_qp] * _inv_def_grad[_qp].transpose();

    usingTensorIndices(i, j, k, l);
    _pk1_jacobian[_qp] = _pk1_stress[_qp].outerProduct(_inv_def_grad[_qp].transpose());
    _pk1_jacobian[_qp] -= _pk1_stress[_qp].mixedProduct<i, l, j, k>(_inv_def_grad[_qp]);
    _pk1_jacobian[_qp] +=
        _detJ[_qp] * _cauchy_jacobian[_qp].tripleProductJkl(
                         _inv_def_grad[_qp], _inv_df[_qp].transpose(), _inv_def_grad[_qp]);
  }
  else
  {
    _pk1_stress[_qp] = _cauchy_stress[_qp];
    _pk1_jacobian[_qp] = _cauchy_jacobian[_qp];
  }
}
