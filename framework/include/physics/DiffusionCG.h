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
 * Creates all the objects needed to solve a diffusion equation with a continuous Galerkin finite
 * element discretization
 */
class DiffusionCG : public DiffusionPhysicsBase
{
public:
  static InputParameters validParams();

  DiffusionCG(const InputParameters & parameters);

private:
  virtual void addSolverVariables() override;
  virtual void addFEKernels() override;
  virtual void addFEBCs() override;

  /// Whether to use automatic differentiation or not
  const bool _use_ad;
};
