//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeNeoHookeanStress.h"

registerMooseObject("TensorMechanicsApp", ComputeNeoHookeanStress);

InputParameters
ComputeNeoHookeanStress::validParams()
{
  InputParameters params = ComputeLagrangianStressPK2::validParams();

  params.addParam<MaterialPropertyName>("lambda",
                                        "lambda",
                                        "Parameter conjugate to Lame parameter"
                                        " for small deformations");
  params.addParam<MaterialPropertyName>("mu",
                                        "mu",
                                        "Parameter conjugate to Lame parameter"
                                        " for small deformations");

  return params;
}

ComputeNeoHookeanStress::ComputeNeoHookeanStress(const InputParameters & parameters)
  : ComputeLagrangianStressPK2(parameters),
    _lambda(getMaterialProperty<Real>(getParam<MaterialPropertyName>("lambda"))),
    _mu(getMaterialProperty<Real>(getParam<MaterialPropertyName>("mu")))

{
}

void
ComputeNeoHookeanStress::computeQpPK2Stress()
{
  // Hyperelasticity is weird, we need to branch on the type of update if we
  // want a truly linear model
  //
  // This is because we need to drop quadratic terms for the linear update
  usingTensorIndices(i_, j_, k_, l_);

  // Large deformation = nonlinear strain
  if (_large_kinematics)
  {
    RankTwoTensor Cinv = (2 * _E[_qp] + RankTwoTensor::Identity()).inverse();
    _S[_qp] = (_lambda[_qp] * log(_F[_qp].det()) - _mu[_qp]) * Cinv +
              _mu[_qp] * RankTwoTensor::Identity();
    _C[_qp] =
        -2 * (_lambda[_qp] * log(_F[_qp].det()) - _mu[_qp]) * Cinv.times<i_, k_, l_, j_>(Cinv) +
        _lambda[_qp] * Cinv.times<i_, j_, k_, l_>(Cinv);
  }
  // Small deformations = linear strain
  else
  {
    const auto I = RankTwoTensor::Identity();
    RankTwoTensor strain = 0.5 * (_F[_qp] + _F[_qp].transpose()) - I;
    _C[_qp] = _lambda[_qp] * I.times<i_, j_, k_, l_>(I) +
              2.0 * _mu[_qp] * RankFourTensor(RankFourTensor::initIdentitySymmetricFour);
    _S[_qp] = _C[_qp] * strain;
  }
}
