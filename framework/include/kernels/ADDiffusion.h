//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDIFFUSION_H
#define ADDIFFUSION_H

#include "ADKernel.h"

template <ComputeStage compute_stage>
class ADDiffusion;

template <>
InputParameters validParams<ADDiffusion<RESIDUAL>>();
template <>
InputParameters validParams<ADDiffusion<JACOBIAN>>();

template <ComputeStage compute_stage>
class ADDiffusion : public ADKernel<compute_stage>
{
public:
  ADDiffusion(const InputParameters & parameters);

protected:
  virtual typename ResidualReturnType<compute_stage>::type computeQpResidual() override;

  using ADKernel<compute_stage>::_grad_u;
  using ADKernel<compute_stage>::_qp;
  using ADKernel<compute_stage>::_grad_test;
  using ADKernel<compute_stage>::_i;
};

#endif /* ADDIFFUSION_H */
