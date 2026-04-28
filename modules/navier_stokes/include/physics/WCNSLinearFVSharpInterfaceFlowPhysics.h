#pragma once

#include "WCNSFVFlowPhysicsBase.h"
#include "WCNSFVTurbulencePhysics.h"

/**
 * Linear-FV segregated flow physics for sharp-interface / reduced-pressure work.
 *
 * This class is intentionally structured as a sibling of WCNSLinearFVFlowPhysics,
 * because the stock linear-FV mixture path does not provide the extra face-flux
 * predictors needed for large-density-ratio sharp-interface coupling.
 */
class WCNSLinearFVSharpInterfaceFlowPhysics final : public WCNSFVFlowPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSLinearFVSharpInterfaceFlowPhysics(const InputParameters & parameters);

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
  void addMomentumPressureKernels() override;
  void addMomentumGravityKernels() override;
  void addMomentumReducedPressureKernels();
  void addMomentumFrictionKernels() override;
  void addMomentumBoussinesqKernels() override;

  void addInletBC() override;
  void addOutletBC() override;
  void addWallsBC() override;
  void addSeparatorBC() override {}

  bool hasForchheimerFriction() const override { return false; }

  MooseFunctorName generatedGeometryFunctorName(const std::string & base_name) const;
  bool shouldCreateGeometryFunctorMaterial() const;
  bool shouldCreateCurvatureProducer() const;
  bool shouldCreateDynamicContactAngleFunctorMaterial() const;

  MooseFunctorName getLinearFrictionCoefName() const override
  {
    return "";
  }

  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

  const bool _non_orthogonal_correction;
  const MooseEnum _pressure_formulation;
  const bool _split_momentum_predictor_operator;
  const bool _add_transient_projection_flux;
  const bool _add_capillary_hydrostatic_flux;
  const bool _use_face_based_predictor_body_force;
  const bool _create_geometry_functors;
  const bool _create_curvature_producer;
  const bool _create_dynamic_contact_angle_functor_material;
};
