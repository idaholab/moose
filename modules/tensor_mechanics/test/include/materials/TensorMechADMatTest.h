//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TENSORMECHADMATTEST_H
#define TENSORMECHADMATTEST_H

#include "ADMaterial.h"
#include "RankFourTensor.h"

template <ComputeStage>
class TensorMechADMatTest;
template <typename>
class RankTwoTensorTempl;
typedef RankTwoTensorTempl<Real> RankTwoTensor;

template <>
InputParameters validParams<TensorMechADMatTest<RESIDUAL>>();
template <>
InputParameters validParams<TensorMechADMatTest<JACOBIAN>>();

/**
 * TensorMechADMatTest is the base class for stress tensors
 */
template <ComputeStage compute_stage>
class TensorMechADMatTest : public ADMaterial<compute_stage>
{
public:
  TensorMechADMatTest(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  MaterialProperty<RankFourTensor> & _elasticity_tensor;
  typename MaterialPropertyType<compute_stage, RankTwoTensor>::type & _total_strain;
  typename MaterialPropertyType<compute_stage, RankTwoTensor>::type & _mechanical_strain;
  typename MaterialPropertyType<compute_stage, RankTwoTensor>::type & _stress;

  unsigned int _ndisp;
  std::vector<const typename VariableGradientType<compute_stage>::type *> _grad_disp;

  RankFourTensor _Cijkl;

  using ADMaterial<compute_stage>::_ad_grad_zero;
  using ADMaterial<compute_stage>::_qp;
  using ADMaterial<compute_stage>::coupledComponents;
};

#endif // TENSORMECHADMATTEST_H
