//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADStressDivergenceTest.h"

registerADMooseObject("TensorMechanicsApp", ADStressDivergenceTest);

template <>
InputParameters
validParams<ADStressDivergenceTest<RESIDUAL>>()
{
  InputParameters params = validParams<ADKernel<RESIDUAL>>();
  params.addRequiredParam<unsigned int>("component", "displacement component");
  return params;
}
template <>
InputParameters
validParams<ADStressDivergenceTest<JACOBIAN>>()
{
  return validParams<ADStressDivergenceTest<RESIDUAL>>();
}

template <ComputeStage compute_stage>
ADStressDivergenceTest<compute_stage>::ADStressDivergenceTest(const InputParameters & parameters)
  : ADKernel<compute_stage>(parameters),
    _stress(this->template getADMaterialProperty<RankTwoTensor>("stress")),
    _component(this->template getParam<unsigned int>("component"))
{
}

template <ComputeStage compute_stage>
typename ResidualReturnType<compute_stage>::type
ADStressDivergenceTest<compute_stage>::computeQpResidual()
{
  return _stress[_qp].row(_component) * _grad_test[_i][_qp];
}
