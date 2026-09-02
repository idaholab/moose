//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVGradientMethod.h"
#include "MeshChangedInterface.h"

#include "libmesh/numeric_vector.h"

#include <unordered_set>

class ElemInfo;
class FaceInfo;
class LinearFVGradientReader;
class RhieChowMassFlux;

/**
 * Reconstructs the pressure gradient used for Rhie-Chow momentum coupling from corrected face
 * fluxes and publishes a relaxed feedback field to linear finite-volume consumers.
 */
class FVReconstructedPressureGradient : public FVGradientMethod, public MeshChangedInterface
{
public:
  using GradientContainer = FVGradientMethod::GradientContainer;

  static InputParameters validParams();

  FVReconstructedPressureGradient(const InputParameters & params);

  /// Bind this stateful method to one Rhie-Chow flow-system configuration.
  void bindFlowSystem(const RhieChowMassFlux & rc,
                      const LinearFVGradientReader & pressure_gradient) const;

  /// Name of the gradient method used before reconstructed feedback is available.
  const GradientMethodName & baseGradientMethodName() const { return _base_gradient_method_name; }

  /// Reset solver-iteration state once per attempted time step.
  void resetForTimeStep(const RhieChowMassFlux & rc) const;

  /// Capture the lagged cell velocity gradient used by the reconstruction.
  void captureLaggedVelocityGradient(const RhieChowMassFlux & rc) const;

  /// Reconstruct the conservative pressure-gradient candidate from the corrected face flux.
  void computeCandidateFromCorrectedFlux(const RhieChowMassFlux & rc) const;

  /// Get the conservative candidate produced by the current pressure corrector.
  const GradientContainer & reconstructedCandidate(const RhieChowMassFlux & rc) const;

  /// Relax the current candidate into the feedback field published to gradient consumers.
  void publishRelaxedFeedback(const RhieChowMassFlux & rc,
                              const GradientContainer & base_gradient) const;

  virtual void meshChanged() override;

private:
  void computeGradientWithoutLimiter(
      SystemBase & system,
      GradientContainer & gradient,
      const std::unordered_set<unsigned int> & variable_numbers) const override;

  /// Resolve the method used before reconstructed feedback is available.
  const FVGradientMethod & resolveBaseGradientMethod(SystemBase & system) const;

  /// Check that a stateful operation is requested by the bound Rhie-Chow object.
  void checkFlowSystem(const RhieChowMassFlux & rc) const;

  /// Blend a reconstructed candidate into the persistent feedback field.
  void updateFeedbackGradient(const GradientContainer & base_gradient,
                              const GradientContainer & reconstructed_candidate) const;

  /// Interpolate a lagged velocity-component gradient to a face.
  RealVectorValue reconstructionVelocityGradient(const RhieChowMassFlux & rc,
                                                 const ElemInfo & elem_info,
                                                 const FaceInfo & fi,
                                                 bool elem_has_info,
                                                 unsigned int velocity_component) const;

  /// Build the set of Rhie-Chow cells touching boundary faces.
  void buildBoundaryCellCache(const RhieChowMassFlux & rc) const;

  /// Gradient method used before reconstructed feedback is available.
  const GradientMethodName _base_gradient_method_name;

  /// Which pressure gradient is retained on cells touching a boundary face.
  const MooseEnum _reconstructed_pressure_gradient_boundary_cells;

  /// Relaxation factor applied to reconstructed pressure gradients.
  const Real _gradient_relaxation;

  /// Rhie-Chow object that owns this stateful reconstruction method.
  mutable const RhieChowMassFlux * _rhie_chow = nullptr;

  /// Pressure system that owns the reconstructed pressure variable.
  mutable const SystemBase * _pressure_system = nullptr;

  /// Pressure variable reconstructed by this method.
  mutable unsigned int _pressure_variable_number = libMesh::invalid_uint;

  /// Momentum systems coupled through the owning Rhie-Chow object.
  mutable std::vector<const SystemBase *> _momentum_systems;

  /// Cached base gradient method.
  mutable const FVGradientMethod * _base_gradient_method = nullptr;

  /// Whether the boundary-cell cache has been built since the last mesh change.
  mutable bool _boundary_cell_cache_built = false;

  /// Rhie-Chow element ids touching a boundary face.
  mutable std::unordered_set<dof_id_type> _boundary_cell_ids;

  /// Lagged velocity gradients indexed by velocity component and spatial direction.
  mutable std::vector<std::vector<std::unique_ptr<NumericVector<Number>>>>
      _lagged_reconstruction_velocity_gradient;

  /// Whether a lagged velocity-gradient snapshot is available.
  mutable bool _lagged_velocity_gradient_available = false;

  /// Producer generation for lagged velocity-gradient snapshots.
  mutable dof_id_type _lagged_velocity_gradient_generation = 0;

  /// Lagged velocity-gradient generation consumed by the current candidate.
  mutable dof_id_type _reconstructed_candidate_generation = 0;

  /// Face-flux generation consumed by the current candidate.
  mutable dof_id_type _reconstructed_candidate_face_flux_generation = 0;

  /// Candidate generation consumed by the published relaxed feedback.
  mutable dof_id_type _published_candidate_generation = 0;

  /// Reconstructed pressure-gradient candidate.
  mutable GradientContainer _reconstructed_pressure_gradient;

  /// Persistent relaxed feedback field.
  mutable GradientContainer _feedback;

  /// Whether the feedback field has been initialized.
  mutable bool _feedback_initialized = false;
};
