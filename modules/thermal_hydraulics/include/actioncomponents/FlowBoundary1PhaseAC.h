//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActionComponent.h"
#include "NamingInterface.h"
#include "FlowChannel1PhasePhysics.h"

/**
 * Base class for a boundary condition ActionComponent attaching to one end of a
 * FlowChannel1PhaseAC, addressed the same way classic THM connects boundary components: through
 * an 'input' parameter of the form 'component_name:in' or 'component_name:out'.
 *
 * Derived classes only need to provide the boundary's ghost-state UserObject (addUserObjects());
 * the weak boundary condition itself (one ADBoundaryFlux3EqnBC per solution variable, referencing
 * that UserObject) is common and added by this class on the 'add_bc' task.
 */
class FlowBoundary1PhaseAC : public virtual ActionComponent, public NamingInterface
{
public:
  static InputParameters validParams();
  FlowBoundary1PhaseAC(const InputParameters & params);

public:
  virtual std::vector<UserObjectName> dependsOnUserObjects() const override
  {
    return {connectedPhysics().numericalFluxUserObjectName()};
  }

protected:
  virtual void actOnAdditionalTasks() override;

  /// Returns the physics of the connected flow channel component (looked up by name, lazily, so
  /// that 'input' may name a component declared before or after this one)
  const FlowChannel1PhasePhysics & connectedPhysics() const;
  /// Adds one ADBoundaryFlux3EqnBC per solution variable (rhoA, rhouA, rhoEA), all referencing
  /// _boundary_uo_name
  void addWeakBCs();

  /// The boundary this component attaches to, e.g. 'pipe:out' - also the actual sideset name on
  /// the connected flow channel's mesh
  const BoundaryName _input;
  /// Name of the connected flow channel component, parsed from 'input'
  const std::string _connected_component_name;
  /// Outward normal direction at this boundary: -1 at the ':in' end, +1 at the ':out' end
  const Real _normal;
  /// Name of this boundary's ghost-state UserObject, created by the derived class
  const UserObjectName _boundary_uo_name;
};
