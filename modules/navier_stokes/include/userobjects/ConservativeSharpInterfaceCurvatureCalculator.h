#pragma once

#include "CellCenteredMapFunctor.h"
#include "FaceCenteredMapFunctor.h"
#include "GeneralUserObject.h"
#include "NonADFunctorInterface.h"
#include "BlockRestrictable.h"
#include "MooseTypes.h"

#include <unordered_map>
#include <vector>

/**
 * reference-solver-like curvature producer for a sharp-interface linear-FV path.
 *
 * Implemented path:
 *
 *   alpha_smooth <- repeated area-weighted averaging sweeps (optional)
 *   gradAlpha    <- Gauss gradient from the cell-centered alpha field
 *   gradAlphaf   <- interpolation of gradAlpha to faces
 *   nHatf*       <- gradAlphaf / (|gradAlphaf| + deltaN)
 *   nHatf        <- boundary-wall contact-angle correction (optional)
 *   K            <- -div(nHatf)
 *
 * The provisional face unit normal is exported separately so a dynamic wall
 * contact-angle functor can evaluate theta(U, nHat) using the same smoothed
 * face geometry that the curvature calculation uses before boundary correction.
 */
class MooseMesh;

class ConservativeSharpInterfaceCurvatureCalculator : public GeneralUserObject,
                                          public NonADFunctorInterface,
                                          public BlockRestrictable
{
public:
  static InputParameters validParams();

  ConservativeSharpInterfaceCurvatureCalculator(const InputParameters & params);

  void initialSetup() override;
  void meshChanged() override;
  void initialize() override;
  void execute() override;
  void finalize() override;

  /// Explicit refresh hook for custom segregated executioners.
  void updateCurvatureMaps(const bool verbose = false);

protected:
  void rebuildSharpInterfaceFaceInfo();
  void updateEffectiveDeltaN();
  void parseStaticContactAngles();

  Moose::FaceArg makeCenteredFaceArg(const FaceInfo * fi,
                                     const Moose::StateArg * limiter_state = nullptr) const;

  bool elemInBlocks(const Elem * elem) const;
  bool faceTouchesBlocks(const FaceInfo * fi) const;
  Real faceMeasure(const FaceInfo * fi) const;
  Real elemMeasure(const FaceInfo * fi, const bool neighbor) const;

  void buildCellAlphaField(const Moose::StateArg & time_arg,
                           std::unordered_map<dof_id_type, Real> & cell_alpha) const;
  void smoothCellAlphaField(std::unordered_map<dof_id_type, Real> & cell_alpha) const;
  void computeCellGradientFromCellField(
      const std::unordered_map<dof_id_type, Real> & cell_field,
      std::unordered_map<dof_id_type, RealVectorValue> & cell_gradient) const;

  Real interpolateCellScalarToFace(const FaceInfo * fi,
                                   const std::unordered_map<dof_id_type, Real> & cell_field) const;
  RealVectorValue interpolateCellVectorToFace(
      const FaceInfo * fi,
      const std::unordered_map<dof_id_type, RealVectorValue> & cell_field) const;

  Real contactAngleRadiansForFace(const FaceInfo * fi,
                                  const Moose::StateArg & time_arg,
                                  const Moose::StateArg * limiter_state) const;
  RealVectorValue correctBoundaryContactAngle(
      const FaceInfo * fi,
      const RealVectorValue & provisional_n_hat_face,
      const RealVectorValue & face_gradient,
      const Moose::StateArg & time_arg,
      const Moose::StateArg * limiter_state,
      RealVectorValue * corrected_face_gradient) const;

  Real effectiveDeltaN() const { return _effective_delta_n; }

  const MooseMesh & _moose_mesh;

  FaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
      _face_smoothed_alpha_gradient;
  FaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
      _provisional_face_unit_normal;
  FaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
      _face_unit_normal;
  CellCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _curvature;

  std::vector<const FaceInfo *> _sharp_interface_face_info;
  std::unordered_map<BoundaryID, Real> _static_contact_angle_radians;

  const Moose::Functor<Real> & _volume_fraction;
  const MooseFunctorName _wall_contact_angle_degrees_functor_name;

  const MooseEnum _delta_n_mode;
  const Real _delta_n_scale;
  const Real _delta_n_fixed_value;
  const bool _use_reference_simple_curvature;
  const unsigned int _n_alpha_smooth_curvature;
  const Real _contact_angle_small_det;

  const MooseFunctorName _face_smoothed_alpha_gradient_name;
  const MooseFunctorName _provisional_face_unit_normal_name;
  const MooseFunctorName _face_unit_normal_name;
  const MooseFunctorName _curvature_name;

  Real _effective_delta_n;
};
