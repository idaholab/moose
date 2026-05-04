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
  void assembleMomentumPredictorOnly();
  void initializeStartupPressureField(const SolverParams & solver_params);
  void performStartupContinuityCorrections(const SolverParams & solver_params);
  void preparePressureCorrectorState(const bool subtract_updated_pressure);
  void reconstructPressureCoupledStateFromCurrentPressure(const bool subtract_updated_pressure);
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
  const Real _volume_fraction_max_courant;
  const bool _volume_fraction_outer_corrections;
  std::string _startup_pressure_initialization;
  const bool _suppress_explicit_hydrostatic_flux_during_seeded_startup;
  const unsigned int _startup_flux_corrections;
};
