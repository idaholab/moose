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
#include "THMVariableCoordinator.h"

/**
 * Junction with a non-zero volume connecting an arbitrary number of FlowChannel1PhaseAC boundaries
 * for 1-phase flow, as a native ActionComponent.
 *
 * Unlike JunctionOneToOne1PhaseAC, this junction has its own mesh (a single NodeElem, built via
 * ElementGenerator, at 'position'), its own solver variables (rhoV, rhouV, rhovV, rhowV, rhoEV) and
 * auxiliary variables (p, T, vel), and its own kernels (ADTimeDerivative + the advective coupling
 * term). A single ADVolumeJunction1PhaseUserObject computes the fluxes exchanged with each
 * connected flow channel plus the junction's own residual; one ADVolumeJunction1PhaseBC per
 * solution variable per connection applies that flux as a boundary condition on the connected flow
 * channel. As with FlowChannel1PhasePhysics's own variables, the junction's variables are routed
 * through THMVariableCoordinator, so that multiple VolumeJunction1PhaseAC instances in one
 * simulation can share the same bare variable names (rhoV, p, ...) across their (disjoint) blocks,
 * exactly as classic THM's VolumeJunction1Phase components do via Simulation::addSimVariable().
 *
 * The junction's own NodeElem is not mesh-adjacent to any connected flow channel's end element
 * (each built and combined by independent MeshGenerators), so cross-element Jacobian coupling is
 * added explicitly via AugmentSparsityBetweenElements, the same mechanism JunctionOneToOne1PhaseAC
 * uses. Unlike that one-to-one junction, only junction-to-flow-channel coupling is needed here (not
 * flow-channel-to-flow-channel), because every governing equation that a connected flow channel's
 * boundary condition depends on reads only that flow channel's own state plus the junction's state
 * - this mirrors classic THM's VolumeJunction1Phase::setupMesh(), which likewise only calls
 * Simulation::augmentSparsity() between each connected element and the junction's NodeElem.
 */
class VolumeJunction1PhaseAC : public virtual ActionComponent, public NamingInterface
{
public:
  static InputParameters validParams();
  VolumeJunction1PhaseAC(const InputParameters & params);

  virtual std::vector<UserObjectName> dependsOnUserObjects() const override;

protected:
  virtual void addMeshGenerators() override;
  virtual void addSolverVariables() override;
  virtual void addUserObjects() override;
  virtual void addMaterials() override;
  virtual void actOnAdditionalTasks() override;
  virtual void addRelationshipManagers(Moose::RelationshipManagerType when_type) override;

  /// Returns the physics of the i-th connected flow channel component (looked up by name, lazily)
  const FlowChannel1PhasePhysics & connectedPhysics(unsigned int i) const;
  /// Returns the fluid properties user object name shared by all connected flow channels, erroring
  /// if they do not all use the same one
  UserObjectName fluidPropertiesName() const;

  /// Requests the junction's auxiliary variables (p, T, vel) from the THMVariableCoordinator
  void addAuxiliaryVariables();
  /// Adds a VolumeJunction1PhaseIC for each junction variable
  void addInitialConditions();
  /// Adds one ADVolumeJunction1PhaseBC per solution variable per connection
  void addWeakBCs();
  /// Adds the ADTimeDerivative and ADVolumeJunctionAdvectionKernel kernels for the junction
  void addKernels();
  /// Adds the pressure/temperature/speed VolumeJunction1PhaseAux auxiliary kernels
  void addAuxiliaryKernels();
  /// Adds a VolumeJunction1PhaseIC for a single junction variable
  void addVolumeJunctionIC(const VariableName & var_name, const std::string & quantity);

  /// The flow channel boundaries to connect, e.g. 'pipeA:out', 'pipeB:in', 'pipeC:in', ...
  const std::vector<BoundaryName> _connections;
  /// Names of the connected flow channel components, parsed from _connections
  std::vector<std::string> _connected_component_names;
  /// Outward normal direction at each connection: -1 at a ':in' end, +1 at a ':out' end
  std::vector<Real> _normals;

  /// Volume of the junction
  const Real _volume;
  /// Spatial position of the center of the junction
  const Point _position;

  /// Scaling factors for the junction's solver variables
  const Real _scaling_factor_rhoV;
  const Real _scaling_factor_rhouV;
  const Real _scaling_factor_rhovV;
  const Real _scaling_factor_rhowV;
  const Real _scaling_factor_rhoEV;

  /// Form loss coefficient
  const Real _K;
  /// Reference area, used with the form loss coefficient
  const Real _A_ref;
  /// Whether to apply the velocity scaling; see ADVolumeJunction1PhaseUserObject
  const bool _apply_velocity_scaling;

  /// Name of the junction's flux/residual UserObject
  const UserObjectName _junction_uo_name;

  /// Coordinator that merges this and other volume junctions' requests for shared, bare-named
  /// variables (rhoV, p, ...) into single MOOSE variables spanning the union of their blocks
  THMVariableCoordinator & _coordinator;

  /// Map of element ID -> connected element IDs on the other side of the coupling, populated in
  /// addUserObjects() (once the mesh exists) and consumed by AugmentSparsityBetweenElements. Maps
  /// the junction's own element to every connected flow channel element, and vice versa.
  std::map<dof_id_type, std::vector<dof_id_type>> _elem_map;
  /// Whether the AugmentSparsityBetweenElements relationship manager has already been added
  bool _rm_added;

  /// Enumeration for junction variable/equation indices, matching classic VolumeJunction1Phase
  enum VolumeJunction1PhaseIndices
  {
    RHOV_INDEX = 0,
    RHOUV_INDEX = 1,
    RHOVV_INDEX = 2,
    RHOWV_INDEX = 3,
    RHOEV_INDEX = 4
  };
  /// Number of equations for the junction
  static const unsigned int N_EQ;

  /// Junction variable names
  const VariableName _rhoV_var_name;
  const VariableName _rhouV_var_name;
  const VariableName _rhovV_var_name;
  const VariableName _rhowV_var_name;
  const VariableName _rhoEV_var_name;
  const VariableName _pressure_var_name;
  const VariableName _temperature_var_name;
  const VariableName _velocity_var_name;
};
