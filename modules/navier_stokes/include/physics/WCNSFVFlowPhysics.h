//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSFVFlowPhysicsBase.h"
#include "WCNSFVTurbulencePhysics.h"

/**
 * Creates all the objects needed to solve the Navier Stokes mass and momentum equations
 */
class WCNSFVFlowPhysics final : public WCNSFVFlowPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSFVFlowPhysics(const InputParameters & parameters);

  /// Get the name of the linear friction coefficient. Returns an empty string if no friction.
  virtual MooseFunctorName getLinearFrictionCoefName() const override;
  /// Return the name of the Rhie Chow user object
  UserObjectName rhieChowUOName() const override;
  /// Return the number of algebraic ghosting layers needed
  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

private:
  virtual void addSolverVariables() override;
  virtual void addFVKernels() override;
  virtual void addUserObjects() override;
  virtual void addCorrectors() override;

  /// Function adding kernels for the time derivative term of the weakly compressible continuity equation
  void addMassTimeKernels();
  /// Function adding kernels for the incompressible continuity equation
  void addMassKernels();
  /// Function adding the pressure constraint
  void addPressurePinKernel();

  /**
   * Functions adding kernels for the incompressible momentum equation
   * If the material properties are not constant, these can be used for
   * weakly-compressible simulations (except the Boussinesq kernel) as well.
   */
  void addMomentumTimeKernels() override;
  void addMomentumViscousDissipationKernels();
  void addMomentumMixingLengthKernels();
  void addMomentumAdvectionKernels();
  void addMomentumPressureKernels() override;
  void addMomentumGravityKernels() override;
  void addMomentumBoussinesqKernels() override;
  void addMomentumFrictionKernels() override;

  /// Functions adding boundary conditions for the incompressible simulation.
  /// These are used for weakly-compressible simulations as well.
  void addInletBC() override;
  void addOutletBC() override;
  void addWallsBC() override;
  void addSeparatorBC() override;

  /// Return whether a Forchheimer friction model is in use
  bool hasForchheimerFriction() const override;

  /// Function which adds the RhieChow interpolator user objects for weakly and incompressible formulations
  void addRhieChowUserObjects() override;
  /// Checks that sufficient Rhie Chow coefficients have been defined for the given dimension, used
  /// for scalar or temperature advection by auxiliary variables
  void checkRhieChowFunctorsDefined() const;

  /// The number of smoothing layers if that treatment is used on porosity
  const unsigned _porosity_smoothing_layers;

  /// Name of the user object in charge of computing the Rhie Chow coefficients
  UserObjectName _rc_uo_name;
};
