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
#include "ConservativeDiffuseInterfaceRhieChowMassFlux.h"
#include "ConservativeDiffuseInterfaceVOFMULESCorrector.h"
#include "FEProblemBase.h"
#include "TheWarehouse.h"
#include "TimeIntegrator.h"

#include "libmesh/petsc_linear_solver.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
using namespace libMesh;

namespace
{
/// Restores the problem's dt, time and old time on destruction. The volume-fraction subcycles
/// move all three, and everything after them must see the step's own values again.
class ProblemTimeGuard
{
public:
  explicit ProblemTimeGuard(FEProblemBase & problem)
    : _problem(problem), _dt(problem.dt()), _time(problem.time()), _time_old(problem.timeOld())
  {
  }

  ~ProblemTimeGuard() { restore(); }

  void restore()
  {
    _problem.dt() = _dt;
    _problem.time() = _time;
    _problem.timeOld() = _time_old;
  }

private:
  FEProblemBase & _problem;
  const Real _dt;
  const Real _time;
  const Real _time_old;
};

/// Marks the Rhie-Chow object as running the startup projection for the guard's lifetime, which
/// switches its hydrostatic, capillary and transient face fluxes off
class DiffuseInterfaceStartupProjectionGuard
{
public:
  explicit DiffuseInterfaceStartupProjectionGuard(ConservativeDiffuseInterfaceRhieChowMassFlux * rc)
    : _rc(rc)
  {
    if (_rc)
      _rc->setStartupProjectionActive(true);
  }

  ~DiffuseInterfaceStartupProjectionGuard()
  {
    if (_rc)
      _rc->setStartupProjectionActive(false);
  }

private:
  ConservativeDiffuseInterfaceRhieChowMassFlux * const _rc;
};

/// Saves the pressure system's current and libMesh solutions and restores them on request or
/// destruction, so the startup projection can solve for a pressure without keeping it. The
/// pressure solve does not touch the previous iterate, which needs no saving.
class PressureStateGuard
{
public:
  explicit PressureStateGuard(LinearSystem & pressure_system)
    : _pressure_system(pressure_system),
      _linear_system(libMesh::cast_ref<LinearImplicitSystem &>(pressure_system.system())),
      _current_solution(*pressure_system.system().current_local_solution),
      _linear_solution(*_linear_system.solution),
      _saved_current(_current_solution.zero_clone()),
      _saved_linear(_linear_solution.zero_clone())
  {
    *_saved_current = _current_solution;
    _saved_current->close();

    *_saved_linear = _linear_solution;
    _saved_linear->close();
  }

  ~PressureStateGuard()
  {
    if (!_restored)
      restore();
  }

  void restore()
  {
    _current_solution = *_saved_current;
    _current_solution.close();

    _linear_solution = *_saved_linear;
    _linear_solution.close();

    _pressure_system.setSolution(_current_solution);

    _restored = true;
  }

private:
  LinearSystem & _pressure_system;
  LinearImplicitSystem & _linear_system;
  NumericVector<Number> & _current_solution;
  NumericVector<Number> & _linear_solution;
  std::unique_ptr<NumericVector<Number>> _saved_current;
  std::unique_ptr<NumericVector<Number>> _saved_linear;
  bool _restored = false;
};
}

// Correctness invariants for this reduced-pressure diffuse-interface solve:
//
// 1. The volume-fraction equation is solved inside each outer PIMPLE correction, before
//    momentum-pressure coupling, so rho, mu, rhoPhi, and VOF transport fluxes correspond to the
//    current outer state.
// 2. During VOF subcycling, solutionOld() is temporarily advanced between subcycles, but after
//    subcycling it must be restored to the true timestep-old alpha.
// 3. Startup continuity projection may repair face fluxes, but it must not overwrite the
//    user-supplied initial reduced-pressure field.
// 4. Problem time must be restored before publishing/finalizing VOF transport state.
// 5. Pressure boundary normal gradients must be refreshed after HbyA is computed and before the
//    pressure equation is assembled.
// 6. Explicit hydrostatic/startup pressure predictor flux suppressions are temporary startup
//    projection state and must always be restored.

