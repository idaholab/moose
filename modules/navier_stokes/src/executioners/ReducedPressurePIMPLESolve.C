//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReducedPressurePIMPLESolve.h"

#include "LinearSystem.h"
#include "ConservativeSharpInterfaceRhieChowMassFlux.h"
#include "ConservativeSharpInterfaceVOFMULESCorrector.h"
#include "TheWarehouse.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
using namespace libMesh;

InputParameters
ReducedPressurePIMPLESolve::validParams()
{
  InputParameters params = PIMPLESolve::validParams();
  params.set<unsigned int>("num_iterations") = 1;
  params.setDocString(
      "num_iterations",
      "The number of outer PIMPLE corrections. For the transient reduced-pressure sharp-interface "
      "path this should remain a small outer-correction count, not a large SIMPLE-style "
      "momentum-pressure convergence loop. The reduced-pressure executioner performs this many "
      "outer corrections explicitly unless a future outer-state convergence metric is added.");
  params.setDocString("num_piso_iterations",
                      "The maximum number of additional inner pressure-correction-only PISO "
                      "stages performed without rebuilding the momentum matrix on each outer "
                      "correction. By default the reduced-pressure executioner performs this many "
                      "additional stages explicitly; early exit only occurs when explicit PISO "
                      "termination tolerances are provided.");
  params.addClassDescription(
      "PIMPLE solve object with explicit hooks for reduced-pressure sharp-interface face-flux "
      "predictors.");
  params.setDocString(
      "active_scalar_systems",
      "The solver system for each sharp-interface volume-fraction transport equation.");
  params.setDocString("should_solve_active_scalars",
                      "Whether to solve the volume-fraction transport equation(s).");
  params.setDocString("active_scalar_equation_relaxation",
                      "The relaxation used for the volume-fraction transport equation(s).");
  params.setDocString(
      "active_scalar_absolute_tolerance",
      "The absolute tolerance(s) on the normalized residual(s) of the volume-fraction "
      "equation(s).");
  params.addRangeCheckedParam<unsigned int>(
      "volume_fraction_subcycles",
      1,
      "volume_fraction_subcycles>0",
      "Number of full alpha transport subcycles used by the bounded volume-fraction update.");
  params.addRangeCheckedParam<Real>(
      "volume_fraction_max_courant",
      1.0,
      "volume_fraction_max_courant>0",
      "Maximum allowed alpha Courant number during subcycling. The executioner increases the "
      "alpha subcycle count as needed so the current transported volumetric flux satisfies this "
      "limit.");
  MooseEnum startup_pressure_initialization("none projection-only", "projection-only");
  params.addParam<MooseEnum>(
      "startup_pressure_initialization",
      startup_pressure_initialization,
      "Startup reduced-pressure initialization policy on the first time step. Use "
      "'projection-only' to apply startup continuity projection without overwriting the "
      "user-supplied reduced-pressure field, or 'none' to skip startup pressure "
      "cleanup entirely.");
  params.addRangeCheckedParam<unsigned int>(
      "startup_flux_corrections",
      1,
      "startup_flux_corrections>0",
      "Number of pressure-only startup cleanup / projection corrections applied when "
      "startup_pressure_initialization is not 'none'.");
  params.addParamNamesToGroup(
      "volume_fraction_subcycles volume_fraction_max_courant startup_pressure_initialization "
      "startup_flux_corrections num_pressure_nonorthogonal_correctors",
      "Volume Fraction Equations");
  return params;
}

ReducedPressurePIMPLESolve::ReducedPressurePIMPLESolve(Executioner & ex)
  : PIMPLESolve(ex),
    _volume_fraction_subcycles(getParam<unsigned int>("volume_fraction_subcycles")),
    _volume_fraction_max_courant(getParam<Real>("volume_fraction_max_courant")),
    _startup_flux_corrections(getParam<unsigned int>("startup_flux_corrections"))
{
  _startup_pressure_initialization =
      getParam<MooseEnum>("startup_pressure_initialization").operator std::string();

  if (_pin_pressure)
    paramError("pin_pressure",
               "ReducedPressurePIMPLE supports pressure boundary-condition constraints only; "
               "pressure pinning is not supported.");
}

