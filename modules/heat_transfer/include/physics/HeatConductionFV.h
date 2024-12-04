//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatConductionPhysicsBase.h"

/**
 * Creates all the objects needed to solve the heat conduction equations with a finite volume
 * discretization
 */
class HeatConductionFV : public HeatConductionPhysicsBase
{
public:
  static InputParameters validParams();

  HeatConductionFV(const InputParameters & parameters);

  virtual void addFVBCs() override;

private:
  virtual void initializePhysicsAdditional() override;
  virtual void addSolverVariables() override;
  virtual void addFVKernels() override;
};
