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

  void commitAcceptedTimestepTransportHistory() const;

protected:
  void preSolveSetup(const SolverParams & solver_params) override;
  void initializeSolveLoop(const SolverParams & solver_params) override;
  void preMomentumPressureIteration(ResidualStorage & residual_storage,
                                    const SolverParams & solver_params) override;
  bool shouldAssembleMomentumPredictorWithoutSolve() const override;
  bool shouldSolveActiveScalarsAfterFlowLoop() const override;
  void finalizeSolve(const bool converged) override;
  void addMomentumPredictorExplicitForcing(const unsigned int system_i,
                                           NumericVector<Number> & rhs) override;
  bool shouldCopyMomentumNonlinearSolutionHistory() const override { return false; }
  void postPreparePressureCorrectorState(const bool subtract_updated_pressure) override;
  void postPublishPressureCorrectedState() override;

private:
  bool startupPressureInitializationEnabled() const;
  bool shouldRunStartupInitialization() const;
  bool solvesVolumeFraction() const;
  void initializeStartupPressureField(const SolverParams & solver_params);
  void resetVOFTransportStateForNewSolve() const;
  void initializeConsistentStartupState();
  void commitAcceptedVOFTransportHistoryIfNeeded() const;
  void advanceOuterIterationHistories();
  void solveVolumeFractionBeforeFlowCorrection(ResidualStorage & residual_storage,
                                               const SolverParams & solver_params);
  void prepareVOFTransportStateForOuterIteration() const;
  void adoptPublishedVOFTransportState() const;
  void
  storeActiveScalarResiduals(ResidualStorage & residual_storage,
                             const std::vector<std::pair<unsigned int, Real>> & vf_residuals) const;
  unsigned int computeVolumeFractionSubcycles() const;
  void synchronizeSystemState(LinearSystem & system) const;
  void setPreviousNewtonToCurrent(LinearSystem & system) const;
  void advanceVolumeFractionSubcycleOldState(LinearSystem & system) const;
  void setProblemSubcycleTime(const unsigned int subcycle,
                              const Real subcycle_dt,
                              const Real global_time_old);
  std::vector<std::pair<unsigned int, Real>> solveVolumeFractionSystems();
  std::pair<unsigned int, Real> solveOneVolumeFractionSystem(const unsigned int i,
                                                             const unsigned int num_subcycles,
                                                             const Real subcycle_dt,
                                                             const Real global_dt,
                                                             const Real global_time_old);
  std::pair<unsigned int, Real>
  runOneVolumeFractionSubcycle(const unsigned int i,
                               LinearSystem & system,
                               ConservativeSharpInterfaceVOFMULESCorrector & corrector,
                               const unsigned int subcycle,
                               const Real subcycle_dt,
                               const Real global_dt,
                               const Real global_time_old);
  void finalizeVolumeFractionTransportState();
  ConservativeSharpInterfaceVOFMULESCorrector *
  sharpInterfaceVOFCorrector(const SolverSystemName & system_name) const;

  std::pair<unsigned int, Real> correctStartupContinuityOnce(const SolverParams & solver_params);

  ConservativeSharpInterfaceRhieChowMassFlux * sharpInterfaceRC() const;

  const unsigned int _volume_fraction_subcycles;
  const Real _volume_fraction_max_courant;
  std::string _startup_pressure_initialization;
  const unsigned int _startup_flux_corrections;
};
