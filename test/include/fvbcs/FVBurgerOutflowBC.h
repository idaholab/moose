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
 * Constant velocity scalar advection boundary conditions
 */
template <ComputeStage compute_stage>
class FVBurgerOutflowBC : public FVFluxBC<compute_stage>
{
public:
  FVBurgerOutflowBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

  usingFVFluxBCMembers;
};