bool
ReducedPressurePIMPLESolve::startupPressureInitializationEnabled() const
{
  return _startup_pressure_initialization != "none";
}

ConservativeSharpInterfaceVOFMULESCorrector *
ReducedPressurePIMPLESolve::sharpInterfaceVOFCorrector(const SolverSystemName & system_name) const
{
  std::vector<UserObject *> objs;
  _problem.theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribThread>(0)
      .queryInto(objs);

  ConservativeSharpInterfaceVOFMULESCorrector * corrector_match = nullptr;
  for (const auto & obj : objs)
    if (auto * corrector = dynamic_cast<ConservativeSharpInterfaceVOFMULESCorrector *>(obj);
        corrector && corrector->systemName() == system_name)
    {
      if (corrector_match)
        mooseError(
            "ReducedPressurePIMPLESolve found multiple ConservativeSharpInterfaceVOFMULESCorrector "
            "objects for system '",
            system_name,
            "'.");
      corrector_match = corrector;
    }

  return corrector_match;
}

void
ReducedPressurePIMPLESolve::preSolveSetup(const SolverParams & /* solver_params */)
{
  if (auto * sharp_rc = sharpInterfaceRC())
    sharp_rc->setSuppressExplicitHydrostaticPressureFlux(false);
}

void
ReducedPressurePIMPLESolve::initializeSolveLoop(const SolverParams & solver_params)
{
  if (_has_active_scalar_systems && _should_solve_active_scalars)
    if (auto * sharp_rc = sharpInterfaceRC())
      sharp_rc->clearVOFTransportState();

  if (startupPressureInitializationEnabled() && _problem.timeStep() == 1)
  {
    for (auto * system : _momentum_systems)
      synchronizeSystemState(*system);
    synchronizeSystemState(_pressure_system);
    for (auto * system : _active_scalar_systems)
      synchronizeSystemState(*system);

    _problem.execute(EXEC_NONLINEAR);
  }

  if (_should_solve_pressure)
  {
    initializeStartupPressureField(solver_params);
    if (_has_active_scalar_systems && _should_solve_active_scalars)
      if (auto * sharp_rc = sharpInterfaceRC())
        sharp_rc->commitAcceptedTimestepTransportHistory();
  }
}

void
ReducedPressurePIMPLESolve::preMomentumPressureIteration(ResidualStorage & residual_storage,
                                                         const SolverParams & solver_params)
{
  if (_should_solve_pressure)
    advancePressureOuterIterationHistory();

  if (_should_solve_momentum)
    // Keep the full nonlinear history on the previous outer-corrector state
    // for the whole current outer loop. The stock momentum solve shifts this
    // stack every predictor solve; here we only want to advance it
    // once per outer SIMPLE iteration.
    advanceSystemOuterIterationHistory(_momentum_systems);

  // Do the alpha subcycling and mixture/rhoPhi refresh inside every outer correction, just before
  // the momentum-pressure coupling work. This keeps rhoPhi consistent with the outer-corrector
  // state instead of freezing one alpha update for a later sequence of momentum-pressure
  // repredictions.
  if (_has_active_scalar_systems && _should_solve_active_scalars)
  {
    // Keep the true timestep-old alpha in solutionOld(), but advance the
    // nonlinear-state stack once per outer iteration so we have a separate
    // previous-outer iterate available.
    advanceSystemOuterIterationHistory(_active_scalar_systems);

    const bool use_previous_timestep_transport_flux =
        _problem.timeStep() == 1 && _current_outer_iteration == 1;
    if (auto * sharp_rc = sharpInterfaceRC())
    {
      sharp_rc->clearVOFTransportState();
      sharp_rc->freezeVOFTransportState(use_previous_timestep_transport_flux);
    }

    _problem.execute(EXEC_NONLINEAR);
    Moose::PetscSupport::petscSetOptions(_active_scalar_petsc_options, solver_params);
    const auto vf_residuals = solveVolumeFractionSystems();

    if (auto * sharp_rc = sharpInterfaceRC())
      sharp_rc->adoptPublishedVOFTransportState();

    _problem.execute(EXEC_NONLINEAR);
    for (const auto i : index_range(vf_residuals))
      residual_storage.ns_residuals[residual_storage.active_scalar_indices[i]] = vf_residuals[i];
  }
}

