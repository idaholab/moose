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
 * Creates all the objects needed to solve the heat conduction equations with CG
 */
class HeatConductionCG : public HeatConductionPhysicsBase
{
public:
  static InputParameters validParams();

  HeatConductionCG(const InputParameters & parameters);

private:
  void addSolverVariables() override;
  void addFEKernels() override;
  void addFEBCs() override;
};
