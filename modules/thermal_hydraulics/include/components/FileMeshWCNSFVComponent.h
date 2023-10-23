//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FileMeshPhysicsComponent.h"

class WCNSFVPhysicsBase;
class WCNSFVFlowPhysics;
class WCNSFVHeatAdvectionPhysics;
class WCNSFVScalarAdvectionPhysics;

/**
 * Component with Navier Stokes module weakly compressible finite volume Physics objects active
 * on it
 */
class FileMeshWCNSFVComponent : public FileMeshPhysicsComponent
{
public:
  static InputParameters validParams();

  FileMeshWCNSFVComponent(const InputParameters & parameters);

  virtual void addRelationshipManagers(Moose::RelationshipManagerType input_rm_type) override;

  /// Whether flow physics are defined on this component
  bool hasFlowPhysics() const { return _has_flow_physics; }
  /// Whether temperature advection physics are defined on this component
  bool hasFluidEnergyPhysics() const { return _has_heat_physics; }
  /// Whether scalar advection physics are defined on this component
  bool hasScalarAdvectionPhysics() const { return _has_scalar_physics; }
  /// General form for convenience. Uses short names (flow/energy/scalar)
  bool hasPhysics(const PhysicsName & phys_name) const;

  /// Return one of the Physics defined on this component
  WCNSFVPhysicsBase * getPhysics(const PhysicsName & phys_name) const;
  /// Return the name of the variables by querying the physics
  VariableName getVariableName(const std::string & short_name) const;

protected:
  virtual void init() override;
  virtual void setupMesh() override;

  void addPhysics(const std::string & action_name,
                  const std::string & physics_name,
                  InputParameters & params);

private:
  /// Keeps track of the names of the Physics
  std::vector<PhysicsName> _physics_names;

  /// Flow physics
  WCNSFVFlowPhysics * _flow_physics;
  /// Fluid energy physics
  WCNSFVHeatAdvectionPhysics * _energy_physics;
  /// Scalar advection physics
  WCNSFVScalarAdvectionPhysics * _scalar_physics;

  /// Whether flow physics are defined on this component
  const bool _has_flow_physics;
  /// Whether temperature advection physics are defined on this component
  const bool _has_heat_physics;
  /// Whether scalar advection physics are defined on this component
  const bool _has_scalar_physics;
};
