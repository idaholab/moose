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
 * Currently does not implement:
 * - friction
 * - other momentum sources and sinks
 * - porous media
 * - transients
 */
class WCNSLinearFVFlowPhysics : public WCNSFVFlowPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSLinearFVFlowPhysics(const InputParameters & parameters);

  virtual std::vector<UserObjectName> getSuppliedUserObjects() const override;

protected:
  virtual void initializePhysicsAdditional() override;

  /// Function adding kernels for the incompressible pressure correction equation
  virtual void addPressureCorrectionKernels();

  virtual void addOutletBC() override;
  virtual void addWallsBC() override;

  virtual void addRhieChowUserObjects() override;
  virtual void addFunctorMaterials() override;

  /// The Rhie-Chow mass flux user object whose published flux the pressure equation corrects.
  const RhieChowMassFlux & pressureCorrectionProvider() const;

  /// The functor the Rhie-Chow object publishes the pressure equation's face coefficient as
  virtual MooseFunctorName pressureDiffusionCoefficientName() const { return "Ainv"; }
  /// The functor the Rhie-Chow object publishes the pressure equation's predictor face flux as
  virtual MooseFunctorName predictorFaceFluxName() const { return "HbyA"; }

  /// Checks that every pressure outlet has a pressure functor
  void checkPressureOutletFunctors() const;
  /// The velocity conditions of a no-slip wall: zero, or the wall functors of a moving wall
  void addNoSlipWallVelocityBCs(const BoundaryName & boundary_name);
  /// The velocity and pressure conditions of a symmetry wall
  void addSymmetryWallBCs(const BoundaryName & boundary_name);
  /// The parameters every Rhie-Chow object of this physics shares, for an object of \p type
  InputParameters rhieChowUserObjectParameters(const std::string & type) const;

  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

  /// Whether to use the correction term for non-orthogonality
  const bool _non_orthogonal_correction;

private:
  virtual void addSolverVariables() override;
  virtual void addFVInterpolationMethods() override;
  virtual void addFVKernels() override;
  virtual void addUserObjects() override;

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
  virtual void addSeparatorBC() override {}

  virtual bool hasForchheimerFriction() const override { return false; };

  virtual MooseFunctorName getLinearFrictionCoefName() const override
  {
    mooseError("Not implemented");
  }
};
