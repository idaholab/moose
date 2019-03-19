//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GAPHEATCONDUCTANCETEST_H
#define GAPHEATCONDUCTANCETEST_H

#include "RealMortarConstraint.h"

template <ComputeStage>
class GapHeatConductanceTest;

declareADValidParams(GapHeatConductanceTest);

template <ComputeStage compute_stage>
class GapHeatConductanceTest : public RealMortarConstraint<compute_stage>
{
public:
  GapHeatConductanceTest(const InputParameters & parameters);

protected:
  virtual ADResidual computeQpResidual() override;

  virtual ADResidual computeQpResidualSide(Moose::ConstraintType type) override;

  const Real _gap_conductance_constant;

  usingRealMortarConstraintMembers;
};

#endif /* GAPHEATCONDUCTANCETEST_H */
