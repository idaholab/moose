#pragma once

#include "PIMPLESolve.h"

#include <string>

class SharpInterfaceRhieChowMassFlux;
class SharpInterfaceCurvatureCalculator;
class SharpInterfaceVOFMULESCorrector;

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

protected:
  std::pair<unsigned int, Real>
  correctVelocity(const bool subtract_updated_pressure,
                  const bool recompute_face_mass_flux,
                  const SolverParams & solver_params) override;
  void addMomentumPredictorExplicitForcing(const unsigned int system_i,
                                           NumericVector<Number> & rhs) override;
  std::vector<std::pair<unsigned int, Real>> solveMomentumPredictor() override;
  bool auditMomentumPredictorRebuild() const override;

private:
  using NonlinearSolutionStateSnapshots =
      std::vector<std::vector<std::unique_ptr<NumericVector<Number>>>>;

  bool startupPressureInitializationEnabled() const;
  bool useEquilibriumStartupPressureInitialization() const;
  bool startupPressureCutAuditEnabled() const;
  bool momentumProbeAuditEnabled() const;
  void assembleMomentumPredictorOnly();
  void initializeStartupPressureField(const SolverParams & solver_params);
  void performStartupContinuityCorrections(const SolverParams & solver_params);
  void writeStartupPressureCutAudit(const std::string & label) const;
  void writeMomentumProbeAudit(const unsigned int simple_iteration_counter,
                               const std::string & stage_label) const;
  void preparePressureCorrectorState(const bool subtract_updated_pressure);
  void reconstructPressureCoupledStateFromCurrentPressure(const bool subtract_updated_pressure);
  void advanceSystemOuterIterationHistory(const std::vector<LinearSystem *> & systems) const;
  void advanceMomentumOuterIterationHistory() const;
  void advanceVolumeFractionOuterIterationHistory() const;
  NonlinearSolutionStateSnapshots snapshotMomentumNonlinearSolutionStates() const;
  void restoreMomentumNonlinearSolutionStates(
      const NonlinearSolutionStateSnapshots & snapshots) const;
  bool shouldPrintStageDiagnostics(const unsigned int simple_iteration_counter) const;
  void printOuterIterationDiagnostics(const unsigned int simple_iteration_counter,
                                      const std::string & stage_label) const;
  void synchronizeSystemState(LinearSystem & system) const;
  std::vector<std::pair<unsigned int, Real>> solveVolumeFractionSystems(
      const SolverParams & solver_params);
  void clampVolumeFractionSystems();
  SharpInterfaceVOFMULESCorrector * sharpInterfaceVOFCorrector(const SolverSystemName & system_name) const;

  std::pair<unsigned int, Real>
  correctVelocityOnce(const bool subtract_updated_pressure,
                      const bool recompute_face_mass_flux,
                      const SolverParams & solver_params);
  std::pair<unsigned int, Real>
  correctStartupContinuityOnce(const bool subtract_updated_pressure,
                               const bool recompute_face_mass_flux,
                               const SolverParams & solver_params);

  SharpInterfaceRhieChowMassFlux * sharpInterfaceRC() const;
  SharpInterfaceCurvatureCalculator * sharpInterfaceCurvature() const;

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
  const bool _volume_fraction_outer_corrections;
  std::string _startup_pressure_initialization;
  const bool _suppress_explicit_hydrostatic_flux_during_seeded_startup;
  const unsigned int _startup_flux_corrections;
  const bool _audit_outer_handoff_stages;
  const unsigned int _audit_outer_handoff_after_outer;
  const bool _audit_stage_diagnostics;
  const Real _audit_stage_diagnostics_start_time;
  const bool _startup_pressure_cut_audit;
  const std::string _startup_pressure_cut_audit_file_base;
  const Real _startup_pressure_cut_audit_y;
  const Real _startup_pressure_cut_audit_z;
  const Real _startup_pressure_cut_audit_reference_pressure;
  const Real _startup_pressure_cut_audit_liquid_density;
  const Real _startup_pressure_cut_audit_gas_density;
  const RealVectorValue _startup_pressure_cut_audit_gravity;
  const Point _startup_pressure_cut_audit_reference_pressure_point;
  const bool _momentum_probe_audit;
  const std::string _momentum_probe_audit_file_base;
  const std::vector<Point> _momentum_probe_points;
  const Real _momentum_probe_audit_start_time;
  const unsigned int _skip_momentum_predictor_after_outer;
  const unsigned int _freeze_alpha_after_outer;
  const unsigned int _skip_pressure_velocity_writeback_from_outer;
  const bool _use_vof_rho_phi_during_momentum_predictor;
  mutable bool _momentum_probe_audit_header_written = false;
  mutable bool _momentum_probe_audit_parallel_warning_emitted = false;
};
