//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSLinearFVFlowPhysics.h"
#include "NS.h"

/**
 * Linear-FV segregated diffuse-interface flow physics for velocity-component momentum unknowns.
 *
 * This class reuses the common segregated Linear FV flow setup and overrides the diffuse-interface
 * pieces needed for large-density-ratio conservative coupling.
 */
class WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics final : public WCNSLinearFVFlowPhysics
{
public:
  static InputParameters validParams();

  WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics(const InputParameters & parameters);

private:
  void addFunctorMaterials() override;

  MooseFunctorName generatedGeometryFunctorName(const std::string & base_name) const;
  bool shouldAddMomentumPressureKernels() const override;
  MooseFunctorName pressureDiffusionTensorName() const override { return "pressure_Ainv"; }
  MooseFunctorName pressureDivergenceFluxName() const override { return "pressure_predictor_flux"; }
  bool pressureDivergenceFluxIsIntegrated() const override { return true; }
  void setMomentumTimeKernelParams(InputParameters & params) const override;
  MooseFunctorName momentumFluxMassFluxFunctorName() const override { return "rho_phi"; }
  bool momentumFluxMassFluxFunctorIsIntegrated() const override { return true; }
  std::string momentumOutletBCType(const BoundaryName & boundary,
                                   const MooseEnum & momentum_outlet_type) const override;
  void setMomentumOutletBCParams(InputParameters & params,
                                 const BoundaryName & boundary,
                                 const MooseEnum & momentum_outlet_type,
                                 unsigned int component) const override;
  std::string pressureOutletBCType(const BoundaryName & boundary,
                                   const MooseEnum & momentum_outlet_type) const override;
  void setPressureOutletBCParams(InputParameters & params,
                                 const BoundaryName & boundary,
                                 const MooseEnum & momentum_outlet_type) const override;
  MooseFunctorName wallPressureSymmetryFluxName() const override { return "phiHbyA"; }
  void addWallPressureBC(const BoundaryName & boundary,
                         const MooseEnum & momentum_wall_type) override;
  bool shouldAddWallPressureTwoTermExpansion() const override;
  std::string rhieChowUserObjectType() const override
  {
    return "ConservativeDiffuseInterfaceRhieChowMassFlux";
  }
  bool rhieChowUserObjectAppliesToBlocks(const RhieChowMassFlux & rc_obj) const override;
  bool isCompatibleRhieChowUserObject(const UserObject & obj) const override;
  void checkIncompatibleRhieChowUserObject(const RhieChowMassFlux & rc_obj) const override;
  void setRhieChowUserObjectParams(InputParameters & params) const override;
  bool shouldCreateGeometryFunctorMaterial() const;

  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

  const bool _create_geometry_functors;
};
