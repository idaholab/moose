#pragma once

#include "PIMPLESolve.h"
#include "RhieChowMassFlux.h"

#include "libmesh/point.h"

#include <string>
#include <unordered_set>
#include <vector>

class ConservativeSharpInterfaceRhieChowMassFlux;
class ConservativeSharpInterfaceCurvatureCalculator;
class ConservativeSharpInterfaceVOFMULESCorrector;

/**
 * PIMPLE solve object with explicit hooks for additional reduced-pressure /
 * sharp-interface face-flux predictors.
 */
class ReducedPressurePIMPLESolve : public PIMPLESolve
{
public:
  static InputParameters validParams();

  ReducedPressurePIMPLESolve(Executioner & ex);

  bool solve() override;
  Real momentumPressureCourant(const Real dt) const;
  RhieChowMassFlux::MaxCourantAudit momentumPressureCourantAudit(const Real dt) const;
  std::string momentumPressureWorstFaceSharpDiagnostics(
      const RhieChowMassFlux::MaxCourantAudit & audit) const;
  Real constrainedMomentumPressureDT(const Real dt) const;
  void commitAcceptedTimestepTransportHistory() const;
  bool adjustMomentumPressureTimeStepEnabled() const
  {
    return _adjust_momentum_pressure_time_step;
  }
  Real momentumPressureMaxCourant() const { return _momentum_pressure_max_courant; }

protected:
  std::pair<unsigned int, Real>
  correctVelocity(const bool subtract_updated_pressure,
                  const bool recompute_face_mass_flux,
                  const SolverParams & solver_params) override;
  void addMomentumPredictorExplicitForcing(const unsigned int system_i,
                                           NumericVector<Number> & rhs) override;
  void addMomentumPredictorBodyForceForcing(const unsigned int system_i,
                                            NumericVector<Number> & rhs) override;
  std::vector<std::pair<unsigned int, Real>> solveMomentumPredictor() override;
  bool auditMomentumPredictorRebuild() const override;

private:
  using NonlinearSolutionStateSnapshots =
      std::vector<std::vector<std::unique_ptr<NumericVector<Number>>>>;

  bool startupPressureInitializationEnabled() const;
  bool useEquilibriumStartupPressureInitialization() const;
  void assembleMomentumPredictorOnly();
  void initializeStartupPressureField(const SolverParams & solver_params);
  void performStartupContinuityCorrections(const SolverParams & solver_params);
  void preparePressureCorrectorState(const bool subtract_updated_pressure);
  void reconstructPressureCoupledStateFromCurrentPressure(const bool subtract_updated_pressure);
  void dumpPressureOuterDebugState(const std::string & stage_label);
  void dumpPressureDebugFaces(const std::string & stage_label);
  void resolvePressureDebugFaceIds();
  void advanceSystemOuterIterationHistory(const std::vector<LinearSystem *> & systems) const;
  void advanceMomentumOuterIterationHistory() const;
  void advanceVolumeFractionOuterIterationHistory() const;
  unsigned int computeVolumeFractionSubcycles() const;
  NonlinearSolutionStateSnapshots snapshotMomentumNonlinearSolutionStates() const;
  void restoreMomentumNonlinearSolutionStates(
      const NonlinearSolutionStateSnapshots & snapshots) const;
  void synchronizeSystemState(LinearSystem & system) const;
  std::vector<std::pair<unsigned int, Real>> solveVolumeFractionSystems(
      const SolverParams & solver_params);
  void clampVolumeFractionSystems();
  ConservativeSharpInterfaceVOFMULESCorrector * sharpInterfaceVOFCorrector(const SolverSystemName & system_name) const;

  std::pair<unsigned int, Real>
  correctVelocityOnce(const bool subtract_updated_pressure,
                      const bool recompute_face_mass_flux,
                      const SolverParams & solver_params);
  std::pair<unsigned int, Real>
  applyPressureCorrectionStage(const bool recompute_face_mass_flux,
                               const bool publish_pressure_corrected_state,
                               const SolverParams & solver_params);
  void publishPressureCorrectedTransportState(const std::string & stage_label);
  void applyReferencePressureCorrectorTail(const std::string & stage_label);
  void reportReferenceContinuityErrors(const std::string & stage_label);
  void correctMovingMeshFaceVelocityAndMakeRelative();
  void updateAbsolutePressureAndReferenceState();
  void relaxPressureFieldForNextPredictor();
  std::pair<unsigned int, Real>
  correctStartupContinuityOnce(const bool subtract_updated_pressure,
                               const bool recompute_face_mass_flux,
                               const SolverParams & solver_params);

  ConservativeSharpInterfaceRhieChowMassFlux * sharpInterfaceRC() const;
  ConservativeSharpInterfaceCurvatureCalculator * sharpInterfaceCurvature() const;

  const std::vector<SolverSystemName> _volume_fraction_system_names;
  const bool _has_volume_fraction_systems;
  const bool _should_solve_volume_fractions;
  std::vector<unsigned int> _volume_fraction_system_numbers;
  std::vector<LinearSystem *> _volume_fraction_systems;

  const std::vector<Real> _volume_fraction_equation_relaxation;
  Moose::PetscSupport::PetscOptions _volume_fraction_petsc_options;
  SIMPLESolverConfiguration _volume_fraction_linear_control;
  const Real _volume_fraction_l_abs_tol;
  const std::vector<Real> _volume_fraction_absolute_tolerance;
  const Real _volume_fraction_min_value;
  const Real _volume_fraction_max_value;
  const unsigned int _volume_fraction_subcycles;
  const Real _volume_fraction_max_courant;
  const bool _adjust_momentum_pressure_time_step;
  const Real _momentum_pressure_max_courant;
  const bool _volume_fraction_outer_corrections;
  const bool _dump_pressure_outer_debug_csv;
  const unsigned int _dump_pressure_outer_debug_start_timestep;
  const unsigned int _dump_pressure_outer_debug_end_timestep;
  const unsigned int _dump_pressure_outer_debug_max_outer_iterations;
  std::unordered_set<dof_id_type> _pressure_debug_face_ids;
  const std::vector<Point> _pressure_debug_face_points;
  bool _pressure_debug_face_ids_resolved = false;
  const bool _audit_momentum_predictor_rebuild;
  const unsigned int _audit_momentum_predictor_rebuild_start_timestep;
  const unsigned int _audit_momentum_predictor_rebuild_end_timestep;
  unsigned int _current_piso_iteration = 0;
  std::string _startup_pressure_initialization;
  const bool _suppress_explicit_hydrostatic_flux_during_seeded_startup;
  const unsigned int _startup_flux_corrections;
  const unsigned int _num_pressure_nonorthogonal_correctors;
  Real _cumulative_continuity_error = 0.0;
};
