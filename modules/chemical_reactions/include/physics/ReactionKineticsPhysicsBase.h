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
#include "ReactionNetworkUtils.h"

class ActionComponent;

#define registerReactionKineticsPhysicsBaseTasks(app_name, derived_name)                           \
  registerPhysicsBaseTasks(app_name, derived_name);                                                \
  registerMooseAction(app_name, derived_name, "add_variable");                                     \
  registerMooseAction(app_name, derived_name, "add_aux_variable");                                 \
  registerMooseAction(app_name, derived_name, "add_ic")

/**
 * Base class to host all common parameters and attributes of Physics actions to solve the diffusion
 * equation for multiple species
 */
class ReactionKineticsPhysicsBase : public PhysicsBase
{
public:
  static InputParameters validParams();

  ReactionKineticsPhysicsBase(const InputParameters & parameters);

  void addComponent(const ActionComponent & component) override;

protected:
  // Notably reads the reaction network
  virtual void initializePhysicsAdditional() override;

  /// Name of the species variables to solve for in the reaction network
  const std::vector<VariableName> & _solver_species;
  /// Number of species to solve for
  const unsigned int _num_solver_species;
  /// Name of the species variables that can be computed without additional solves, simply auxkernels
  const std::vector<AuxVariableName> & _aux_species;
  /// Number of auxiliary species
  const unsigned int _num_aux_species;
  /// Reaction network after being parsed in initializePhysics()
  std::vector<ReactionNetworkUtils::Reaction> _reactions;

  /// Number of reactions involved in the network
  unsigned int _num_reactions;
  /// Vector of vectors, indexed by (i, j), of whether primary solver species 'i' is present in reaction 'j'
  std::vector<std::vector<bool>> _primary_participation;

private:
  /// Add solver variables (currently coded for CGFE)
  virtual void addSolverVariables() override;
  /// Add nodal auxiliary variables
  virtual void addAuxiliaryVariables() override;
  /// Add default preconditioning options (not implemented at this time)
  virtual void addPreconditioning() override;
  /// Add initial conditions for the solver variable (auxiliary not implemented)
  virtual void addInitialConditions() override;
};
