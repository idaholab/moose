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

#define registerNavierStokesPhysicsBaseTasks(app_name, derived_name)                               \
  registerPhysicsBaseTasks(app_name, derived_name);                                                \
  registerMooseAction(app_name, derived_name, "add_geometric_rm")

/**
 * Base class to hold common parameters and utilities between all the weakly compressible
 * Navier Stokes-based equations (WCNSFV)
 * Includes incompressible flow (INSFV).
 */
class NavierStokesPhysicsBase : public PhysicsBase
{
public:
  static InputParameters validParams();

  NavierStokesPhysicsBase(const InputParameters & parameters);

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
};