InputParameters
ReducedPressurePIMPLESolve::validParams()
{
  InputParameters params = PIMPLESolve::validParams();
  params.set<unsigned int>("num_iterations") = 1;
  params.setDocString(
      "num_iterations",
      "The maximum number of outer PIMPLE corrections per time step. The outer loop stops earlier "
      "once the momentum, pressure and volume-fraction residuals are all below their absolute "
      "tolerances.");
  params.setDocString("num_piso_iterations",
                      "The number of additional pressure-correction-only PISO stages performed "
                      "without rebuilding the momentum matrix on each outer correction.");
  params.addClassDescription(
      "PIMPLE solve object with explicit hooks for reduced-pressure diffuse-interface face-flux "
      "predictors.");
  params.setDocString(
      "active_scalar_systems",
      "The solver system for each diffuse-interface volume-fraction transport equation.");
  params.setDocString("should_solve_active_scalars",
                      "Whether to solve the volume-fraction transport equation(s).");
  params.setDocString("active_scalar_equation_relaxation",
                      "The relaxation used for the volume-fraction transport equation(s).");
  params.setDocString(
      "active_scalar_absolute_tolerance",
      "The absolute tolerance(s) on the largest change of the volume fraction(s) between "
      "consecutive outer iterations.");
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
  params.addParam<bool>("startup_continuity_projection",
                        true,
                        "Whether to repair the initial face flux with pressure-only continuity "
                        "projections on the first time step, leaving the user-supplied reduced "
                        "pressure field untouched.");
  params.addRangeCheckedParam<unsigned int>(
      "startup_flux_corrections",
      1,
      "startup_flux_corrections>0",
      "Number of pressure-only continuity projections of the startup repair.");
  params.addParamNamesToGroup(
      "volume_fraction_subcycles volume_fraction_max_courant startup_continuity_projection "
      "startup_flux_corrections",
      "Volume Fraction Equations");
  return params;
}

ReducedPressurePIMPLESolve::ReducedPressurePIMPLESolve(Executioner & ex)
  : PIMPLESolve(ex),
    _volume_fraction_subcycles(getParam<unsigned int>("volume_fraction_subcycles")),
    _volume_fraction_max_courant(getParam<Real>("volume_fraction_max_courant")),
    _startup_continuity_projection(getParam<bool>("startup_continuity_projection")),
    _startup_flux_corrections(getParam<unsigned int>("startup_flux_corrections"))
{
}

bool
ReducedPressurePIMPLESolve::solvesVolumeFraction() const
{
  return _has_active_scalar_systems && _should_solve_active_scalars;
}

ConservativeDiffuseInterfaceVOFMULESCorrector *
ReducedPressurePIMPLESolve::diffuseInterfaceVOFCorrector(const SolverSystemName & system_name) const
{
  std::vector<UserObject *> objs;
  _problem.theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribThread>(0)
      .queryInto(objs);

  ConservativeDiffuseInterfaceVOFMULESCorrector * corrector_match = nullptr;
  for (const auto & obj : objs)
    if (auto * corrector = dynamic_cast<ConservativeDiffuseInterfaceVOFMULESCorrector *>(obj);
        corrector && corrector->systemName() == system_name)
    {
      if (corrector_match)
        mooseError("ReducedPressurePIMPLESolve found multiple "
                   "ConservativeDiffuseInterfaceVOFMULESCorrector "
                   "objects for system '",
                   system_name,
                   "'.");
      corrector_match = corrector;
    }

  return corrector_match;
}

void
ReducedPressurePIMPLESolve::initializeSolveLoop(const SolverParams & solver_params)
{
  _current_outer_iteration = 0;

  // The startup projection runs on the first step only
  if (_startup_continuity_projection && _problem.timeStep() == 1)
  {
    // Every solution state of every system starts from the initial condition, so the first
    // step's old and previous-iterate solutions are the initial condition and not zero
    for (auto * system : _momentum_systems)
      synchronizeSystemState(*system);
    synchronizeSystemState(_pressure_system);
    for (auto * system : _active_scalar_systems)
      synchronizeSystemState(*system);
    _problem.execute(EXEC_NONLINEAR);

    if (_should_solve_pressure)
      initializeStartupPressureField(solver_params);
  }

  // Only the first solve of the run seeds the transport history. Every later write happens on
  // step acceptance, so a step that is solved twice keeps the history of the last accepted one.
  if (_should_solve_pressure && solvesVolumeFraction())
    if (auto * diffuse_rc = diffuseInterfaceRC())
      diffuse_rc->seedTransportHistoryOnce();
}

