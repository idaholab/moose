#pragma once

#include "WCNSFVFlowPhysicsBase.h"
#include "WCNSFVTurbulencePhysics.h"

/**
 * Linear-FV segregated sharp-interface flow physics for the reference-parity U path.
 *
 * This class is intentionally structured as a sibling of WCNSLinearFVFlowPhysics,
 * because the stock linear-FV mixture path does not provide the extra face-flux
 * predictors needed for large-density-ratio sharp-interface coupling.
 */
class WCNSLinearFVConservativeSharpInterfaceFlowPhysics final : public WCNSFVFlowPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSLinearFVConservativeSharpInterfaceFlowPhysics(const InputParameters & parameters);

protected:
  void initializePhysicsAdditional() override;

private:
  void addSolverVariables() override;
  void addFVKernels() override;
  void addUserObjects() override;
  void addFunctorMaterials() override;
  void addRhieChowUserObjects() override;

  void addPressureCorrectionKernels();
  void addCurvatureUserObject();
  void addDynamicContactAngleFunctorMaterial();
  void addMomentumTimeKernels() override;
  void addMomentumFluxKernels();
  void addMomentumConditioningKernels();
  void addMomentumPressureKernels() override;
  void addMomentumGravityKernels() override;
  void addMomentumReducedPressureKernels();
  void addMomentumFaceBasedReducedPressureKernels();
  void addMomentumFrictionKernels() override;
  void addMomentumBoussinesqKernels() override;

  void addInletBC() override;
  void addOutletBC() override;
  void addWallsBC() override;
  void addSeparatorBC() override {}

  bool hasForchheimerFriction() const override { return false; }

  MooseFunctorName generatedGeometryFunctorName(const std::string & base_name) const;
  MooseFunctorName momentumTransportMassFluxFunctorName() const;
  MooseFunctorName generatedBoundaryMomentumFunctorName(const BoundaryName & boundary,
                                                        unsigned int component,
                                                        const std::string & family) const;
  bool useMomentumContinuityErrorSink() const;
  bool shouldCreateGeometryFunctorMaterial() const;
  bool shouldCreateCurvatureProducer() const;
  bool shouldCreateDynamicContactAngleFunctorMaterial() const;
  void addVelocityBoundaryInputFunctorMaterials();

  MooseFunctorName getLinearFrictionCoefName() const override
  {
    return "";
  }

  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

  const bool _non_orthogonal_correction;
  const MooseEnum _pressure_formulation;
  const bool _add_transient_projection_flux;
  const bool _add_capillary_hydrostatic_flux;
  const bool _create_geometry_functors;
  const bool _create_curvature_producer;
  const bool _create_dynamic_contact_angle_functor_material;
};
