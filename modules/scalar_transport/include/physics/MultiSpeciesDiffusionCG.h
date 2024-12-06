//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiSpeciesDiffusionPhysicsBase.h"

/**
 * Creates all the objects needed to solve diffusion equations for multiple species with a
 * continuous Galerkin finite element discretization
 */
class MultiSpeciesDiffusionCG : public MultiSpeciesDiffusionPhysicsBase
{
public:
  static InputParameters validParams();

  MultiSpeciesDiffusionCG(const InputParameters & parameters);

protected:
  virtual void addSolverVariables() override;
  virtual void addFEKernels() override;
  virtual void addFEBCs() override;
};
