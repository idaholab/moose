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

#include <vector>

class ConservativeDiffuseInterfaceRhieChowMassFlux;
class ConservativeDiffuseInterfaceVOFMULESCorrector;
class ConservativeDiffuseInterfaceMixtureEnthalpy;

/**
 * The solve object of the reduced-pressure diffuse-interface PIMPLE algorithm. On top of the
 * parent's momentum-pressure loop, one outer iteration of a step runs as follows:
 * 1. The corrected face flux of the previous outer iteration (the previous accepted step on the
 *    first one) is frozen as lagged_corrected_face_phi.
 * 2. The volume fraction is solved with that flux: an implicit upwind solve followed by the
 *    MULES correction, in as many subcycles as the volume-fraction Courant number requires. The
 *    corrector publishes the limited alpha flux and the consistent mass flux rho_phi.
 * 3. The momentum predictor is assembled with the mixture density and rho_phi of the new volume
 *    fraction, and the pressure equation is solved as in the parent, with the diffuse-interface
 *    Rhie-Chow object supplying the face forces and the volumetric flux form.
 * 4. The energy equation, when present, is solved for the conserved mixture enthalpy and the
 *    temperature is recovered from it.
 * The outer loop stops once the momentum and pressure residuals and the largest change of the
 * volume fraction between outer iterations are below their tolerances.
 *
 * On the first step an optional startup projection repairs the initial face flux with
 * pressure-only continuity solves while leaving the user's reduced pressure field untouched, so
 * that the transient correction of the Rhie-Chow object has a divergence-free flux to start from.
 */
class ReducedPressurePIMPLESolve : public PIMPLESolve
{
public:
  static InputParameters validParams();

  ReducedPressurePIMPLESolve(Executioner & ex);

  /// Stores the face flux and transport state of a step the executioner accepted, which the next
  /// step's first outer iteration and transient correction start from
  void commitAcceptedTimestepTransportHistory() const;

protected:
  void initializeSolveLoop(const SolverParams & solver_params) override;
  /// Advances the outer-iteration histories, then solves the volume fraction with the corrected
  /// face flux frozen for this outer iteration
  void preMomentumPressureIteration(ResidualStorage & residual_storage,
                                    const SolverParams & solver_params) override;
  /// The volume fraction is solved before the flow correction of each outer iteration instead
  bool shouldSolveActiveScalarsAfterFlowLoop() const override { return false; }
  void postPreparePressureCorrectorState() override;
  void postPublishPressureCorrectedState() override;
  /// Recovers the temperature from the solved conserved mixture enthalpy
  void postEnergySolve() override;
  /**
   * The conserved-enthalpy energy matrix is the temperature system, an M-matrix, with its columns
   * scaled by 1 / (d(rho*h)/dT) > 0. It is not diagonally dominant where the heat capacity jumps
   * across the interface, and enforcing dominance would lag the solve by a correction of the size
   * of rho*h itself.
   */
  bool enforceEnergyDiagonalDominance() const override { return mixtureEnthalpies().empty(); }

private:
  /// Whether volume-fraction systems exist and are to be solved
  bool solvesVolumeFraction() const;
  /// The startup projection: assembles the predictor coefficients and runs pressure-only
  /// continuity corrections that repair the face flux and then restore the pressure field
  void initializeStartupPressureField(const SolverParams & solver_params);
  /// Assembles the momentum predictor operators without solving them, so that the startup
  /// projection's pressure equation has H/A and 1/A
  void assembleMomentumPredictorWithoutSolve();
  /// Shifts the pressure and momentum systems' nonlinear state stacks once per outer iteration
  void advanceOuterIterationHistories();
  /**
   * Shifts the nonlinear solution-state stack of each system by one and stores the current
   * solution as state 1, so that state 1 is the previous outer iterate for the whole coming outer
   * iteration however many linear solves it contains.
   */
  void advanceSystemOuterIterationHistory(const std::vector<LinearSystem *> & systems) const;
  /// The larger of the requested subcycle count and the one that keeps the volume-fraction
  /// Courant number of the current flux under volume_fraction_max_courant
  unsigned int computeVolumeFractionSubcycles() const;
  /// Copies a system's current solution into its old, previous-iterate and all nonlinear states
  void synchronizeSystemState(LinearSystem & system) const;
  /// Copies a system's current solution into its previous-iterate state, when it keeps one
  void setPreviousNewtonToCurrent(LinearSystem & system) const;
  /// Solves every volume-fraction system through its subcycles and publishes the transport
  /// fluxes at the global time
  std::vector<std::pair<unsigned int, Real>> solveVolumeFractionSystems();
  /**
   * Solves one volume-fraction system over all subcycles of the step, starting from the
   * time-step-old alpha, and reports the largest change of alpha from the previous outer iterate
   * as its residual.
   */
  std::pair<unsigned int, Real> solveOneVolumeFractionSystem(const unsigned int i,
                                                             const unsigned int num_subcycles,
                                                             const Real subcycle_dt,
                                                             const Real global_dt,
                                                             const Real global_time_old);
  /// One subcycle: set the problem time, solve the upwind system, apply the MULES correction
  std::pair<unsigned int, Real>
  runOneVolumeFractionSubcycle(const unsigned int i,
                               LinearSystem & system,
                               ConservativeDiffuseInterfaceVOFMULESCorrector & corrector,
                               const unsigned int subcycle,
                               const Real subcycle_dt,
                               const Real global_dt,
                               const Real global_time_old);
  /// The corrector of a volume-fraction system, found by its system name; null if there is none
  ConservativeDiffuseInterfaceVOFMULESCorrector *
  diffuseInterfaceVOFCorrector(const SolverSystemName & system_name) const;

  /// One pressure-only continuity correction of the startup projection
  std::pair<unsigned int, Real> correctStartupContinuityOnce(const SolverParams & solver_params);

  /// The Rhie-Chow object as the diffuse-interface one, or null with the stock object
  ConservativeDiffuseInterfaceRhieChowMassFlux * diffuseInterfaceRC() const;
  /// The objects relating a conserved mixture enthalpy to its temperature
  std::vector<ConservativeDiffuseInterfaceMixtureEnthalpy *> mixtureEnthalpies() const;

  /// Requested number of volume-fraction subcycles per step, raised when the Courant limit needs
  const unsigned int _volume_fraction_subcycles;
  /// The volume-fraction Courant number the subcycles keep under
  const Real _volume_fraction_max_courant;
  /// Whether the first step starts with the startup projection
  const bool _startup_continuity_projection;
  /// Number of pressure-only corrections of the startup projection
  const unsigned int _startup_flux_corrections;
  /// The outer iteration of the current step, counted from one
  unsigned int _current_outer_iteration = 0;
};
