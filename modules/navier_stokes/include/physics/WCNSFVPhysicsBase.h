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
  registerMooseAction(app_name, derived_name, "add_user_object");                                  \
  registerMooseAction(app_name, derived_name, "add_postprocessor");                                \
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

  /// Add user objects: for now mainly the Rhie Chow user object
  virtual void addUserObjects() override;
  /// Add postprocessors, could be moved up to the base class
  void addPostprocessors() override;

  /// Convenience routine to be able to retrieve the actual variable names from their default names
  VariableName getFlowVariableName(const std::string & default_name) const;

protected:
  InputParameters getAdditionalRMParams() const override;
  virtual unsigned short getNumberAlgebraicGhostingLayersNeeded() const;

  /// Return the name of the Rhie Chow user object
  std::string rhieChowUOName() const;

  /// Checks that the parameters shared between two flow physics are the same
  void checkCommonParametersConsistent(const InputParameters & parameters) const override;

  /// Detects if we are using the new Physics syntax or the old NavierStokesFV action
  bool usingWCNSFVPhysics() const
  {
    return !(parameters().get<std::string>("registered_identifier") == "Modules/NavierStokesFV");
  }

  /// Whether to define variables if they do not exist
  bool _define_variables;

  /// The velocity / momentum face interpolation method for advecting other quantities
  const MooseEnum _velocity_interpolation;

  /// A physics object defining the flow equations
  const WCNSFVFlowPhysics * _flow_equations_physics;

  /// Postprocessors describing the momentum inlet for each boundary. Indexing based on the number of flux boundaries
  std::vector<PostprocessorName> _flux_inlet_pps;
  /// Direction of each flux inlet. Indexing based on the number of flux boundaries
  std::vector<Point> _flux_inlet_directions;

private:
  /// Function which adds the RhieChow interpolator user objects for weakly and incompressible formulations
  void addRhieChowUserObjects();

  /// Check whether another flow Physics object has been specified
  bool hasCoupledFlowPhysics() const { return !(!_flow_equations_physics); };

  /// Checks that sufficient Rhie Chow coefficients have been defined for the given dimension, used
  /// for scalar or temperature advection by auxiliary variables
  void checkRhieChowFunctorsDefined() const;
};
