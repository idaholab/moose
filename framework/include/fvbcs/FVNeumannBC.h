//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"

/**
 * Neumann boundary (== fixed inflow) condition for finite volume scheme
 */
template <ComputeStage compute_stage>
class FVNeumannBC : public FVFluxBC<compute_stage>
{
public:
  FVNeumannBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;
  // virtual ADReal computeQpJacobian() override { return 0; }

  const Real _value;

  usingFVFluxBCMembers;
};
