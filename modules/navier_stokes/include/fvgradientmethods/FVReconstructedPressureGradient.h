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
#include "LinearFVGradientReader.h"
#include "MeshChangedInterface.h"

#include "libmesh/numeric_vector.h"

#include <unordered_set>

class ElemInfo;
class FaceInfo;
class RhieChowMassFlux;

/**
 * Reconstructs the pressure gradient used for Rhie-Chow momentum coupling from corrected face
 * fluxes and publishes a relaxed coupling pressure-gradient field to linear finite-volume
 * consumers.
 */
class FVReconstructedPressureGradient : public FVGradientMethod, public MeshChangedInterface
{
public:
  using GradientContainer = FVGradientMethod::GradientContainer;
  using GradientView = LinearFVGradientReader::GradientContainer;

  static InputParameters validParams();

  FVReconstructedPressureGradient(const InputParameters & params);

  /// Link this stateful method to one Rhie-Chow flow-system configuration.
  void linkFlowSystem(RhieChowMassFlux & rc,
                      const LinearFVGradientReader & pressure_gradient) const;

  /// Validate the one-time Rhie-Chow linkage and field layouts required by reconstruction.
  void validateSetup(const RhieChowMassFlux & rc) const;

  /// Name of the gradient method used before the reconstructed coupling pressure gradient exists.
  const GradientMethodName & baseGradientMethodName() const { return _base_gradient_method_name; }

  /// Prepare solver-iteration state for a new time-step attempt.
  void resetForTimeStep(const RhieChowMassFlux & rc) const;

  /// Save the lagged cell velocity gradient used by the reconstruction.
  void saveLaggedVelocityGradient(RhieChowMassFlux & rc) const;

  /// Reconstruct the conservative pressure-gradient candidate from the corrected face flux.
  void computeCandidateFromCorrectedFlux(const RhieChowMassFlux & rc) const;

  /// Get the conservative candidate produced by the current pressure corrector.
  const GradientContainer & reconstructedCandidate(const RhieChowMassFlux & rc) const;

  /// Relax and publish the current candidate as the coupling pressure gradient.
  void publishCouplingPressureGradient(const RhieChowMassFlux & rc,
                                       const GradientView & base_gradient) const;

  virtual void meshChanged() override;

private:
  void computeGradientWithoutLimiter(
      SystemBase & system,
      GradientContainer & gradient,
      const std::unordered_set<unsigned int> & variable_numbers) const override;

  /// Resolve the method used before the reconstructed coupling pressure gradient exists.
  const FVGradientMethod & resolveBaseGradientMethod(SystemBase & system) const;

  /// Check that a stateful operation is requested by the bound Rhie-Chow object.
  void checkFlowSystem(const RhieChowMassFlux & rc) const;

  /// Copy gradient values while reusing compatible destination storage.
  void copyGradient(const GradientView & source, GradientContainer & destination) const;

  /// Reset state that is local to one time-step attempt.
  void resetAttemptState() const;

  /// Blend a reconstructed candidate into the persistent coupling pressure gradient.
  void updateCouplingPressureGradient(const GradientView & base_gradient,
                                      const GradientContainer & reconstructed_candidate) const;

  /// Interpolate a lagged velocity-component gradient to a face.
  RealVectorValue reconstructionVelocityGradient(const RhieChowMassFlux & rc,
                                                 const ElemInfo & elem_info,
                                                 const FaceInfo & fi,
                                                 bool elem_has_info,
                                                 unsigned int velocity_component) const;

  /// Gradient method used before the reconstructed coupling pressure gradient exists.
  const GradientMethodName _base_gradient_method_name;

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

  /// Ordinary gradient fields for the velocity components.
  mutable std::vector<const LinearFVGradientReader *> _velocity_gradient_fields;

  /// Cached base gradient method.
  mutable const FVGradientMethod * _base_gradient_method = nullptr;

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

  /// Candidate generation consumed by the published coupling pressure gradient.
  mutable dof_id_type _published_candidate_generation = 0;

  /// Reconstructed pressure-gradient candidate.
  mutable GradientContainer _reconstructed_pressure_gradient;

  /// Persistent coupling pressure gradient.
  mutable GradientContainer _coupling_pressure_gradient;

  /// Whether the coupling pressure gradient has been initialized.
  mutable bool _coupling_pressure_gradient_initialized = false;
};
