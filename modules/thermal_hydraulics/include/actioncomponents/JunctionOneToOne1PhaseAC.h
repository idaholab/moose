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
 * Junction connecting one FlowChannel1PhaseAC to one other FlowChannel1PhaseAC for 1-phase flow,
 * as a native ActionComponent. Adds no mesh/DOFs of its own - purely a UserObject
 * (ADJunctionOneToOne1PhaseUserObject) plus one ADJunctionOneToOne1PhaseBC per solution variable
 * on each of the two connected boundaries, referencing that UserObject, the same shape as
 * FlowBoundary1PhaseAC's derivatives but with flux exchanged between two flow channels instead of
 * a single prescribed ghost state.
 *
 * Because the two connected flow channels' meshes are not adjacent (each independently built and
 * combined by its own MeshGenerators), the Jacobian needs an explicit cross-element coupling
 * declaration - added via AugmentSparsityBetweenElements, the same RelationshipManager classic
 * THM's Component1DJunction uses (through Simulation::augmentSparsity).
 */
class JunctionOneToOne1PhaseAC : public virtual ActionComponent, public NamingInterface
{
public:
  static InputParameters validParams();
  JunctionOneToOne1PhaseAC(const InputParameters & params);

  virtual std::vector<UserObjectName> dependsOnUserObjects() const override
  {
    return {connectedPhysics(0).numericalFluxUserObjectName(),
            connectedPhysics(1).numericalFluxUserObjectName()};
  }

protected:
  virtual void actOnAdditionalTasks() override;
  virtual void addUserObjects() override;
  virtual void addRelationshipManagers(Moose::RelationshipManagerType when_type) override;

  /// Returns the physics of the i-th connected flow channel component (looked up by name, lazily)
  const FlowChannel1PhasePhysics & connectedPhysics(unsigned int i) const;
  /// Adds one ADJunctionOneToOne1PhaseBC per solution variable per connected boundary
  void addWeakBCs();

  /// The two flow channel boundaries to connect, e.g. 'pipeA:out' and 'pipeB:in'
  const std::vector<BoundaryName> _connections;
  /// Names of the two connected flow channel components, parsed from _connections
  std::vector<std::string> _connected_component_names;
  /// Outward normal direction at each connection: -1 at a ':in' end, +1 at a ':out' end
  std::vector<Real> _normals;
  /// Name of the junction's flux UserObject
  const UserObjectName _junction_uo_name;

  /// Map of element ID -> connected element IDs on the other side of the junction, populated in
  /// addUserObjects() (once the mesh exists) and consumed by AugmentSparsityBetweenElements
  std::map<dof_id_type, std::vector<dof_id_type>> _elem_map;
  /// Whether the AugmentSparsityBetweenElements relationship manager has already been added
  bool _rm_added;
};
