//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PhysicsBase.h"

class ActionComponent;

#define registerMultiSpeciesDiffusionPhysicsBaseTasks(app_name, derived_name)                      \
  registerPhysicsBaseTasks(app_name, derived_name);                                                \
  registerMooseAction(app_name, derived_name, "add_preconditioning");                              \
  registerMooseAction(app_name, derived_name, "add_postprocessor");                                \
  registerMooseAction(app_name, derived_name, "add_ic")

/**
 * Base class to host all common parameters and attributes of Physics actions to solve the diffusion
 * equation for multiple species
 */
class MultiSpeciesDiffusionPhysicsBase : public PhysicsBase
{
public:
  static InputParameters validParams();

  MultiSpeciesDiffusionPhysicsBase(const InputParameters & parameters);

  void addComponent(const ActionComponent & component) override;

protected:
  /// Name of the diffused variables
  const std::vector<VariableName> & _species_names;
  /// Number of species
  const unsigned int _num_species;
  /// Boundaries on which a Neumann boundary condition is applied. Outer indexing is variables
  const std::vector<std::vector<BoundaryName>> & _neumann_boundaries;
  /// Boundaries on which a Dirichlet boundary condition is applied. Outer indexing is variables
  const std::vector<std::vector<BoundaryName>> & _dirichlet_boundaries;

  /// Whether to use automatic differentiation or not
  const bool _use_ad;

private:
  /// Add default preconditioning options
  virtual void addPreconditioning() override;
  /// Add postprocessing of the fluxes
  virtual void addPostprocessors() override;
  /// Add initial conditions for each diffused variable
  virtual void addInitialConditions() override;
};
