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

// Forward Declaration
template <ComputeStage>
class ADStressDivergenceTest;

declareADValidParams(ADStressDivergenceTest);

template <ComputeStage compute_stage>
class ADStressDivergenceTest : public ADKernel<compute_stage>
{
public:
  ADStressDivergenceTest(const InputParameters & parameters);

protected:
  virtual ADResidual computeQpResidual();

  const ADMaterialProperty(RankTwoTensor) & _stress;
  const unsigned int _component;

  usingKernelMembers;
};

#endif // ADSTRESSDIVERGENCETEST_H
