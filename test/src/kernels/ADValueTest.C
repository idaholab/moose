//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADValueTest.h"

registerADMooseObject("MooseTestApp", ADValueTest);

defineADValidParams(ADValueTest, ADKernel, );

template <ComputeStage compute_stage>
ADValueTest<compute_stage>::ADValueTest(const InputParameters & parameters)
  : ADKernel<compute_stage>(parameters)
{
}

template <ComputeStage compute_stage>
ADReal
ADValueTest<compute_stage>::computeQpResidual()
{
  return -_u[_qp] * _test[_i][_qp];
}
