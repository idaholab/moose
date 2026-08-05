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

class RhieChowMassFlux;
class UserObject;

/**
 * Creates all the objects needed to solve the Navier-Stokes equations with the SIMPLE algorithm
 * using the linear finite volume discretization.
 *
 * Derived classes may override the protected hooks to customize selected pressure, momentum, and
 * boundary-condition objects while reusing the stock linear-FV flow setup.
 */
class WCNSLinearFVFlowPhysics : public WCNSFVFlowPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSLinearFVFlowPhysics(const InputParameters & parameters);

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

  MooseFunctorName getLinearFrictionCoefName() const override { mooseError("Not implemented"); }

  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

  virtual void addAdditionalUserObjects() {}
  virtual bool useMomentumContinuityErrorSink() const { return false; }
  virtual bool shouldAddMomentumPressureKernels() const { return true; }
  virtual bool shouldAddMomentumReducedPressureKernels() const { return false; }
  virtual MooseFunctorName pressureDiffusionTensorName() const { return "Ainv"; }
  virtual MooseFunctorName pressureDivergenceFluxName() const { return "HbyA"; }
  virtual bool pressureDivergenceFluxIsIntegrated() const { return false; }
  virtual std::string momentumTimeKernelType() const { return "LinearFVTimeDerivative"; }
  virtual std::string momentumTimeDensityParameterName() const { return "factor"; }
  virtual void setMomentumTimeKernelParams(InputParameters & params) const;
  void setVelocitySolverVariableParams(InputParameters & params) const;
  void setVelocityVariableParams(InputParameters & params) const;
  virtual std::string rhieChowUserObjectType() const { return "RhieChowMassFlux"; }
  virtual bool rhieChowUserObjectAppliesToBlocks(const RhieChowMassFlux & rc_obj) const;
  virtual bool isCompatibleRhieChowUserObject(const UserObject & obj) const;
  virtual void checkIncompatibleRhieChowUserObject(const RhieChowMassFlux & rc_obj) const;
  virtual void setRhieChowUserObjectParams(InputParameters & params) const;
  virtual std::string momentumFluxKernelType() const { return "LinearWCNSFVMomentumFlux"; }
  virtual MooseFunctorName momentumFluxMassFluxFunctorName() const { return ""; }
  virtual bool momentumFluxMassFluxFunctorIsIntegrated() const { return false; }
  virtual std::string momentumOutletBCType(const BoundaryName & boundary,
                                           const MooseEnum & momentum_outlet_type) const;
  virtual void setMomentumOutletBCParams(InputParameters & params,
                                         const BoundaryName & boundary,
                                         const MooseEnum & momentum_outlet_type,
                                         unsigned int component) const;
  virtual std::string pressureOutletBCType(const BoundaryName & boundary,
                                           const MooseEnum & momentum_outlet_type) const;
  virtual void setPressureOutletBCParams(InputParameters & params,
                                         const BoundaryName & boundary,
                                         const MooseEnum & momentum_outlet_type) const;
  virtual MooseFunctorName wallPressureSymmetryFluxName() const { return "HbyA"; }
  virtual void addWallPressureBC(const BoundaryName & boundary,
                                 const MooseEnum & momentum_wall_type);
  virtual bool shouldAddWallPressureTwoTermExpansion() const
  {
    return getParam<bool>("pressure_two_term_bc_expansion");
  }
  virtual MooseFunctorName inletVelocityFunctorName(const BoundaryName & boundary,
                                                    unsigned int component) const;
  virtual MooseFunctorName wallVelocityFunctorName(const BoundaryName & boundary,
                                                   unsigned int component) const;

  /// Whether to use the correction term for non-orthogonality
  const bool _non_orthogonal_correction;
};
