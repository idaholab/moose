//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef ADSTRESSDIVERGENCETEST_H
#define ADSTRESSDIVERGENCETEST_H

#include "ADKernel.h"
#include "MaterialProperty.h"

// Forward Declaration
template <ComputeStage>
class ADStressDivergenceTest;

template <>
InputParameters validParams<ADStressDivergenceTest<RESIDUAL>>();
template <>
InputParameters validParams<ADStressDivergenceTest<JACOBIAN>>();

template <ComputeStage compute_stage>
class ADStressDivergenceTest : public ADKernel<compute_stage>
{
public:
  ADStressDivergenceTest(const InputParameters & parameters);

protected:
  virtual typename ResidualReturnType<compute_stage>::type computeQpResidual();

  const typename MaterialPropertyType<compute_stage, RankTwoTensor>::type & _stress;
  const unsigned int _component;

  using ADKernel<compute_stage>::_qp;
  using ADKernel<compute_stage>::_grad_test;
  using ADKernel<compute_stage>::_i;
};

#endif // ADSTRESSDIVERGENCETEST_H
