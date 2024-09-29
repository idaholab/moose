//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowBoundary.h"

class ThermalHydraulicsFlowPhysics;

/**
 * Base class for boundary components connected to flow channels leveraging Physics
 */
class PhysicsFlowBoundary : public FlowBoundary
{
public:
  static InputParameters validParams();

  PhysicsFlowBoundary(const InputParameters & params);

  /// Whether the flow boundary is reversible
  virtual bool isReversible() const
  {
    mooseError("isReversible not implemented for this flow boundary");
  }

  /// The name of the boundary user object
  const UserObjectName & getBoundaryUOName() const { return _boundary_uo_name; }

  /// Whether the flow boundary is a flux boundary for the equation with this variable
  virtual bool isFluxBoundary(const VariableName & /*var_name*/) const
  {
    mooseError("Not implemented");
  }
  /// Whether the flow boundary is a Dirichlet boundary for this variable
  virtual bool isValueBoundary(const VariableName & /*var_name*/) const
  {
    mooseError("Not implemented");
  }
  /// Returns the boundary value for this variable
  virtual MooseFunctorName getBoundaryValue(const VariableName & /*var_name*/) const
  {
    mooseError("Not implemented");
  }
  /// Returns the boundary flux for the equation associated with this variable
  virtual MooseFunctorName getBoundaryFlux(const VariableName & /*var_name*/) const
  {
    mooseError("Not implemented");
  }

protected:
  virtual void init() override;
  virtual void check() const override;

  /// Numerical flux user object name
  UserObjectName _numerical_flux_name;
  /// Name of boundary user object name
  const UserObjectName _boundary_uo_name;
  /// Thermal hydraulics Physics active on this flow boundary
  std::set<ThermalHydraulicsFlowPhysics *> _th_physics;
};
