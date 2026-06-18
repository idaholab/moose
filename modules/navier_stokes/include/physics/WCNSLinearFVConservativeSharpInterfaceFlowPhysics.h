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
 * Linear-FV segregated sharp-interface flow physics for velocity-component momentum unknowns.
 *
 * This class reuses the common segregated Linear FV flow setup and overrides the sharp-interface
 * pieces needed for large-density-ratio conservative coupling.
 */
class WCNSLinearFVConservativeSharpInterfaceFlowPhysics final : public WCNSLinearFVFlowPhysics
{
public:
  static InputParameters validParams();

  WCNSLinearFVConservativeSharpInterfaceFlowPhysics(const InputParameters & parameters);

private:
  void addFunctorMaterials() override;
  void addRhieChowUserObjects() override;

  MooseFunctorName generatedGeometryFunctorName(const std::string & base_name) const;
  void setVelocityParams(InputParameters & params) const;
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
  bool shouldCreateGeometryFunctorMaterial() const;

  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

  const bool _create_geometry_functors;
};
