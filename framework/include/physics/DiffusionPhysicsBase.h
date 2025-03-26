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
#include "PhysicsComponentInterface.h"

class ActionComponent;

#define registerDiffusionPhysicsBaseTasks(app_name, derived_name)                                  \
  registerPhysicsBaseTasks(app_name, derived_name);                                                \
  registerMooseAction(app_name, derived_name, "add_preconditioning");                              \
  registerMooseAction(app_name, derived_name, "add_postprocessor");                                \
  registerMooseAction(app_name, derived_name, "add_ic")

/**
 * Base class to host all common parameters and attributes of Physics actions to solve the diffusion
 * equation
 */
class DiffusionPhysicsBase : virtual public PhysicsBase, virtual public PhysicsComponentInterface
{
public:
  static InputParameters validParams();

  DiffusionPhysicsBase(const InputParameters & parameters);

protected:
  /// Name of the diffused variable
  const VariableName & _var_name;
  /// Boundaries on which a Neumann boundary condition is applied
  const std::vector<BoundaryName> & _neumann_boundaries;
  /// Boundaries on which a Dirichlet boundary condition is applied
  const std::vector<BoundaryName> & _dirichlet_boundaries;

private:
  virtual void addPreconditioning() override;
  /// Add postprocessing of the fluxes
  virtual void addPostprocessors() override;
  // The initial conditions for CG and FV can use the same classes
  virtual void addInitialConditions() override;
  virtual void addInitialConditionsFromComponents() override;
};
