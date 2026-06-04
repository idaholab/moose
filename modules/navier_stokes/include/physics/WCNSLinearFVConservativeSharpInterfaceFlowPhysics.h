#pragma once

#include "WCNSLinearFVFlowPhysicsBase.h"
#include "NS.h"

/**
 * Linear-FV segregated sharp-interface flow physics for the reference-parity U path.
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

  void addAdditionalUserObjects() override;
  void addCurvatureUserObject();
  void addDynamicContactAngleFunctorMaterial();
  void addMomentumConditioningKernels() override;
  void addMomentumReducedPressureKernels() override;
  void addOutletBC() override;
  void addWallsBC() override;

  MooseFunctorName generatedGeometryFunctorName(const std::string & base_name) const;
  MooseFunctorName generatedBoundaryMomentumFunctorName(const BoundaryName & boundary,
                                                        unsigned int component,
                                                        const std::string & family) const;
  bool useMomentumContinuityErrorSink() const override;
  bool shouldAddMomentumPressureKernels() const override;
  bool shouldAddMomentumReducedPressureKernels() const override;
  MooseFunctorName pressureDiffusionTensorName() const override { return "pressure_Ainv"; }
  MooseFunctorName pressureDivergenceFluxName() const override { return "pressure_predictor_flux"; }
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
  bool shouldCreateCurvatureProducer() const;
  bool shouldCreateDynamicContactAngleFunctorMaterial() const;
  void addVelocityBoundaryInputFunctorMaterials();

  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

  const MooseEnum _pressure_formulation;
  const bool _add_transient_projection_flux;
  const bool _add_capillary_hydrostatic_flux;
  const bool _create_geometry_functors;
  const bool _create_curvature_producer;
  const bool _create_dynamic_contact_angle_functor_material;
};
