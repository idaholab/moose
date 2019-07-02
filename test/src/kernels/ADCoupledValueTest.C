//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoupledValueTest.h"

registerADMooseObject("MooseTestApp", ADCoupledValueTest);

defineADValidParams(ADCoupledValueTest,
                    ADKernel,
                    params.addCoupledVar("v", 2.0, "The coupled variable."););

template <ComputeStage compute_stage>
ADCoupledValueTest<compute_stage>::ADCoupledValueTest(const InputParameters & parameters)
  : ADKernel<compute_stage>(parameters), _v(adCoupledValue("v"))
{
}

template <ComputeStage compute_stage>
ADReal
ADCoupledValueTest<compute_stage>::computeQpResidual()
{
  return _test[_i][_qp] * -_v[_qp];
}
