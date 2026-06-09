//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "RhieChowMassFlux.h"
#include "LinearAssemblySegregatedSolve.h"

#include <memory>

/**
 * PIMPLE-based (PISO + SIMPLE) for transient solution object with
 * linear FV system assembly. A detailed discussion of the algorithm
 * is available in
 * @book{
 *   greenshieldsweller2022,
 *   title     = "Notes on Computational Fluid Dynamics: General Principles",
 *   author    = "Greenshields, Christopher and Weller, Henry",
 *   year      = 2022,
 *   publisher = "CFD Direct Ltd",
 *   address   = "Reading, UK"
 * }
 * This will be the basis for the SIMPLE algorithm as well, we just
 * set the PISO iterations to 0.
 */
class PIMPLESolve : public LinearAssemblySegregatedSolve
{
public:
  PIMPLESolve(Executioner & ex);

  static InputParameters validParams();

protected:
  virtual std::pair<unsigned int, Real>
  correctVelocity(const bool subtract_updated_pressure,
                  const bool recompute_face_mass_flux,
                  const SolverParams & solver_params) override;

  virtual void preparePressureCorrectorState(const bool subtract_updated_pressure);
  virtual std::pair<unsigned int, Real>
  applyPressureCorrectionStage(const bool recompute_face_mass_flux,
                               const bool publish_pressure_corrected_state,
                               const SolverParams & solver_params);
  virtual void postPressureCorrectorSolve(const bool final_nonorthogonal_iteration);
  virtual void publishPressureCorrectedState(const bool recompute_face_mass_flux);
  void storePressurePreviousOuterIterationState();
  void relaxPressureFieldForNextPredictor();

  void advanceSystemOuterIterationHistory(const std::vector<LinearSystem *> & systems) const;
  void advancePressureOuterIterationHistory() const;

  bool hasPISOAbsoluteTerminationCriterion() const;
  bool shouldContinuePISOIterations(const unsigned int piso_iteration_counter,
                                    const Real stage_residual,
                                    const Real first_stage_residual) const;

  /// Number of H(u) and u iterations with fixed face flux.
  const unsigned int _num_piso_iterations;
  const Real _piso_absolute_tolerance;
  const Real _piso_relative_tolerance;
  const unsigned int _num_pressure_nonorthogonal_correctors;
  unsigned int _current_piso_iteration = 0;

private:
  std::unique_ptr<NumericVector<Number>> _pressure_previous_outer_solution;
};
