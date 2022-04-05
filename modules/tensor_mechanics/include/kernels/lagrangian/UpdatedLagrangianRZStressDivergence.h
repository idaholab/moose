//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "UpdatedLagrangianStressDivergence.h"

/// Enforce equilibrium with an updated Lagrangian formulation in RZ coordinates
class UpdatedLagrangianRZStressDivergence : public UpdatedLagrangianStressDivergence
{
public:
  static InputParameters validParams();
  UpdatedLagrangianRZStressDivergence(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual RankTwoTensor testGrad(unsigned int i) override;
  virtual RankTwoTensor trialGrad(unsigned int k, bool stabilize) override;
};