bool
ReducedPressurePIMPLESolve::shouldAssembleMomentumPredictorWithoutSolve() const
{
  return _should_solve_pressure && !_momentum_systems.empty() && _rc_uo;
}

bool
ReducedPressurePIMPLESolve::shouldSolveActiveScalarsAfterFlowLoop() const
{
  return false;
}

void
ReducedPressurePIMPLESolve::finalizeSolve(const bool /* converged */)
{
  if (auto * sharp_rc = sharpInterfaceRC())
    sharp_rc->setSuppressExplicitHydrostaticPressureFlux(false);
}

void
ReducedPressurePIMPLESolve::addMomentumPredictorExplicitForcing(const unsigned int system_i,
                                                                NumericVector<Number> & rhs)
{
  if (auto * sharp_rc = sharpInterfaceRC(); sharp_rc && sharp_rc->splitMomentumPredictorOperator())
    sharp_rc->addMomentumPredictorExplicitForcing(system_i, rhs);
}

ConservativeSharpInterfaceRhieChowMassFlux *
ReducedPressurePIMPLESolve::sharpInterfaceRC() const
{
  return dynamic_cast<ConservativeSharpInterfaceRhieChowMassFlux *>(_rc_uo);
}

void
ReducedPressurePIMPLESolve::commitAcceptedTimestepTransportHistory() const
{
  if (auto * sharp_rc = sharpInterfaceRC())
    sharp_rc->commitAcceptedTimestepTransportHistory();
}

void
ReducedPressurePIMPLESolve::synchronizeSystemState(LinearSystem & system) const
{
  auto & current_local_solution = *(system.system().current_local_solution);
  current_local_solution.close();
  system.setSolution(current_local_solution);

  auto & current_solution = system.solution();
  current_solution.close();

  system.solutionOld().close();
  system.solutionOld() = current_solution;
  system.solutionOld().close();

  for (unsigned int state = 1;
       system.hasSolutionState(state, Moose::SolutionIterationType::Nonlinear);
       ++state)
  {
    auto & nonlinear_state = system.solutionState(state, Moose::SolutionIterationType::Nonlinear);
    nonlinear_state.close();
    nonlinear_state = system.solutionOld();
    nonlinear_state.close();
  }

  if (auto * previous_newton_solution = system.solutionPreviousNewton())
  {
    previous_newton_solution->close();
    *previous_newton_solution = current_solution;
    previous_newton_solution->close();
  }
}

void
ReducedPressurePIMPLESolve::initializeStartupPressureField(const SolverParams & solver_params)
{
  if (!startupPressureInitializationEnabled() || _problem.timeStep() != 1)
    return;

  if (!_should_solve_pressure)
    return;

  _console << "Applying startup continuity projection before PIMPLE iterations" << std::endl;

  // Honor the current reduced-pressure field, assemble the momentum predictor coefficients, and run
  // pressure-only startup continuity corrections before the first outer iteration.
  if (_momentum_systems.empty() || !_rc_uo)
    mooseError("ReducedPressurePIMPLESolve startup projection requires momentum systems and a "
               "Rhie-Chow user object.");

  assembleMomentumPredictorWithoutSolve();
  _rc_uo->initFaceMassFlux();

  _console << "Applying startup continuity corrections" << std::endl;

  for (unsigned int startup_it = 0; startup_it < _startup_flux_corrections; ++startup_it)
    (void)correctStartupContinuityOnce(solver_params);

  synchronizeSystemState(_pressure_system);
  _problem.execute(EXEC_NONLINEAR);
}

