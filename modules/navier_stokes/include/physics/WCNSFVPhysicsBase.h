//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NavierStokesPhysicsBase.h"

class WCNSFVFlowPhysics;

#define registerWCNSFVPhysicsBaseTasks(app_name, derived_name)                                     \
  registerPhysicsBaseTasks(app_name, derived_name);                                                \
  registerMooseAction(app_name, derived_name, "add_geometric_rm");

/**
 * Base class to hold common parameters and utilities between all the weakly compressible
 * Navier Stokes-based equations (WCNSFV)
 * Includes incompressible flow (INSFV).
 */
class WCNSFVPhysicsBase : public NavierStokesPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSFVPhysicsBase(const InputParameters & parameters);

  void initializePhysicsAdditional() override;

  void findCoupledFlowPhysics();

protected:
  /// Detects if we are using the new Physics syntax or the old NavierStokesFV action
  bool usingNavierStokesFVSyntax() const
  {
    return (parameters().get<std::string>("registered_identifier") == "Modules/NavierStokesFV");
  }

  /// Parameters to change or add relationship managers
  InputParameters getAdditionalRMParams() const override;

  /// Return the number of ghosting layers needed
  virtual unsigned short getNumberAlgebraicGhostingLayersNeeded() const = 0;

  /// Whether to define variables if they do not exist
  bool _define_variables;

  /// A physics object defining the flow equations
  const WCNSFVFlowPhysics * _flow_equations_physics;

private:
  /// Check whether another flow Physics object has been specified
  bool hasCoupledFlowPhysics() const { return !(!_flow_equations_physics); };
};
