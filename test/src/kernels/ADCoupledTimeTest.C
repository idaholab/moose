//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "ADCoupledTimeTest.h"

registerADMooseObject("MooseTestApp", ADCoupledTimeTest);

defineADValidParams(ADCoupledTimeTest,
                    ADTimeKernel,
                    params.addCoupledVar("v", 2.0, "The coupled variable."););

template <ComputeStage compute_stage>
ADCoupledTimeTest<compute_stage>::ADCoupledTimeTest(const InputParameters & parameters)
  : ADTimeKernel<compute_stage>(parameters), _v_dot(adCoupledDot("v"))
{
}

template <ComputeStage compute_stage>
ADResidual
ADCoupledTimeTest<compute_stage>::precomputeQpResidual()
{
  return _v_dot[_qp];
}
