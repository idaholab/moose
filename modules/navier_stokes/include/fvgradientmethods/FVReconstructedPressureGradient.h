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
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"

#include <unordered_set>

class ElemInfo;
class FaceInfo;
class FEProblemBase;
class RhieChowMassFlux;

/**
 * Gradient method that reconstructs and relaxes the pressure gradient used for Rhie-Chow
 * momentum-pressure coupling on the linear finite-volume segregated solver. Responsibilities:
 * - Binds to exactly one RhieChowMassFlux flow-system configuration, together with the momentum
 *   systems and velocity-gradient fields needed to reconstruct that system's pressure gradient.
 * - Falls back to an ordinary base gradient method (base_gradient_method) before any
 *   flux-consistent gradient has been reconstructed, e.g. before the first pressure corrector.
 * - Drives, once per pressure corrector, the reconstruction cycle that freezes a lagged
 *   cell-velocity gradient, projects the corrected face fluxes back to a cell velocity in a
 *   least-squares sense, and inverts the discrete momentum balance for the pressure-gradient
 *   component consistent with that velocity.
 * - Relaxes (gradient_relaxation) and publishes that reconstructed candidate as the coupling
 *   pressure gradient supplied to the next momentum predictor.
 * - Preserves the last accepted coupling gradient across time-step retries, restart, and
 *   recovery, and clears all cached reconstruction state when the mesh changes.
 */
class FVReconstructedPressureGradient : public FVGradientMethod, public MeshChangedInterface
{
public:
  using GradientContainer = FVGradientMethod::GradientContainer;
  using GradientView = LinearFVGradientReader::GradientContainer;

  static InputParameters validParams();

  FVReconstructedPressureGradient(const InputParameters & params);

  /// Link this stateful method to one Rhie-Chow flow-system configuration.
  void linkFlowSystem(RhieChowMassFlux & rc, const LinearFVGradientReader & pressure_gradient);

  /// Validate the one-time Rhie-Chow linkage and field layouts required by reconstruction.
  void validateSetup(const RhieChowMassFlux & rc) const;

  /// Name of the gradient method used before the reconstructed coupling pressure gradient exists.
  const GradientMethodName & baseGradientMethodName() const { return _base_gradient_method_name; }

  /// Prepare solver-iteration state for a new time-step attempt.
  void resetForTimeStep(const RhieChowMassFlux & rc);

  /// Save the lagged cell velocity gradient used by the reconstruction.
  void saveLaggedVelocityGradient(RhieChowMassFlux & rc);

  /// Reconstruct the conservative pressure-gradient candidate from the corrected face flux.
  void computeCandidateFromCorrectedFlux(const RhieChowMassFlux & rc);

  /// Get the conservative candidate produced by the current pressure corrector.
  const GradientContainer & reconstructedCandidate(const RhieChowMassFlux & rc) const;

  /// Whether at least one corrected-flux pressure-gradient candidate has been reconstructed.
  bool hasReconstructedCandidate() const { return !_reconstructed_pressure_gradient.empty(); }

  /**
   * Relax and publish the current candidate as the coupling pressure gradient.
   * @param rc The bound Rhie-Chow flow-system configuration whose pressure gradient is being
   * finalized
   * @param base_gradient The ordinary (unreconstructed) pressure gradient used to initialize the
   * coupling gradient the first time it is published
   */
  void finalizeCouplingPressureGradient(const RhieChowMassFlux & rc,
                                        const GradientView & base_gradient);

  virtual void meshChanged() override;

private:
  /// Current stage of the pressure-gradient reconstruction cycle.
  enum class PressureGradientReconstructionState
  {
    /// The next reconstruction cycle requires a lagged velocity-gradient snapshot.
    NeedLaggedVelocityGradient,
    /// The lagged velocity gradient is available and a reconstructed candidate is required.
    NeedCandidate,
    /// The reconstructed candidate is available and ready to be published.
    CandidateReady
  };

  /// Operations that advance or reset the pressure-gradient reconstruction cycle.
  enum class PressureGradientReconstructionEvent
  {
    /// Reset the cycle to require a new lagged velocity-gradient snapshot.
    Reset,
    /// Record that the lagged velocity-gradient snapshot has been saved.
    SaveLaggedVelocityGradient,
    /// Record that a pressure-gradient candidate has been reconstructed.
    ReconstructCandidate,
    /// Record that the reconstructed candidate has been published.
    PublishCandidate
  };

  void computeGradientWithoutLimiter(
      SystemBase & system,
      GradientContainer & gradient,
      const std::unordered_set<unsigned int> & variable_numbers) const override;

  void resolveGradientMethodDependencies(FEProblemBase & fe_problem) override;

  /// Check that a stateful operation is requested by the bound Rhie-Chow object.
  void checkFlowSystem(const RhieChowMassFlux & rc) const;