void
ReducedPressurePIMPLESolve::preMomentumPressureIteration(ResidualStorage & residual_storage,
                                                         const SolverParams & solver_params)
{
  ++_current_outer_iteration;
  advanceOuterIterationHistories();

  if (!solvesVolumeFraction())
    return;

  // Set lagged_corrected_face_phi, the face flux that transports alpha, for this outer iteration.
  // On the first outer iteration it is copied from previous_timestep_corrected_face_phi, the
  // corrected flux the Rhie-Chow object stores when a step is accepted (and seeds before the
  // first solve); on later ones from the current corrected flux of this attempt. The current flux
  // is not used on the first iteration because it is derived from the face fields of the last
  // pressure correction, which a rejected step does not roll back: on a repeated step it is the
  // failed attempt's flux.
  if (auto * diffuse_rc = diffuseInterfaceRC())
    diffuse_rc->freezeLaggedCorrectedFaceFlux(_current_outer_iteration == 1);

  _problem.execute(EXEC_NONLINEAR);

  Moose::PetscSupport::petscSetOptions(_active_scalar_petsc_options, solver_params);
  const auto vf_residuals = solveVolumeFractionSystems();

  _problem.execute(EXEC_NONLINEAR);

  // The volume-fraction changes are the active-scalar residuals of this outer iteration
  for (const auto i : index_range(vf_residuals))
  {
    _console << " Volume fraction change: " << _active_scalar_system_names[i] << " " << COLOR_GREEN
             << vf_residuals[i].second << COLOR_DEFAULT << std::endl;
    residual_storage.ns_residuals[residual_storage.active_scalar_indices[i]] = vf_residuals[i];
  }
}

void
ReducedPressurePIMPLESolve::advanceOuterIterationHistories()
{
  if (_should_solve_pressure)
    advanceSystemOuterIterationHistory({&_pressure_system});

  if (_should_solve_momentum)
    // Keep the full nonlinear history on the previous outer-corrector state
    // for the whole current outer loop. The stock momentum solve shifts this
    // stack every predictor solve; here we only want to advance it
    // once per outer SIMPLE iteration.
    advanceSystemOuterIterationHistory(_momentum_systems);

  // The volume fraction's nonlinear states are not advanced: its solve sets the previous iterate
  // to the current solution and its subcycles shift the stack, so an advance here would be
  // overwritten before anything reads it. solveOneVolumeFractionSystem keeps its own copy of the
  // previous outer iterate.
}

void
ReducedPressurePIMPLESolve::advanceSystemOuterIterationHistory(
    const std::vector<LinearSystem *> & systems) const
{
  for (auto * system : systems)
  {
    unsigned int max_state = 0;
    while (system->hasSolutionState(max_state + 1, Moose::SolutionIterationType::Nonlinear))
      ++max_state;

    for (unsigned int state = max_state; state > 1; --state)
    {
      auto & nonlinear_state =
          system->solutionState(state, Moose::SolutionIterationType::Nonlinear);
      nonlinear_state = system->solutionState(state - 1, Moose::SolutionIterationType::Nonlinear);
      nonlinear_state.close();
    }

    if (max_state >= 1)
    {
      auto & previous_outer_solution =
          system->solutionState(1, Moose::SolutionIterationType::Nonlinear);
      previous_outer_solution = *(system->system().current_local_solution);
      previous_outer_solution.close();
    }
  }
}

void
ReducedPressurePIMPLESolve::assembleMomentumPredictorWithoutSolve()
{
  for (const auto system_i : index_range(_momentum_systems))
  {
    assembleMomentumPredictorOperator(system_i, /* prepare_without_solve = */ true);
    _momentum_systems[system_i]->system().update();
  }

  auto & momentum_system_0 =
      libMesh::cast_ref<LinearImplicitSystem &>(_momentum_systems[0]->system());
  auto & momentum_solver =
      libMesh::cast_ref<PetscLinearSolver<Real> &>(*momentum_system_0.get_linear_solver());
  momentum_solver.reuse_preconditioner(false);
}

