//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechADMatTest.h"

registerADMooseObject("TensorMechanicsApp", TensorMechADMatTest);

template <>
InputParameters
validParams<TensorMechADMatTest<RESIDUAL>>()
{
  InputParameters params = validParams<ADMaterial<RESIDUAL>>();
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addRequiredParam<Real>("poissons_ratio", "Poisson's ratio for the material.");
  params.addRequiredParam<Real>("youngs_modulus", "Young's modulus of the material.");
  return params;
}
template <>
InputParameters
validParams<TensorMechADMatTest<JACOBIAN>>()
{
  return validParams<TensorMechADMatTest<RESIDUAL>>();
}

template <ComputeStage compute_stage>
TensorMechADMatTest<compute_stage>::TensorMechADMatTest(const InputParameters & parameters)
  : ADMaterial<compute_stage>(parameters),
    _elasticity_tensor(this->template declareProperty<RankFourTensor>("elasticity_tensor")),
    _total_strain(this->template declareADProperty<RankTwoTensor>("total_strain")),
    _mechanical_strain(this->template declareADProperty<RankTwoTensor>("mechanical_strain")),
    _stress(this->template declareADProperty<RankTwoTensor>("stress")),
    _ndisp(coupledComponents("displacements")),
    _grad_disp(3)
{
  if (this->template getParam<bool>("use_displaced_mesh"))
    mooseError("The linear stress/strain calculator needs to run on the undisplaced mesh.");

  auto poissons_ratio = this->template getParam<Real>("poissons_ratio");
  auto youngs_modulus = this->template getParam<Real>("youngs_modulus");
  _Cijkl.fillSymmetricIsotropicEandNu(youngs_modulus, poissons_ratio);

  for (decltype(_ndisp) i = 0; i < _ndisp; ++i)
    _grad_disp[i] = &this->template adCoupledGradientTemplate<compute_stage>("displacements", i);
  for (decltype(_ndisp) i = _ndisp; i < 3; ++i)
    _grad_disp[i] = &this->template adGradZero<compute_stage>();
}

template <ComputeStage compute_stage>
void
TensorMechADMatTest<compute_stage>::computeQpProperties()
{
  _elasticity_tensor[_qp] = _Cijkl;

  typename RankTwoTensorType<compute_stage>::type grad_tensor(
      (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]);

  _total_strain[_qp] = (grad_tensor + grad_tensor.transpose()) / 2.;
  _mechanical_strain[_qp] = _total_strain[_qp];

  _stress[_qp] = _elasticity_tensor[_qp] * _mechanical_strain[_qp];
}