void
ReducedPressurePIMPLESolve::postPreparePressureCorrectorState(const bool subtract_updated_pressure)
{
  if (auto * sharp_rc = sharpInterfaceRC())
    sharp_rc->updateAdditionalPressureFluxFunctors(subtract_updated_pressure, _print_fields);

  // Refresh the patch velocity / target-flux state from the latest momentum
  // predictor before assembling the constrained pressure boundary gradient.
  _rc_uo->updateVelocityBoundaryState();

  _rc_uo->updatePressureBoundaryNormalGradients(/* apply_pressure_flux_adjustment = */ false);
}

unsigned int
ReducedPressurePIMPLESolve::computeVolumeFractionSubcycles() const
{
  unsigned int subcycles = _volume_fraction_subcycles;

  if (const auto * sharp_rc = sharpInterfaceRC())
  {
    const Real alpha_courant = sharp_rc->maxCourant(_problem.dt());
    if (std::isfinite(alpha_courant) && alpha_courant > _volume_fraction_max_courant)
    {
      const auto required_subcycles =
          static_cast<unsigned int>(std::ceil(alpha_courant / _volume_fraction_max_courant));
      subcycles = std::max(subcycles, std::max(required_subcycles, 1u));
    }
  }

  return std::max(subcycles, 1u);
}

std::vector<std::pair<unsigned int, Real>>
ReducedPressurePIMPLESolve::solveVolumeFractionSystems()
{
  std::vector<std::pair<unsigned int, Real>> residuals(_active_scalar_system_names.size(),
                                                       std::make_pair(0, 1.0));

  const Real global_dt = _problem.dt();
  const Real global_time = _problem.time();
  const Real global_time_old = _problem.timeOld();
  const unsigned int num_subcycles = computeVolumeFractionSubcycles();
  const Real subcycle_dt = global_dt / num_subcycles;

  if (num_subcycles > _volume_fraction_subcycles)
    _console << name() << ": increasing alpha subcycles from " << _volume_fraction_subcycles
             << " to " << num_subcycles << " to keep alpha CFL <= " << _volume_fraction_max_courant
             << " at dt=" << global_dt << std::endl;

  for (const auto i : index_range(_active_scalar_system_names))
  {
    auto * system = _active_scalar_systems[i];
    system->saveOldSolutions();

    // solutionOld() must stay as the true timestep-old alpha for the whole
    // outer loop. solutionPreviousNewton() is only the local/subcycle field-
    // relaxation state, while the previous-outer iterate now lives in the
    // nonlinear solution-state stack advanced at outer-loop entry.
    if (auto * previous_solution = system->solutionPreviousNewton())
    {
      *previous_solution = *(system->system().current_local_solution);
      previous_solution->close();
    }

    auto * corrector = sharpInterfaceVOFCorrector(_active_scalar_system_names[i]);
    if (corrector)
      corrector->resetSubcycleFluxes();

    for (const auto subcycle : make_range(num_subcycles))
    {
      _problem.dt() = subcycle_dt;
      _problem.timeOld() = global_time_old + subcycle * subcycle_dt;
      _problem.time() = _problem.timeOld() + subcycle_dt;

      if (subcycle > 0)
      {
        system->solutionOld() = *(system->system().current_local_solution);
        system->solutionOld().close();
        if (auto * previous_solution = system->solutionPreviousNewton())
        {
          *previous_solution = system->solutionOld();
          previous_solution->close();
        }
      }

      _problem.execute(EXEC_NONLINEAR);
      if (!corrector)
        mooseError(
            "ReducedPressurePIMPLESolve requires a ConservativeSharpInterfaceVOFMULESCorrector "
            "for volume-fraction system '",
            _active_scalar_system_names[i],
            "'.");

      // This path bounds alpha through the limited face fluxes instead of projecting the solved
      // field. Do not apply the inherited scalar lower limiter here.
      residuals[i] = solveAdvectedSystem(_active_scalar_system_numbers[i],
                                         *system,
                                         _active_scalar_equation_relaxation[i],
                                         _active_scalar_linear_control,
                                         _active_scalar_l_abs_tol,
                                         1.0,
                                         std::numeric_limits<Real>::min());
      system->computeGradients();
      corrector->applyCorrection(subcycle_dt, subcycle_dt / global_dt);
    }

    system->restoreOldSolutions();
    if (auto * previous_solution = system->solutionPreviousNewton())
    {
      *previous_solution = *(system->system().current_local_solution);
      previous_solution->close();
    }
  }

  _problem.dt() = global_dt;
  _problem.time() = global_time;
  _problem.timeOld() = global_time_old;

  finalizeVolumeFractionTransportState();

  return residuals;
}

