//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PIMPLESolve.h"
#include "RhieChowMassFlux.h"

#include "libmesh/point.h"

#include <string>
#include <vector>

class ConservativeSharpInterfaceRhieChowMassFlux;
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

  Real momentumPressureCourant(const Real dt) const;
  Real constrainedMomentumPressureDT(const Real dt) const;
  void commitAcceptedTimestepTransportHistory() const;
  bool adjustMomentumPressureTimeStepEnabled() const { return _adjust_momentum_pressure_time_step; }
  Real momentumPressureMaxCourant() const { return _momentum_pressure_max_courant; }

protected:
  void preSolveSetup(const SolverParams & solver_params) override;
  void addIterationResiduals(ResidualStorage & residual_storage) override;
  void initializeSolveLoop(const SolverParams & solver_params) override;
  void preMomentumPressureIteration(ResidualStorage & residual_storage,
                                    const SolverParams & solver_params) override;
  bool shouldAssembleMomentumPredictorWithoutSolve() const override;
  void assembleMomentumPredictorWithoutSolve() override;
  void finalizeSolve(const bool converged) override;
  void addMomentumPredictorExplicitForcing(const unsigned int system_i,
                                           NumericVector<Number> & rhs) override;
  std::vector<std::pair<unsigned int, Real>> solveMomentumPredictor() override;
  void preparePressureCorrectorState(const bool subtract_updated_pressure) override;
  void publishPressureCorrectedState(const bool recompute_face_mass_flux) override;

private:
  using NonlinearSolutionStateSnapshots =
      std::vector<std::vector<std::unique_ptr<NumericVector<Number>>>>;

  bool startupPressureInitializationEnabled() const;
  void assembleMomentumPredictorOnly();
  void initializeStartupPressureField(const SolverParams & solver_params);
  void performStartupContinuityCorrections(const SolverParams & solver_params);
  unsigned int computeVolumeFractionSubcycles() const;
  NonlinearSolutionStateSnapshots snapshotMomentumNonlinearSolutionStates() const;
  void
  restoreMomentumNonlinearSolutionStates(const NonlinearSolutionStateSnapshots & snapshots) const;
  void synchronizeSystemState(LinearSystem & system) const;
  std::vector<std::pair<unsigned int, Real>>
  solveVolumeFractionSystems(const SolverParams & solver_params);
  void finalizeVolumeFractionTransportState();
  void clampVolumeFractionSystem(LinearSystem & system);
  ConservativeSharpInterfaceVOFMULESCorrector *
  sharpInterfaceVOFCorrector(const SolverSystemName & system_name) const;

  std::pair<unsigned int, Real> correctVelocityOnce(const bool subtract_updated_pressure,
                                                    const bool recompute_face_mass_flux,
                                                    const SolverParams & solver_params);
  std::pair<unsigned int, Real> correctStartupContinuityOnce(const bool subtract_updated_pressure,
                                                             const bool recompute_face_mass_flux,
                                                             const SolverParams & solver_params);

  ConservativeSharpInterfaceRhieChowMassFlux * sharpInterfaceRC() const;

  const std::vector<SolverSystemName> _volume_fraction_system_names;
  const bool _has_volume_fraction_systems;
  const bool _should_solve_volume_fractions;
  std::vector<std::size_t> _volume_fraction_indices;
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
  std::string _startup_pressure_initialization;
  const unsigned int _startup_flux_corrections;
};
