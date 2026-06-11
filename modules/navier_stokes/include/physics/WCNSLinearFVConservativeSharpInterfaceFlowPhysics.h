//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSLinearFVFlowPhysicsBase.h"
#include "NS.h"

/**
 * Linear-FV segregated sharp-interface flow physics for velocity-component momentum unknowns.
 *
 * This class reuses the common segregated Linear FV flow setup and overrides the sharp-interface
 * pieces needed for large-density-ratio conservative coupling.
 */
class WCNSLinearFVConservativeSharpInterfaceFlowPhysics final : public WCNSLinearFVFlowPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSLinearFVConservativeSharpInterfaceFlowPhysics(const InputParameters & parameters);

private:
  void addFunctorMaterials() override;
  void addRhieChowUserObjects() override;

  void addMomentumReducedPressureKernels() override;
  void addOutletBC() override;
  void addWallsBC() override;

  MooseFunctorName generatedGeometryFunctorName(const std::string & base_name) const;
  MooseFunctorName generatedBoundaryMomentumFunctorName(const BoundaryName & boundary,
                                                        unsigned int component,
                                                        const std::string & family) const;
  bool shouldAddMomentumPressureKernels() const override;
  bool shouldAddMomentumReducedPressureKernels() const override;
  MooseFunctorName pressureDiffusionTensorName() const override { return "pressure_Ainv"; }
  MooseFunctorName pressureDivergenceFluxName() const override { return "pressure_predictor_flux"; }
  bool pressureDivergenceFluxIsIntegrated() const override { return true; }
  std::string momentumTimeKernelType() const override
  {
    return "LinearWCNSFVMomentumTimeDerivative";
  }
  std::string momentumTimeDensityParameterName() const override { return NS::density; }
  std::string momentumFluxKernelType() const override
  {
    return "LinearWCNSFVConservativeMomentumFlux";
  }
  MooseFunctorName momentumFluxMassFluxFunctorName() const override
  {
    return "rho_phi_mass_flux_density";
  }
  MooseFunctorName inletVelocityFunctorName(const BoundaryName & boundary,
                                            unsigned int component) const override;
  MooseFunctorName wallVelocityFunctorName(const BoundaryName & boundary,
                                           unsigned int component) const override;
  bool shouldCreateGeometryFunctorMaterial() const;
  void addVelocityBoundaryInputFunctorMaterials();

  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

  const MooseEnum _pressure_formulation;
  const bool _add_transient_projection_flux;
  const bool _add_capillary_hydrostatic_flux;
  const bool _create_geometry_functors;
};
