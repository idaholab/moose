//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NavierStokesFlowPhysics.h"

class WCNSFVFlowPhysics;

/**
 * Base class to hold common parameters and utilities between all the weakly compressible
 * Navier Stokes-based equations
 * Includes incompressible flow (INSFV).
 */
class WCNSFVPhysicsBase : public NavierStokesFlowPhysics
{
public:
  static InputParameters validParams();

  WCNSFVPhysicsBase(const InputParameters & parameters);

  /// GeneralUO not the right base class probably
  virtual void initialize() override{};
  virtual void execute() override{};
  virtual void finalize() override{};

protected:
  /// Add user objects: for now mainly the Rhie Chow user object
  virtual void addUserObjects() override;
  /// Add postprocessors, could be moved up to the base class
  void addPostprocessors() override;

  /// The velocity / momentum face interpolation method for advecting other quantities
  const MooseEnum _velocity_interpolation;

  /// A physics object defining the flow equations
  const WCNSFVFlowPhysics * _flow_equations_physics;

private:
  /// Function which adds the RhieChow interpolator user objects for weakly and incompressible formulations
  void addRhieChowUserObjects();

  /// Check whether another Physics object has been specified
  bool hasCoupledFlowPhysics() const { return !(!_flow_equations_physics); };

  /// Checks that sufficient Rhie Chow coefficients have been defined for the given dimension, used
  /// for scalar or temperature advection by auxiliary variables
  void checkRhieChowFunctorsDefined() const;
};
