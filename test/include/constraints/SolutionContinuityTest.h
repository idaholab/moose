//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SOLUTIONCONTINUITYTEST_H
#define SOLUTIONCONTINUITYTEST_H

#include "RealMortarConstraint.h"

template <ComputeStage>
class SolutionContinuityTest;

declareADValidParams(SolutionContinuityTest);

template <ComputeStage compute_stage>
class SolutionContinuityTest : public RealMortarConstraint<compute_stage>
{
public:
  SolutionContinuityTest(const InputParameters & parameters);

protected:
  virtual ADResidual computeQpResidual() override;

  virtual ADResidual computeQpResidualSide(Moose::ConstraintType type) override;

  usingRealMortarConstraintMembers;
};

#endif /* SOLUTIONCONTINUITYTEST_H */
