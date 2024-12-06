//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DiffusionPhysicsBase.h"

/**
 * Creates all the objects needed to solve a diffusion equation with a cell-centered finite
 * volume discretization
 */
class DiffusionFV : public DiffusionPhysicsBase
{
public:
  static InputParameters validParams();

  DiffusionFV(const InputParameters & parameters);

private:
  virtual void addSolverVariables() override;
  virtual void addFVKernels() override;
  virtual void addFVBCs() override;
  virtual void initializePhysicsAdditional() override;
  virtual InputParameters getAdditionalRMParams() const override;
};