ConservativeDiffuseInterfaceRhieChowMassFlux *
ReducedPressurePIMPLESolve::diffuseInterfaceRC() const
{
  return dynamic_cast<ConservativeDiffuseInterfaceRhieChowMassFlux *>(_rc_uo);
}

void
ReducedPressurePIMPLESolve::commitAcceptedTimestepTransportHistory() const
{
  if (auto * diffuse_rc = diffuseInterfaceRC())
    diffuse_rc->commitAcceptedTimestepTransportHistory();

  // The transported fluxes the momentum equation reads at the next step's start belong to the
  // accepted step as well.
  for (const auto & system_name : _active_scalar_system_names)
    if (auto * corrector = diffuseInterfaceVOFCorrector(system_name))
      corrector->commitAcceptedTimestepFluxes();
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
  // Nonlinear state 1 is the previous-iterate solution, so the loop covers it
}

void
ReducedPressurePIMPLESolve::setPreviousNewtonToCurrent(LinearSystem & system) const
{
  if (auto * previous_solution = system.solutionPreviousNewton())
  {
    *previous_solution = *(system.system().current_local_solution);
    previous_solution->close();
  }
}

void
ReducedPressurePIMPLESolve::initializeStartupPressureField(const SolverParams & solver_params)
{
  _console << "Applying startup continuity projection before PIMPLE iterations" << std::endl;

  // Honor the current reduced-pressure field, assemble the momentum predictor coefficients, and run
  // pressure-only startup continuity corrections before the first outer iteration.
  if (_momentum_systems.empty() || !_rc_uo)
    mooseError("ReducedPressurePIMPLESolve startup projection requires momentum systems and a "
               "Rhie-Chow user object.");

  // The startup assembly forms H/A, which reads the coupling pressure-gradient snapshot. The
  // outer loop takes that snapshot at its first iteration, after this point, so take it here as
  // well or the startup projection reads a snapshot that does not exist yet.
  _rc_uo->initPressureGradient();
  _rc_uo->prepareMomentumPredictor();
  assembleMomentumPredictorWithoutSolve();
  _rc_uo->initFaceMassFlux();

  _console << "Applying startup continuity corrections" << std::endl;

  for (unsigned int startup_it = 0; startup_it < _startup_flux_corrections; ++startup_it)
    (void)correctStartupContinuityOnce(solver_params);

  // Each correction restores the pressure, which initializeSolveLoop already copied into its old
  // and nonlinear states
  _problem.execute(EXEC_NONLINEAR);
}

void
ReducedPressurePIMPLESolve::postPreparePressureCorrectorState()
{
  if (auto * diffuse_rc = diffuseInterfaceRC())
  {
    diffuse_rc->updateAdditionalPressureFluxFunctors();

    // Refresh the patch velocity / target-flux state from the latest momentum
    // predictor before assembling the constrained pressure boundary gradient.
    diffuse_rc->updateVelocityBoundaryState();
    diffuse_rc->updatePressureBoundaryNormalGradients();
  }
}

unsigned int
ReducedPressurePIMPLESolve::computeVolumeFractionSubcycles() const
{
  unsigned int subcycles = _volume_fraction_subcycles;

  if (const auto * diffuse_rc = diffuseInterfaceRC())
  {
    const Real alpha_courant = diffuse_rc->maxCourant(_problem.dt());
    if (std::isfinite(alpha_courant) && alpha_courant > _volume_fraction_max_courant)
    {
      const auto required_subcycles =
          static_cast<unsigned int>(std::ceil(alpha_courant / _volume_fraction_max_courant));
      subcycles = std::max(subcycles, required_subcycles);
    }
  }

  // volume_fraction_subcycles is at least one by its range check
  return subcycles;
}

