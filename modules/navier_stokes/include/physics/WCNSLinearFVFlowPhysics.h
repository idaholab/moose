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

class RhieChowMassFlux;

/**
 * Creates all the objects needed to solve the Navier-Stokes equations with the SIMPLE algorithm
 * using the linear finite volume discretization
 * Porous media use superficial velocity variables, the PorousRhieChowMassFlux user object and,
 * optionally, Bernoulli pressure-jump baffles on user-selected sidesets.
 */
class WCNSLinearFVFlowPhysics : public WCNSFVFlowPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSLinearFVFlowPhysics(const InputParameters & parameters);

  virtual std::vector<UserObjectName> getSuppliedUserObjects() const override;

protected:
  virtual void initializePhysicsAdditional() override;

  virtual void addFunctorMaterials() override;

  /// The Rhie-Chow mass flux user object whose published flux the pressure equation corrects.
  const RhieChowMassFlux & pressureCorrectionProvider() const;

  /// The functor the Rhie-Chow object publishes the pressure equation's face coefficient as
  virtual MooseFunctorName pressureDiffusionCoefficientName() const { return "Ainv"; }
  /// The functor the Rhie-Chow object publishes the pressure equation's predictor face flux as
  virtual MooseFunctorName predictorFaceFluxName() const { return "HbyA"; }

private:
  virtual void actOnAdditionalTasks() override;
  virtual void addSolverVariables() override;
  virtual void addFVInterpolationMethods() override;
  /// Add the jump-aware and reconstructed gradient methods required by pressure baffles
  void addFVGradientMethods();
  /// Add the pressure-jump model acting on the baffle sidesets
  void addPressureJumpModel();
  virtual void addFVKernels() override;
  virtual void addUserObjects() override;

  /// Function adding kernels for the incompressible pressure correction equation
  void addPressureCorrectionKernels();

  /**
   * Functions adding kernels for the incompressible momentum equation
   * If the material properties are not constant, these can be used for
   * weakly-compressible simulations (except the Boussinesq kernel) as well.
   */
  void addMomentumTimeKernels() override;
  void addMomentumFluxKernels();
  virtual void addAxisymmetricViscousSourceKernel(const std::vector<SubdomainName> & rz_blocks,
                                                  unsigned int radial_index) override;
  virtual void addMomentumPressureKernels() override;
  virtual void addMomentumGravityKernels() override;
  virtual void addMomentumFrictionKernels() override;
  virtual void addMomentumBoussinesqKernels() override;

  virtual void addInletBC() override;
  virtual void addOutletBC() override;
  virtual void addWallsBC() override;
  virtual void addSeparatorBC() override {}

  virtual bool hasForchheimerFriction() const override;

  virtual void addRhieChowUserObjects() override;

  virtual MooseFunctorName getLinearFrictionCoefName() const override
  {
    mooseError("Not implemented");
  }

  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

  /// Whether to use the correction term for non-orthogonality
  const bool _non_orthogonal_correction;
  /// Sidesets carrying Bernoulli pressure-jump baffles (porous medium treatment only)
  const std::vector<BoundaryName> _baffle_sidesets;
  /// Whether pressure-jump baffles are present
  const bool _has_baffles;
  /// Whether the momentum pressure kernels use the reconstructed pressure gradient
  const bool _use_reconstructed_pressure_gradient;
};