  /// Copy gradient values while reusing compatible destination storage.
  void copyGradient(const GradientView & source, GradientContainer & destination);

  /// Apply and validate one reconstruction-cycle transition.
  void transition(PressureGradientReconstructionEvent event);

  /// Blend a reconstructed candidate into the persistent coupling pressure gradient.
  void updateCouplingPressureGradient(const RhieChowMassFlux & rc,
                                      const GradientView & base_gradient,
                                      const GradientContainer & reconstructed_candidate);

  /// Interpolate a lagged velocity-component gradient to a face.
  RealVectorValue reconstructionVelocityGradient(const RhieChowMassFlux & rc,
                                                 const ElemInfo & elem_info,
                                                 const FaceInfo & fi,
                                                 bool elem_has_info,
                                                 unsigned int velocity_component) const;

  /**
   * Add one face's contribution to the area-weighted least-squares system that reconstructs the
   * cell velocity of elem_info from corrected face fluxes. The pressure-corrected face-normal
   * volumetric flux at fi, with the lagged velocity-gradient Taylor correction from the cell
   * center to the face removed, gives one linear equation for the unknown cell velocity,
   * u_P . n_f ~= qhat_f. This face equation contributes |S_f| n_f n_f^T to matrix and
   * |S_f| qhat_f n_f to projection_rhs; once every face of elem_info has been added, solving
   * that system gives the cell velocity that is later substituted into the discrete momentum
   * balance to recover the pressure-gradient component consistent with these corrected fluxes.
   * @param rc The bound Rhie-Chow flow-system configuration supplying the corrected face flux
   * and lagged velocity gradients
   * @param elem_info The cell whose velocity-projection system is being assembled
   * @param fi The face being added to the projection
   * @param surface_vector The coordinate-system-aware area vector of this face, outward from
   * elem_info
   * @param elem_has_info Whether elem_info is the "elem" side of fi, as opposed to its
   * "neighbor" side
   * @param matrix Area-weighted normal-equation matrix accumulated over the cell's faces,
   * updated in place with this face's contribution
   * @param projection_rhs Area-weighted right-hand side accumulated over the cell's faces,
   * updated in place with this face's contribution
   */
  void assembleFaceProjection(const RhieChowMassFlux & rc,
                              const ElemInfo & elem_info,
                              const FaceInfo * fi,
                              const Point & surface_vector,
                              bool elem_has_info,
                              DenseMatrix<Real> & matrix,
                              DenseVector<Real> & projection_rhs) const;

  /// Solve a cell's velocity-projection system.
  DenseVector<Real> solveFaceProjection(const DenseMatrix<Real> & matrix,
                                        const DenseVector<Real> & projection_rhs) const;

  /// Invert one diagonal momentum equation to recover a pressure-gradient component.
  Real reconstructPressureGradient(const RhieChowMassFlux & rc,
                                   const ElemInfo & elem_info,
                                   unsigned int component,
                                   Real reconstructed_velocity) const;

  /// Gradient method used before the reconstructed coupling pressure gradient exists.
  const GradientMethodName _base_gradient_method_name;

  /// Resolved method used before the reconstructed coupling pressure gradient exists.
  const FVGradientMethod * _base_gradient_method = nullptr;

  /// Relaxation factor applied to reconstructed pressure gradients.
  const Real _gradient_relaxation;

  /// Whether face equations include the lagged velocity-gradient Taylor correction.
  const bool _use_velocity_gradient_taylor_correction;

  /// Rhie-Chow object that owns this stateful reconstruction method.
  const RhieChowMassFlux * _rhie_chow = nullptr;

  /// Pressure system that owns the reconstructed pressure variable.
  const SystemBase * _pressure_system = nullptr;

  /// Pressure variable reconstructed by this method.
  unsigned int _pressure_variable_number = libMesh::invalid_uint;

  /// Momentum systems coupled through the owning Rhie-Chow object.
  std::vector<const SystemBase *> _momentum_systems;

  /// Ordinary gradient fields for the velocity components.
  std::vector<const LinearFVGradientReader *> _velocity_gradient_fields;

  /// Lagged velocity gradients indexed by velocity component and spatial direction.
  std::vector<std::vector<std::unique_ptr<NumericVector<Number>>>>
      _lagged_reconstruction_velocity_gradient;

  /// Current position in the pressure-gradient reconstruction cycle.
  PressureGradientReconstructionState _reconstruction_state =
      PressureGradientReconstructionState::NeedLaggedVelocityGradient;

  /// Corrected face-flux iteration used by the previous reconstruction.
  dof_id_type _last_reconstructed_face_flux_iteration = 0;

  /// Reconstructed pressure-gradient candidate.
  GradientContainer _reconstructed_pressure_gradient;

  /// Persistent coupling pressure gradient.
  GradientContainer _coupling_pressure_gradient;

  /// Whether the coupling pressure gradient has been initialized.
  bool _coupling_pressure_gradient_initialized = false;
};
