//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADVECTORTIMEDERIVATIVE_H
#define ADVECTORTIMEDERIVATIVE_H

#include "ADTimeKernel.h"

// Forward Declaration
template <ComputeStage>
class ADVectorTimeDerivative;

declareADValidParams(ADVectorTimeDerivative);

template <ComputeStage compute_stage>
class ADVectorTimeDerivative : public ADVectorTimeKernel<compute_stage>
{
public:
  ADVectorTimeDerivative(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  usingVectorTimeKernelMembers;
};

#endif // ADVECTORTIMEDERIVATIVE_H
