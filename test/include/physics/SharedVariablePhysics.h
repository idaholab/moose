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
 * Test Physics that requests a solver variable and an auxiliary variable shared with other
 * instances of this Physics, to test PhysicsVariableCoordinator. The solver variable is either a
 * finite element or a finite volume variable, with a diffusion kernel of the same discretization.
 */
class SharedVariablePhysics : public PhysicsBase
{
public:
  static InputParameters validParams();

  SharedVariablePhysics(const InputParameters & parameters);

  virtual InputParameters getAdditionalRMParams() const override;

private:
  virtual void addSolverVariables() override;
  virtual void addAuxiliaryVariables() override;
  virtual void addFEKernels() override;
  virtual void addFVKernels() override;

  /// Whether the shared solver variable is a finite volume variable
  const bool _fv;
  /// Order of the shared solver variable (finite element only)
  const std::string _order;
};