std::vector<std::pair<unsigned int, Real>>
ReducedPressurePIMPLESolve::solveVolumeFractionSystems()
{
  ProblemTimeGuard time_guard(_problem);

  std::vector<std::pair<unsigned int, Real>> residuals(_active_scalar_system_names.size(),
                                                       std::make_pair(0, 1.0));

  const Real global_dt = _problem.dt();
  const Real global_time_old = _problem.timeOld();
  const unsigned int num_subcycles = computeVolumeFractionSubcycles();
  const Real subcycle_dt = global_dt / num_subcycles;

  if (num_subcycles > _volume_fraction_subcycles)
    _console << name() << ": increasing alpha subcycles from " << _volume_fraction_subcycles
             << " to " << num_subcycles << " to keep alpha CFL <= " << _volume_fraction_max_courant
             << " at dt=" << global_dt << std::endl;

  // A subcycle advances solutionOld() to the field of the previous subcycle, but not the older
  // solution a multi-step integrator also reads, so only a one-step integrator subcycles
  // consistently
  if (num_subcycles > 1)
    for (auto * system : _active_scalar_systems)
      for (const auto & time_integrator : system->getTimeIntegrators())
        if (time_integrator->numStatesRequired() > 1)
          mooseError("Volume-fraction subcycling (",
                     num_subcycles,
                     " subcycles) requires a one-step time integrator such as ImplicitEuler; ",
                     time_integrator->type(),
                     " also reads the older solution, which the subcycles do not advance.");

  for (const auto i : index_range(_active_scalar_system_names))
    residuals[i] =
        solveOneVolumeFractionSystem(i, num_subcycles, subcycle_dt, global_dt, global_time_old);

  // The transport state is published at the global time, not the last subcycle's.
  time_guard.restore();
  for (const auto & system : _active_scalar_systems)
    system->computeGradients();
  for (const auto & system_name : _active_scalar_system_names)
    if (auto * corrector = diffuseInterfaceVOFCorrector(system_name))
      corrector->refreshPublishedRhoPhi();

  return residuals;
}

std::pair<unsigned int, Real>
ReducedPressurePIMPLESolve::solveOneVolumeFractionSystem(const unsigned int i,
                                                         const unsigned int num_subcycles,
                                                         const Real subcycle_dt,
                                                         const Real global_dt,
                                                         const Real global_time_old)
{
  auto * system = _active_scalar_systems[i];
  // The previous outer iterate; the nonlinear state stack cannot hold it because the subcycles
  // reuse that slot below.
  const auto previous_outer_iterate = system->system().current_local_solution->clone();
  // Every outer iteration transports alpha afresh from the time-step-old field with the flux of
  // this iteration, rather than correcting the previous iterate: the subcycles below overwrite
  // solutionOld() as they go, and saveOldSolutions/restoreOldSolutions bracket that.
  system->saveOldSolutions();

  // solutionOld() stays the true time-step-old alpha for the whole outer loop (restored below).
  // solutionPreviousNewton(), nonlinear state 1, is the subcycle relaxation state, which the
  // subcycles overwrite, hence the copy of the previous outer iterate above.
  setPreviousNewtonToCurrent(*system);

  // Find this system's ConservativeDiffuseInterfaceVOFMULESCorrector, the user object that turns
  // the upwind alpha solution into a bounded one and publishes the face fluxes
  // alpha_phi_limited and rho_phi. The executioner calls it itself, once after each subcycle's
  // upwind solve below, because no execute_on flag runs a user object at that point.
  // resetSubcycleFluxes() zeroes the published fluxes; every subcycle then adds its share.
  auto * corrector = diffuseInterfaceVOFCorrector(_active_scalar_system_names[i]);
  if (corrector)
    corrector->resetSubcycleFluxes();
  else
    mooseError("ReducedPressurePIMPLESolve requires a "
               "ConservativeDiffuseInterfaceVOFMULESCorrector for volume-fraction system '",
               _active_scalar_system_names[i],
               "'.");

  std::pair<unsigned int, Real> residual{0, 1.0};
  for (const auto subcycle : make_range(num_subcycles))
    residual = runOneVolumeFractionSubcycle(
        i, *system, *corrector, subcycle, subcycle_dt, global_dt, global_time_old);

  system->restoreOldSolutions();
  setPreviousNewtonToCurrent(*system);

  // The limited volume fraction never satisfies the upwind system it was solved from, so that
  // linear residual stays at the size of the antidiffusive correction however far the outer loop
  // goes. What converges is the field, so the step is judged on its largest change instead.
  const auto & current = *system->system().current_local_solution;
  Real change = 0.0;
  for (const auto dof :
       make_range(system->solution().first_local_index(), system->solution().last_local_index()))
    change = std::max(change, std::abs(current(dof) - (*previous_outer_iterate)(dof)));
  _communicator.max(change);
  residual.second = change;

  return residual;
}

