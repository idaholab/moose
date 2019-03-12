//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADTIMEDERIVATIVE_H
#define ADTIMEDERIVATIVE_H

#include "ADTimeKernel.h"

// Forward Declaration
template <ComputeStage>
class ADTimeDerivative;

declareADValidParams(ADTimeDerivative);

template <ComputeStage compute_stage>
class ADTimeDerivative : public ADTimeKernel<compute_stage>
{
public:
  ADTimeDerivative(const InputParameters & parameters);

protected:
  virtual ADResidual precomputeQpResidual() override;

  usingTimeKernelMembers;
};

#endif // ADTIMEDERIVATIVE_H
