//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatConductionCG.h"
#include "THMVariableCoordinator.h"

/**
 * Heat conduction physics for a 2D heat structure ActionComponent (see HeatStructureCylindricalAC/
 * HeatStructurePlateAC, which create and attach one of these to themselves in their constructor).
 *
 * This is a thin subclass of the framework's generic HeatConductionCG Physics: it inherits
 * addFEKernels()/addFEBCs()/addMaterials()/addPreconditioning() unchanged, and overrides only
 * addSolverVariables()/addInitialConditions() to route through THMVariableCoordinator instead of
 * HeatConductionCG's own direct problem-adding calls. This lets multiple heat structures in one
 * simulation share the bare 'T_solid' variable name across their (disjoint) blocks, the same way
 * classic THM's heat structures all share Simulation::addSimVariable()'s single 'T_solid' field -
 * PhysicsBase's own shouldCreateVariable()/shouldCreateIC() guards explicitly do not support this
 * (they error on a variable/IC that exists but does not already cover the requested blocks).
 */
class HeatStructurePhysics : public HeatConductionCG
{
public:
  static InputParameters validParams();
  HeatStructurePhysics(const InputParameters & params);

private:
  virtual void addSolverVariables() override;
  virtual void addInitialConditions() override;

  /// Coordinator that merges this and other components' requests for shared, bare-named
  /// variables (T_solid, rhoA, ...) into single MOOSE variables spanning the union of their blocks
  THMVariableCoordinator & _coordinator;
};