/**
 * Advances volume-fraction system \p i by one subcycle, from t^n + s dt_s to t^n + (s + 1) dt_s
 * for subcycle s of step size dt_s = \p subcycle_dt. It sets the problem time to the subcycle's,
 * solves the implicit upwind (donor) alpha equation with the frozen face flux, and then lets the
 * MULES corrector add the limited antidiffusive correction to that solution and accumulate the
 * subcycle's share dt_s / dt of the published fluxes. solveVolumeFractionSystems() restores the
 * global time afterwards.
 * Returns the linear iteration count and the normalized initial residual of the donor solve.
 */
std::pair<unsigned int, Real>
ReducedPressurePIMPLESolve::runOneVolumeFractionSubcycle(
    const unsigned int i,
    LinearSystem & system,
    ConservativeDiffuseInterfaceVOFMULESCorrector & corrector,
    const unsigned int subcycle,
    const Real subcycle_dt,
    const Real global_dt,
    const Real global_time_old)
{
  _problem.dt() = subcycle_dt;
  _problem.timeOld() = global_time_old + subcycle * subcycle_dt;
  _problem.time() = _problem.timeOld() + subcycle_dt;

  // Every subcycle but the first starts from the field the previous one produced
  if (subcycle > 0)
  {
    system.solutionOld() = *(system.system().current_local_solution);
    system.solutionOld().close();
    setPreviousNewtonToCurrent(system);
  }

  _problem.execute(EXEC_NONLINEAR);

  // This path bounds alpha through the limited face fluxes instead of projecting the solved field.
  // Do not apply the inherited scalar lower limiter here.
  const auto residual = solveAdvectedSystem(
      _active_scalar_system_numbers[i],
      system,
      _active_scalar_equation_relaxation[i],
      _active_scalar_linear_control,
      _active_scalar_l_abs_tol,
      (_active_scalar_pc_solve_counter % _active_scalar_pc_recompute_frequency) != 0,
      /* field_relaxation = */ 1.0,
      /* min_value_limiter = */ std::numeric_limits<Real>::min());
  ++_active_scalar_pc_solve_counter;
  system.computeGradients();

  // The donor solve weighted the new solution by w0/dt, the leading coefficient of the time
  // integrator (w0 = 1 for implicit Euler, 3/2 for BDF2 at constant step), so the explicit
  // correction that completes it advances alpha by dt/w0 per unit face flux
  const auto & time_integrators = system.getTimeIntegrators();
  mooseAssert(time_integrators.size() == 1, "One time integrator per volume-fraction system");
  const Real effective_dt = 1.0 / time_integrators.front()->timeDerivativeMatrixContribution(1.0);
  corrector.applyCorrection(effective_dt, subcycle_dt / global_dt);

  return residual;
}

std::pair<unsigned int, Real>
ReducedPressurePIMPLESolve::correctStartupContinuityOnce(const SolverParams & solver_params)
{
  PressureStateGuard pressure_state(_pressure_system);
  DiffuseInterfaceStartupProjectionGuard projection_scope(diffuseInterfaceRC());

  // The startup repair is deliberately a reduced corrector: it solves continuity and rebuilds the
  // face flux, but does not relax the pressure or publish a corrected velocity, because the guard
  // above restores the physical p_rgh field immediately afterwards. So it runs the first stages of
  // the corrector rather than the whole pipeline.
  _rc_uo->computeHbyA(_print_fields);
  postPreparePressureCorrectorState();

  Moose::PetscSupport::petscSetOptions(_pressure_petsc_options, solver_params);

  const auto residuals = solvePressureCorrector();
  _rc_uo->computeFaceMassFlux();

  // Restore the user/equilibrium startup reduced-pressure field. Startup
  // continuity cleanup should repair phi, not overwrite the physical p_rgh field
  // before the first real pressure equation.
  pressure_state.restore();
  _pressure_system.computeGradients();

  return residuals;
}

void
ReducedPressurePIMPLESolve::postPublishPressureCorrectedState()
{
  if (auto * diffuse_rc = diffuseInterfaceRC())
    diffuse_rc->updateVelocityBoundaryState();
}
