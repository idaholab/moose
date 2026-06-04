//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "WCNSFVFlowPhysicsBase.h"
#include "WCNSFVTurbulencePhysics.h"

/**
 * Shared setup for segregated Linear FV flow physics.
 *
 * This base owns the common velocity/pressure variable setup and the standard Linear FV flow kernel
 * choreography. Derived classes override the small set of hooks that differ between the stock and
 * sharp-interface formulations.
 */
class WCNSLinearFVFlowPhysicsBase : public WCNSFVFlowPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSLinearFVFlowPhysicsBase(const InputParameters & parameters);

protected:
  void initializePhysicsAdditional() override;
  void addSolverVariables() override;
  void addFVKernels() override;
  void addUserObjects() override;
  void addFunctorMaterials() override;

  void addPressureCorrectionKernels();

  void addMomentumTimeKernels() override;
  virtual void addMomentumFluxKernels();
  void addMomentumPressureKernels() override;
  void addMomentumGravityKernels() override;
  void addMomentumFrictionKernels() override;
  void addMomentumBoussinesqKernels() override;
  virtual void addMomentumConditioningKernels() {}
  virtual void addMomentumReducedPressureKernels() {}

  void addInletBC() override;
  void addOutletBC() override;
  void addWallsBC() override;
  void addSeparatorBC() override {}

  bool hasForchheimerFriction() const override { return false; }

  void addRhieChowUserObjects() override;

  MooseFunctorName getLinearFrictionCoefName() const override
  {
    mooseError("Not implemented");
  }

  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

  virtual void addAdditionalUserObjects() {}
  virtual bool useMomentumContinuityErrorSink() const { return false; }
  virtual bool shouldAddMomentumPressureKernels() const { return true; }
  virtual bool shouldAddMomentumReducedPressureKernels() const { return false; }
  virtual MooseFunctorName pressureDiffusionTensorName() const { return "Ainv"; }
  virtual MooseFunctorName pressureDivergenceFluxName() const { return "HbyA"; }
  virtual std::string momentumTimeKernelType() const { return "LinearFVTimeDerivative"; }
  virtual std::string momentumTimeDensityParameterName() const { return "factor"; }
  virtual std::string momentumFluxKernelType() const { return "LinearWCNSFVMomentumFlux"; }
  virtual MooseFunctorName momentumFluxMassFluxFunctorName() const { return ""; }
  virtual MooseFunctorName inletVelocityFunctorName(const BoundaryName & boundary,
                                                    unsigned int component) const;
  virtual MooseFunctorName wallVelocityFunctorName(const BoundaryName & boundary,
                                                   unsigned int component) const;

  /// Whether to use the correction term for non-orthogonality
  const bool _non_orthogonal_correction;
};
