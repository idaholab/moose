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
  void assembleMomentumPredictorWithoutSolve() override;
  bool shouldSolveActiveScalarsAfterFlowLoop() const override;
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
  ConservativeSharpInterfaceVOFMULESCorrector *
  sharpInterfaceVOFCorrector(const SolverSystemName & system_name) const;

  std::pair<unsigned int, Real> correctStartupContinuityOnce(const bool subtract_updated_pressure,
                                                             const bool recompute_face_mass_flux,
                                                             const SolverParams & solver_params);

  ConservativeSharpInterfaceRhieChowMassFlux * sharpInterfaceRC() const;

  const unsigned int _volume_fraction_subcycles;
  const Real _volume_fraction_max_courant;
  std::string _startup_pressure_initialization;
  const unsigned int _startup_flux_corrections;
};
