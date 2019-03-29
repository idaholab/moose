//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef ADCOUPLEDTIMETEST_H
#define ADCOUPLEDTIMETEST_H

#include "ADTimeKernel.h"

template <ComputeStage>
class ADCoupledTimeTest;

declareADValidParams(ADCoupledTimeTest);

template <ComputeStage compute_stage>
class ADCoupledTimeTest : public ADTimeKernel<compute_stage>
{
public:
  ADCoupledTimeTest(const InputParameters & parameters);

protected:
  virtual ADResidual precomputeQpResidual() override;

  const ADVariableValue & _v_dot;

  usingKernelMembers;
};

#endif /* ADCOUPLEDTIMETEST_H */