void
ReducedPressurePIMPLESolve::finalizeVolumeFractionTransportState()
{
  for (const auto & system : _active_scalar_systems)
    system->computeGradients();

  for (const auto & system_name : _active_scalar_system_names)
    if (auto * corrector = sharpInterfaceVOFCorrector(system_name))
      corrector->refreshPublishedRhoPhi();
}

std::pair<unsigned int, Real>
ReducedPressurePIMPLESolve::correctStartupContinuityOnce(const SolverParams & solver_params)
{
  LinearImplicitSystem & pressure_linear_system =
      libMesh::cast_ref<LinearImplicitSystem &>(_pressure_system.system());
  auto & pressure_current_solution = *(_pressure_system.system().current_local_solution.get());
  auto saved_pressure_current_solution = pressure_current_solution.zero_clone();
  *saved_pressure_current_solution = pressure_current_solution;
  saved_pressure_current_solution->close();

  auto & pressure_linear_solution = *(pressure_linear_system.solution);
  auto saved_pressure_linear_solution = pressure_linear_solution.zero_clone();
  *saved_pressure_linear_solution = pressure_linear_solution;
  saved_pressure_linear_solution->close();

  auto * const pressure_old_solution = _pressure_system.solutionPreviousNewton();
  std::unique_ptr<NumericVector<Number>> saved_pressure_old_solution;
  if (pressure_old_solution)
  {
    saved_pressure_old_solution = pressure_old_solution->zero_clone();
    *saved_pressure_old_solution = *pressure_old_solution;
    saved_pressure_old_solution->close();
  }

  auto * const sharp_rc = sharpInterfaceRC();
  const bool saved_suppress_explicit_hydrostatic_pressure_flux =
      sharp_rc ? sharp_rc->suppressExplicitHydrostaticPressureFlux() : false;
  const bool saved_suppress_startup_pressure_predictor_flux_sources =
      sharp_rc ? sharp_rc->suppressStartupPressurePredictorFluxSources() : false;
  if (sharp_rc)
  {
    // Projection-only startup cleanup should solve only the continuity flux repair.
    sharp_rc->setSuppressStartupPressurePredictorFluxSources(true);
    sharp_rc->setSuppressExplicitHydrostaticPressureFlux(true);
  }

  preparePressureCorrectorState(true);

  const auto residuals = applyPressureCorrectionStage(true, false, solver_params);

  // Restore the user/equilibrium startup reduced-pressure field. Startup
  // continuity cleanup should repair phi, not overwrite the physical p_rgh field
  // before the first real pressure equation.
  pressure_current_solution = *saved_pressure_current_solution;
  pressure_current_solution.close();
  pressure_linear_solution = *saved_pressure_linear_solution;
  pressure_linear_solution.close();
  _pressure_system.setSolution(pressure_current_solution);

  if (pressure_old_solution && saved_pressure_old_solution)
  {
    *pressure_old_solution = *saved_pressure_old_solution;
    pressure_old_solution->close();
  }

  _pressure_system.computeGradients();

  if (sharp_rc)
  {
    sharp_rc->setSuppressStartupPressurePredictorFluxSources(
        saved_suppress_startup_pressure_predictor_flux_sources);
    sharp_rc->setSuppressExplicitHydrostaticPressureFlux(
        saved_suppress_explicit_hydrostatic_pressure_flux);
  }

  return residuals;
}

void
ReducedPressurePIMPLESolve::postPublishPressureCorrectedState()
{
  _rc_uo->updateVelocityBoundaryState();
}
