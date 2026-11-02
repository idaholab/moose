//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PhysicsBase.h"

/**
 * Test Physics that requests a finite element solver variable and an auxiliary variable shared
 * with other instances of this Physics, to test PhysicsVariableCoordinator for what the diffusion
 * Physics cannot: a different scaling factor and a shared auxiliary variable.
 */
class SharedVariablePhysics : public PhysicsBase
{
public:
  static InputParameters validParams();

  SharedVariablePhysics(const InputParameters & parameters);

private:
  virtual void addSolverVariables() override;
  virtual void addAuxiliaryVariables() override;
  virtual void addFEKernels() override;
};
