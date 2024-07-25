//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
 * Creates all the objects needed to solve the Navier-Stokes mass and momentum equations
 * using the linear finite volume discretization
 * Currently does not implement:
 * - friction
 * - other momentum sources and sinks
 * - porous media
 */
class WCNSFVLinearFlowPhysics final : public WCNSFVFlowPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSFVLinearFlowPhysics(const InputParameters & parameters);

protected:
  virtual void initializePhysicsAdditional() override;

private:
  virtual void addNonlinearVariables() override;
  virtual void addFVKernels() override;
  virtual void addUserObjects() override;

  /// Function adding kernels for the incompressible pressure correction equation
  void addINSPressureCorrectionKernels();

  /**
   * Functions adding kernels for the incompressible momentum equation
   * If the material properties are not constant, these can be used for
   * weakly-compressible simulations (except the Boussinesq kernel) as well.
   */
  void addINSMomentumFluxKernels();
  virtual void addINSMomentumPressureKernels() override;
  virtual void addINSMomentumGravityKernels() override;
  virtual void addINSMomentumBoussinesqKernels() override;

  virtual void addINSInletBC() override;
  virtual void addINSOutletBC() override;
  virtual void addINSWallsBC() override;

  virtual bool hasForchheimerFriction() const override { return false; };

  virtual void addRhieChowUserObjects() override;

  UserObjectName rhieChowUOName() const override;

  /// Name of the vector to hold pressure momentum equation contributions
  const TagName _pressure_tag = "p_tag";

  /// Whether to use the correction term for non-orthogonality
  const bool _non_orthogonal_correction;
};
