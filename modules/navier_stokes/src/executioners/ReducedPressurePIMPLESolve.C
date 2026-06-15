//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReducedPressurePIMPLESolve.h"

#include "FEProblem.h"
#include "LinearSystem.h"
#include "MooseApp.h"
#include "SegregatedSolverUtils.h"
#include "ConservativeSharpInterfaceRhieChowMassFlux.h"
#include "ConservativeSharpInterfaceVOFMULESCorrector.h"
#include "TheWarehouse.h"
#include "libmesh/petsc_linear_solver.h"

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
  params.addParam<std::vector<SolverSystemName>>(
      "volume_fraction_systems",
      {},
      "The solver system for each sharp-interface volume-fraction transport equation.");
  params.addParam<std::vector<Real>>(
      "volume_fraction_equation_relaxation",
      std::vector<Real>(),
      "The relaxation used for the volume-fraction transport equations.");
  params.addParam<MultiMooseEnum>("volume_fraction_petsc_options",
                                  Moose::PetscSupport::getCommonPetscFlags(),
                                  "Singleton PETSc options for the volume-fraction equation(s)");
  params.addParam<MultiMooseEnum>(
      "volume_fraction_petsc_options_iname",
      Moose::PetscSupport::getCommonPetscKeys(),
      "Names of PETSc name/value pairs for the volume-fraction equation(s)");
  params.addParam<std::vector<std::string>>(
      "volume_fraction_petsc_options_value",
      "Values of PETSc name/value pairs for the volume-fraction equation(s)");
  params.addParam<std::vector<Real>>(
      "volume_fraction_absolute_tolerance",
      std::vector<Real>(),
      "The absolute tolerance(s) on the normalized residual(s) of the volume-fraction "
      "equation(s).");
  params.addRangeCheckedParam<Real>(
      "volume_fraction_l_tol",
      1e-5,
      "0.0<=volume_fraction_l_tol & volume_fraction_l_tol<1.0",
      "The relative tolerance on the normalized residual in the linear solver of the "
      "volume-fraction equation(s).");
  params.addRangeCheckedParam<Real>(
      "volume_fraction_l_abs_tol",
      1e-10,
      "0.0<volume_fraction_l_abs_tol",
      "The absolute tolerance on the normalized residual in the linear solver of the "
      "volume-fraction equation(s).");
  params.addParam<unsigned int>(
      "volume_fraction_l_max_its",
      10000,
      "The maximum allowed iterations in the linear solver of the volume-fraction equation(s).");
  params.addParam<bool>("should_solve_volume_fractions",
                        true,
                        "Whether we should solve the volume-fraction equation(s).");
  params.addParam<Real>("volume_fraction_min_value",
                        0.0,
                        "Lower clamp applied to transported volume-fraction fields after each "
                        "solve.");
  params.addParam<Real>("volume_fraction_max_value",
                        1.0,
                        "Upper clamp applied to transported volume-fraction fields after each "
                        "solve.");
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
  params.addParam<bool>(
      "adjust_momentum_pressure_time_step",
      false,
      "Whether the ReducedPressurePIMPLE executioner should shrink the current timestep so the "
      "Rhie-Chow face-flux Courant number stays below momentum_pressure_max_courant before the "
      "outer PIMPLE loop.");
  params.addRangeCheckedParam<Real>(
      "momentum_pressure_max_courant",
      1.0,
      "momentum_pressure_max_courant>0",
      "Maximum allowed momentum/pressure face-flux Courant number when "
      "adjust_momentum_pressure_time_step=true.");
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
      "volume_fraction_systems volume_fraction_equation_relaxation volume_fraction_petsc_options "
      "volume_fraction_petsc_options_iname volume_fraction_petsc_options_value "
      "volume_fraction_absolute_tolerance volume_fraction_l_tol volume_fraction_l_abs_tol "
      "volume_fraction_l_max_its should_solve_volume_fractions volume_fraction_min_value "
      "volume_fraction_max_value volume_fraction_subcycles volume_fraction_max_courant "
      "adjust_momentum_pressure_time_step momentum_pressure_max_courant "
      "startup_pressure_initialization startup_flux_corrections "
      "num_pressure_nonorthogonal_correctors n_nonorthogonal_correctors",
      "Volume Fraction Equations");
  return params;
}

ReducedPressurePIMPLESolve::ReducedPressurePIMPLESolve(Executioner & ex)
  : PIMPLESolve(ex),
    _volume_fraction_system_names(
        getParam<std::vector<SolverSystemName>>("volume_fraction_systems")),
    _has_volume_fraction_systems(!_volume_fraction_system_names.empty()),
    _should_solve_volume_fractions(getParam<bool>("should_solve_volume_fractions")),
    _volume_fraction_equation_relaxation(
        getParam<std::vector<Real>>("volume_fraction_equation_relaxation")),
    _volume_fraction_l_abs_tol(getParam<Real>("volume_fraction_l_abs_tol")),
    _volume_fraction_absolute_tolerance(
        getParam<std::vector<Real>>("volume_fraction_absolute_tolerance")),
    _volume_fraction_min_value(getParam<Real>("volume_fraction_min_value")),
    _volume_fraction_max_value(getParam<Real>("volume_fraction_max_value")),
    _volume_fraction_subcycles(getParam<unsigned int>("volume_fraction_subcycles")),
    _volume_fraction_max_courant(getParam<Real>("volume_fraction_max_courant")),
    _adjust_momentum_pressure_time_step(getParam<bool>("adjust_momentum_pressure_time_step")),
    _momentum_pressure_max_courant(getParam<Real>("momentum_pressure_max_courant")),
    _startup_flux_corrections(getParam<unsigned int>("startup_flux_corrections"))
{
  _startup_pressure_initialization =
      getParam<MooseEnum>("startup_pressure_initialization").operator std::string();

  if (_pin_pressure)
    paramError("pin_pressure",
               "ReducedPressurePIMPLE supports pressure boundary-condition constraints only; "
               "pressure pinning is not supported.");

  if (_volume_fraction_min_value > _volume_fraction_max_value)
    paramError("volume_fraction_max_value",
               "volume_fraction_max_value must be >= volume_fraction_min_value.");

  if (_has_volume_fraction_systems)
  {
    if (_volume_fraction_equation_relaxation.size() != _volume_fraction_system_names.size())
      paramError("volume_fraction_equation_relaxation",
                 "Should be the same size as the number of volume-fraction systems");
    if (_volume_fraction_absolute_tolerance.size() != _volume_fraction_system_names.size())
      paramError("volume_fraction_absolute_tolerance",
                 "Should be the same size as the number of volume-fraction systems");

    for (const auto system_i : index_range(_volume_fraction_system_names))
    {
      _volume_fraction_system_numbers.push_back(
          _problem.linearSysNum(_volume_fraction_system_names[system_i]));
      _volume_fraction_systems.push_back(
          &_problem.getLinearSystem(_volume_fraction_system_numbers[system_i]));
      if (_should_solve_volume_fractions)
        _systems_to_solve.push_back(_volume_fraction_systems.back());
    }

    const auto & volume_fraction_petsc_options =
        getParam<MultiMooseEnum>("volume_fraction_petsc_options");
    const auto & volume_fraction_petsc_pair_options = getParam<MooseEnumItem, std::string>(
        "volume_fraction_petsc_options_iname", "volume_fraction_petsc_options_value");
    Moose::PetscSupport::addPetscFlagsToPetscOptions(
        volume_fraction_petsc_options, "", *this, _volume_fraction_petsc_options);
    Moose::PetscSupport::addPetscPairsToPetscOptions(volume_fraction_petsc_pair_options,
                                                     _problem.mesh().dimension(),
                                                     "",
                                                     *this,
                                                     _volume_fraction_petsc_options);

    _volume_fraction_linear_control.real_valued_data["rel_tol"] =
        getParam<Real>("volume_fraction_l_tol");
    _volume_fraction_linear_control.real_valued_data["abs_tol"] =
        getParam<Real>("volume_fraction_l_abs_tol");
    _volume_fraction_linear_control.int_valued_data["max_its"] =
        getParam<unsigned int>("volume_fraction_l_max_its");
  }
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

Real
ReducedPressurePIMPLESolve::momentumPressureCourant(const Real dt) const
{
  if (!_rc_uo || dt <= 0.0)
    return 0.0;

  return _rc_uo->maxCourant(dt);
}

Real
ReducedPressurePIMPLESolve::constrainedMomentumPressureDT(const Real dt) const
{
  if (!_adjust_momentum_pressure_time_step || dt <= 0.0)
    return dt;

  const Real courant = momentumPressureCourant(dt);
  if (!std::isfinite(courant) || courant <= _momentum_pressure_max_courant)
    return dt;

  const Real adjusted_dt = dt * _momentum_pressure_max_courant / courant;
  if (!(adjusted_dt > 0.0) || adjusted_dt >= dt)
    return dt;

  return adjusted_dt;
}

bool
ReducedPressurePIMPLESolve::solve()
{
  if (!_problem.shouldSolve())
    return true;

  if (auto * sharp_rc = sharpInterfaceRC())
    sharp_rc->setSuppressExplicitHydrostaticPressureFlux(false);

  SolverParams solver_params;
  solver_params._type = Moose::SolveType::ST_LINEAR;
  solver_params._line_search = Moose::LineSearchType::LS_NONE;

  unsigned int simple_iteration_counter = 0;

  ResidualStorage residual_storage = setupResidualStorage();
  auto & ns_residuals = residual_storage.ns_residuals;
  auto & ns_abs_tols = residual_storage.ns_abs_tols;
  const auto & momentum_indices = residual_storage.momentum_indices;
  const auto pressure_index = residual_storage.pressure_index;
  const auto energy_index = residual_storage.energy_index;
  const auto solid_energy_index = residual_storage.solid_energy_index;
  const auto & active_scalar_indices = residual_storage.active_scalar_indices;
  const auto & turbulence_indices = residual_storage.turbulence_indices;
  const auto & pm_radiation_indices = residual_storage.pm_radiation_indices;

  std::vector<std::size_t> volume_fraction_indices;
  if (_has_volume_fraction_systems && _should_solve_volume_fractions)
    for (const auto i : index_range(_volume_fraction_system_names))
    {
      volume_fraction_indices.push_back(ns_residuals.size());
      ns_residuals.push_back(std::make_pair(0, 1.0));
      ns_abs_tols.push_back(_volume_fraction_absolute_tolerance[i]);
    }

  bool converged = residual_storage.converged && volume_fraction_indices.empty();

  if (_cht.enabled() && _should_solve_energy)
    _cht.initializeCHTCouplingFields();

  if (_has_volume_fraction_systems && _should_solve_volume_fractions)
    if (auto * sharp_rc = sharpInterfaceRC())
      sharp_rc->clearVOFTransportState();

  if (startupPressureInitializationEnabled() && _problem.timeStep() == 1)
  {
    for (auto * system : _momentum_systems)
      synchronizeSystemState(*system);
    synchronizeSystemState(_pressure_system);
    for (auto * system : _volume_fraction_systems)
      synchronizeSystemState(*system);

    _problem.execute(EXEC_NONLINEAR);
  }

  if (_should_solve_pressure)
  {
    initializeStartupPressureField(solver_params);
    if (_has_volume_fraction_systems && _should_solve_volume_fractions)
      if (auto * sharp_rc = sharpInterfaceRC())
        sharp_rc->commitAcceptedTimestepTransportHistory();
  }

  while (simple_iteration_counter < _num_iterations && !converged)
  {
    simple_iteration_counter++;
    _current_outer_iteration = simple_iteration_counter;

    if (_should_solve_pressure)
      advancePressureOuterIterationHistory();

    if (_should_solve_momentum)
      // Keep the full nonlinear history on the previous outer-corrector state
      // for the whole current outer loop. The stock momentum solve shifts this
      // stack every predictor solve; here we only want to advance it
      // once per outer SIMPLE iteration.
      advanceMomentumOuterIterationHistory();

    // Do the alpha subcycling and mixture/rhoPhi refresh inside every outer correction, just before
    // the momentum-pressure coupling work. This keeps rhoPhi consistent with the outer-corrector
    // state instead of freezing one alpha update for a later sequence of momentum-pressure
    // repredictions.
    if (_has_volume_fraction_systems && _should_solve_volume_fractions)
    {
      // Keep the true timestep-old alpha in solutionOld(), but advance the
      // nonlinear-state stack once per outer iteration so we have a separate
      // previous-outer iterate available.
      advanceVolumeFractionOuterIterationHistory();

      const bool use_previous_timestep_transport_flux =
          _problem.timeStep() == 1 && _current_outer_iteration == 1;
      if (auto * sharp_rc = sharpInterfaceRC())
      {
        sharp_rc->clearVOFTransportState();
        sharp_rc->freezeVOFTransportState(use_previous_timestep_transport_flux);
      }

      _problem.execute(EXEC_NONLINEAR);
      Moose::PetscSupport::petscSetOptions(_volume_fraction_petsc_options, solver_params);
      const auto vf_residuals = solveVolumeFractionSystems(solver_params);

      if (auto * sharp_rc = sharpInterfaceRC())
        sharp_rc->adoptPublishedVOFTransportState();

      _problem.execute(EXEC_NONLINEAR);
      for (const auto i : index_range(vf_residuals))
        ns_residuals[volume_fraction_indices[i]] = vf_residuals[i];
    }

    if (_should_solve_momentum)
      Moose::PetscSupport::petscSetOptions(_momentum_petsc_options, solver_params);

    if (_should_solve_pressure && simple_iteration_counter == 1)
      _pressure_system.computeGradients();

    _console << "Iteration " << simple_iteration_counter << " Residual norms:" << std::endl;

    if (_should_solve_momentum)
    {
      auto momentum_residual = solveMomentumPredictor();
      for (const auto system_i : index_range(momentum_residual))
        ns_residuals[momentum_indices[system_i]] = momentum_residual[system_i];
    }
    else if (_should_solve_pressure && !_momentum_systems.empty() && _rc_uo)
      assembleMomentumPredictorOnly();

    if (_should_solve_pressure)
      ns_residuals[pressure_index] = correctVelocity(true, true, solver_params);

    if (_has_energy_system && _should_solve_energy)
    {
      _cht.resetCHTConvergence();
      while (!_cht.converged())
      {
        if (_cht.enabled())
          _cht.updateCHTBoundaryCouplingFields(NS::CHTSide::FLUID);

        Moose::PetscSupport::petscSetOptions(_energy_petsc_options, solver_params);
        ns_residuals[energy_index] = solveAdvectedSystem(_energy_sys_number,
                                                         *_energy_system,
                                                         _energy_equation_relaxation,
                                                         _energy_linear_control,
                                                         _energy_l_abs_tol);

        if (_has_pm_radiation_systems && _should_solve_pm_radiation)
        {
          Moose::PetscSupport::petscSetOptions(_pm_radiation_petsc_options, solver_params);
          for (const auto i : index_range(_pm_radiation_system_names))
            ns_residuals[pm_radiation_indices[i]] =
                solveAdvectedSystem(_pm_radiation_system_numbers[i],
                                    *_pm_radiation_systems[i],
                                    _pm_radiation_equation_relaxation[i],
                                    _pm_radiation_linear_control,
                                    _pm_radiation_l_abs_tol);
        }

        if (_has_solid_energy_system && _should_solve_solid_energy)
        {
          if (_cht.enabled())
          {
            _energy_system->computeGradients();
            _cht.updateCHTBoundaryCouplingFields(NS::CHTSide::SOLID);
          }

          Moose::PetscSupport::petscSetOptions(_solid_energy_petsc_options, solver_params);
          ns_residuals[solid_energy_index] = solveSolidEnergy();

          if (_cht.enabled())
            _solid_energy_system->computeGradients();
        }

        if (_cht.enabled())
        {
          _cht.sumIntegratedFluxes();
          _cht.printIntegratedFluxes();
        }

        _cht.incrementCHTIterators();
      }
      if (_cht.enabled())
        _cht.resetIntegratedFluxes();
    }

    if (_has_active_scalar_systems && _should_solve_active_scalars)
    {
      _problem.execute(EXEC_NONLINEAR);
      Moose::PetscSupport::petscSetOptions(_active_scalar_petsc_options, solver_params);
      for (const auto i : index_range(_active_scalar_system_names))
        ns_residuals[active_scalar_indices[i]] =
            solveAdvectedSystem(_active_scalar_system_numbers[i],
                                *_active_scalar_systems[i],
                                _active_scalar_equation_relaxation[i],
                                _active_scalar_linear_control,
                                _active_scalar_l_abs_tol);
    }

    if (_has_turbulence_systems && _should_solve_turbulence)
    {
      Moose::PetscSupport::petscSetOptions(_turbulence_petsc_options, solver_params);
      for (const auto i : index_range(_turbulence_system_names))
        ns_residuals[turbulence_indices[i]] =
            solveAdvectedSystem(_turbulence_system_numbers[i],
                                *_turbulence_systems[i],
                                _turbulence_equation_relaxation[i],
                                _turbulence_linear_control,
                                _turbulence_l_abs_tol,
                                _turbulence_field_relaxation[i],
                                _turbulence_field_min_limit[i]);
    }

    _problem.execute(EXEC_NONLINEAR);

    converged = NS::FV::converged(ns_residuals, ns_abs_tols);
  }

  if (_has_passive_scalar_systems && _should_solve_passive_scalars &&
      (converged || _continue_on_max_its))
  {
    bool passive_scalar_converged = false;
    unsigned int ps_iteration_counter = 0;

    _console << "Passive scalar iteration " << ps_iteration_counter
             << " Residual norms:" << std::endl;

    while (ps_iteration_counter < _num_iterations && !passive_scalar_converged)
    {
      ps_iteration_counter++;
      std::vector<std::pair<unsigned int, Real>> scalar_residuals(
          _passive_scalar_system_names.size(), std::make_pair(0, 1.0));
      std::vector<Real> scalar_abs_tols;
      for (const auto scalar_tol : _passive_scalar_absolute_tolerance)
        scalar_abs_tols.push_back(scalar_tol);

      Moose::PetscSupport::petscSetOptions(_passive_scalar_petsc_options, solver_params);
      for (const auto i : index_range(_passive_scalar_system_names))
        scalar_residuals[i] = solveAdvectedSystem(_passive_scalar_system_numbers[i],
                                                  *_passive_scalar_systems[i],
                                                  _passive_scalar_equation_relaxation[i],
                                                  _passive_scalar_linear_control,
                                                  _passive_scalar_l_abs_tol);

      passive_scalar_converged = NS::FV::converged(scalar_residuals, scalar_abs_tols);
    }

    converged = passive_scalar_converged && converged;
  }

  converged = _continue_on_max_its ? true : converged;

  if (auto * sharp_rc = sharpInterfaceRC())
    sharp_rc->setSuppressExplicitHydrostaticPressureFlux(false);

  return converged;
}

std::vector<std::pair<unsigned int, Real>>
ReducedPressurePIMPLESolve::solveMomentumPredictor()
{
  auto nonlinear_state_snapshots = snapshotMomentumNonlinearSolutionStates();
  auto residuals = LinearAssemblySegregatedSolve::solveMomentumPredictor();
  restoreMomentumNonlinearSolutionStates(nonlinear_state_snapshots);

  return residuals;
}

void
ReducedPressurePIMPLESolve::addMomentumPredictorExplicitForcing(const unsigned int system_i,
                                                                NumericVector<Number> & rhs)
{
  if (auto * sharp_rc = sharpInterfaceRC(); sharp_rc && sharp_rc->splitMomentumPredictorOperator())
    sharp_rc->addMomentumPredictorExplicitForcing(system_i, rhs);
}

void
ReducedPressurePIMPLESolve::addMomentumPredictorBodyForceForcing(const unsigned int system_i,
                                                                 NumericVector<Number> & rhs)
{
  if (auto * sharp_rc = sharpInterfaceRC(); sharp_rc && sharp_rc->splitMomentumPredictorOperator())
    sharp_rc->addMomentumPredictorBodyForceForcing(system_i, rhs);
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
ReducedPressurePIMPLESolve::assembleMomentumPredictorOnly()
{
  if (_momentum_systems.empty())
    return;

  if (auto * sharp_rc = sharpInterfaceRC())
    sharp_rc->updateContinuityErrorField();

  if (_rc_uo)
    _rc_uo->clearMomentumPredictorOperatorCache();

  LinearImplicitSystem & momentum_system_0 =
      libMesh::cast_ref<LinearImplicitSystem &>(_momentum_systems[0]->system());
  PetscLinearSolver<Real> & momentum_solver =
      libMesh::cast_ref<PetscLinearSolver<Real> &>(*momentum_system_0.get_linear_solver());

  for (const auto system_i : index_range(_momentum_systems))
  {
    _problem.setCurrentLinearSystem(_momentum_system_numbers[system_i]);

    LinearImplicitSystem & momentum_system =
        libMesh::cast_ref<LinearImplicitSystem &>(_momentum_systems[system_i]->system());
    momentum_system.update();
    NumericVector<Number> & solution = *(momentum_system.solution);
    NumericVector<Number> & rhs = *(momentum_system.rhs);
    SparseMatrix<Number> & mmat = *(momentum_system.matrix);

    auto diff_diagonal = solution.zero_clone();
    std::unique_ptr<NumericVector<Number>> predictor_diagonal_raw;
    std::unique_ptr<NumericVector<Number>> predictor_rhs_base;
    std::unique_ptr<NumericVector<Number>> predictor_explicit_force;
    std::unique_ptr<NumericVector<Number>> predictor_body_force;

    // Assemble and relax the momentum predictor exactly as in the main SIMPLE loop,
    // but stop before the linear solve so startup pressure correction can reuse the
    // same diagonal / HbyA operator without advancing momentum.
    momentum_system.current_local_solution->close();
    solution.close();
    _momentum_systems[system_i]->solutionOld().close();
    if (auto * previous_newton = _momentum_systems[system_i]->solutionPreviousNewton())
      previous_newton->close();
    rhs.close();

    if (_rc_uo && _rc_uo->splitMomentumPredictorOperator())
      _rc_uo->beginFVSplitMomentumPredictorOperatorAssembly(system_i);

    try
    {
      _problem.computeLinearSystemSys(momentum_system, mmat, rhs, /*compute_grads*/ true);
    }
    catch (...)
    {
      if (_rc_uo && _rc_uo->splitMomentumPredictorOperator())
        _rc_uo->clearMomentumPredictorOperatorCache();
      throw;
    }
    rhs.close();

    if (_rc_uo && _rc_uo->splitMomentumPredictorOperator())
      _rc_uo->completeFVSplitMomentumPredictorOperatorAssembly(
          system_i, _momentum_equation_relaxation, _force_momentum_diagonal_dominance);

    applyMomentumEquationRelaxation(mmat, rhs, solution, *diff_diagonal);

    if (_rc_uo && !_rc_uo->splitMomentumPredictorOperator())
    {
      predictor_diagonal_raw = solution.zero_clone();
      predictor_diagonal_raw->close();
      auto * petsc_mat = dynamic_cast<PetscMatrix<Number> *>(momentum_system.matrix);
      mooseAssert(petsc_mat,
                  "ReducedPressurePIMPLESolve startup predictor caching requires PETSc matrices.");
      petsc_mat->get_diagonal(*predictor_diagonal_raw);
      predictor_diagonal_raw->close();
      _rc_uo->cacheStartupPredictorDiagonal(system_i, *predictor_diagonal_raw);
    }

    if (_rc_uo && _rc_uo->splitMomentumPredictorOperator())
    {
      predictor_rhs_base = rhs.clone();
      *predictor_rhs_base = rhs;
      predictor_rhs_base->close();

      predictor_body_force = rhs.clone();
      predictor_body_force->zero();
      predictor_body_force->close();
      addMomentumPredictorBodyForceForcing(system_i, *predictor_body_force);
      predictor_body_force->close();

      addMomentumPredictorExplicitForcing(system_i, rhs);

      predictor_explicit_force = rhs.clone();
      *predictor_explicit_force = rhs;
      predictor_explicit_force->add(-1.0, *predictor_rhs_base);
      predictor_explicit_force->close();

      _rc_uo->setMomentumPredictorForcing(
          system_i, predictor_explicit_force.get(), predictor_body_force.get());
    }

    momentum_system.update();

    if (_rc_uo && !_rc_uo->splitMomentumPredictorOperator())
      _rc_uo->cacheMomentumPredictorOperator(system_i,
                                             predictor_rhs_base.get(),
                                             predictor_explicit_force.get(),
                                             predictor_body_force.get());
  }

  momentum_solver.reuse_preconditioner(false);
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
  if (!_momentum_systems.empty() && _rc_uo)
  {
    assembleMomentumPredictorOnly();
    _rc_uo->initFaceMassFlux();
    performStartupContinuityCorrections(solver_params);
    synchronizeSystemState(_pressure_system);
  }
  else
  {
    _console << "Falling back to startup pressure-velocity correction because predictor-only "
                "startup projection is unavailable"
             << std::endl;
    Moose::PetscSupport::petscSetOptions(_pressure_petsc_options, solver_params);
    for (const auto startup_it : make_range(_startup_flux_corrections))
    {
      (void)startup_it;
      correctVelocityOnce(true, true, solver_params);
    }
  }
  _problem.execute(EXEC_NONLINEAR);
}

void
ReducedPressurePIMPLESolve::performStartupContinuityCorrections(const SolverParams & solver_params)
{
  if (!startupPressureInitializationEnabled() || _problem.timeStep() != 1)
    return;

  if (!_should_solve_pressure || _momentum_systems.empty() || !_rc_uo)
    return;

  _console << "Applying startup continuity corrections" << std::endl;
  Moose::PetscSupport::petscSetOptions(_pressure_petsc_options, solver_params);

  for (const auto startup_it : make_range(_startup_flux_corrections))
  {
    (void)startup_it;
    (void)correctStartupContinuityOnce(true, true, solver_params);
  }

  _problem.execute(EXEC_NONLINEAR);
}

void
ReducedPressurePIMPLESolve::preparePressureCorrectorState(const bool subtract_updated_pressure)
{
  _rc_uo->computeHbyA(subtract_updated_pressure, _print_fields);

  if (auto * sharp_rc = sharpInterfaceRC())
    sharp_rc->updateAdditionalPressureFluxFunctors(subtract_updated_pressure, _print_fields);

  // Refresh the patch velocity / target-flux state from the latest momentum
  // predictor before assembling the constrained pressure boundary gradient.
  _rc_uo->updateVelocityBoundaryState();

  _rc_uo->updatePressureBoundaryNormalGradients(/* apply_pressure_flux_adjustment = */ false);
}

void
ReducedPressurePIMPLESolve::reconstructPressureCoupledStateFromCurrentPressure(
    const bool subtract_updated_pressure)
{
  preparePressureCorrectorState(subtract_updated_pressure);

  _rc_uo->computeFaceMassFlux();

  if (auto * sharp_rc = sharpInterfaceRC())
    sharp_rc->applyAdditionalFaceMassFluxCorrection();

  _rc_uo->computeCellVelocity();

  _rc_uo->updateVelocityBoundaryState();
}

void
ReducedPressurePIMPLESolve::advanceMomentumOuterIterationHistory() const
{
  advanceSystemOuterIterationHistory(_momentum_systems);
}

void
ReducedPressurePIMPLESolve::advanceVolumeFractionOuterIterationHistory() const
{
  advanceSystemOuterIterationHistory(_volume_fraction_systems);
}

unsigned int
ReducedPressurePIMPLESolve::computeVolumeFractionSubcycles() const
{
  unsigned int subcycles = _volume_fraction_subcycles;

  if (const auto * sharp_rc = sharpInterfaceRC())
  {
    const Real alpha_courant = sharp_rc->maxVolumeFractionCourant(_problem.dt());
    if (std::isfinite(alpha_courant) && alpha_courant > _volume_fraction_max_courant)
    {
      const auto required_subcycles =
          static_cast<unsigned int>(std::ceil(alpha_courant / _volume_fraction_max_courant));
      subcycles = std::max(subcycles, std::max(required_subcycles, 1u));
    }
  }

  return std::max(subcycles, 1u);
}

void
ReducedPressurePIMPLESolve::restoreMomentumNonlinearSolutionStates(
    const NonlinearSolutionStateSnapshots & snapshots) const
{
  mooseAssert(snapshots.size() == _momentum_systems.size(),
              "Momentum nonlinear-state snapshots must match the number of momentum systems.");

  for (const auto system_i : index_range(_momentum_systems))
    for (const auto state_i : index_range(snapshots[system_i]))
    {
      auto & nonlinear_state = _momentum_systems[system_i]->solutionState(
          state_i + 1, Moose::SolutionIterationType::Nonlinear);
      nonlinear_state = *snapshots[system_i][state_i];
      nonlinear_state.close();
    }
}

ReducedPressurePIMPLESolve::NonlinearSolutionStateSnapshots
ReducedPressurePIMPLESolve::snapshotMomentumNonlinearSolutionStates() const
{
  NonlinearSolutionStateSnapshots snapshots(_momentum_systems.size());

  for (const auto system_i : index_range(_momentum_systems))
    for (unsigned int state = 1; _momentum_systems[system_i]->hasSolutionState(
             state, Moose::SolutionIterationType::Nonlinear);
         ++state)
    {
      const auto & nonlinear_state = _momentum_systems[system_i]->solutionState(
          state, Moose::SolutionIterationType::Nonlinear);
      auto snapshot = nonlinear_state.zero_clone();
      *snapshot = nonlinear_state;
      snapshot->close();
      snapshots[system_i].push_back(std::move(snapshot));
    }

  return snapshots;
}

std::vector<std::pair<unsigned int, Real>>
ReducedPressurePIMPLESolve::solveVolumeFractionSystems(const SolverParams & /*solver_params*/)
{
  std::vector<std::pair<unsigned int, Real>> residuals(_volume_fraction_system_names.size(),
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

  for (const auto i : index_range(_volume_fraction_system_names))
  {
    auto * system = _volume_fraction_systems[i];
    auto saved_old_solution = system->solutionOld().zero_clone();
    *saved_old_solution = system->solutionOld();
    saved_old_solution->close();

    // solutionOld() must stay as the true timestep-old alpha for the whole
    // outer loop. solutionPreviousNewton() is only the local/subcycle field-
    // relaxation state, while the previous-outer iterate now lives in the
    // nonlinear solution-state stack advanced at outer-loop entry.
    if (auto * previous_solution = system->solutionPreviousNewton())
    {
      *previous_solution = *(system->system().current_local_solution);
      previous_solution->close();
    }

    auto * corrector = sharpInterfaceVOFCorrector(_volume_fraction_system_names[i]);
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
      if (corrector)
      {
        // This path bounds alpha through the limited face fluxes instead of projecting the solved
        // field. Do not apply the inherited scalar lower limiter here.
        residuals[i] = solveAdvectedSystem(_volume_fraction_system_numbers[i],
                                           *system,
                                           _volume_fraction_equation_relaxation[i],
                                           _volume_fraction_linear_control,
                                           _volume_fraction_l_abs_tol,
                                           1.0,
                                           std::numeric_limits<Real>::min());
        system->computeGradients();
        corrector->applyCorrection(subcycle_dt, subcycle_dt / global_dt);
      }
      else
        residuals[i] = solveAdvectedSystem(_volume_fraction_system_numbers[i],
                                           *system,
                                           _volume_fraction_equation_relaxation[i],
                                           _volume_fraction_linear_control,
                                           _volume_fraction_l_abs_tol,
                                           1.0,
                                           _volume_fraction_min_value);
    }

    system->solutionOld() = *saved_old_solution;
    system->solutionOld().close();
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
  for (const auto system_i : index_range(_volume_fraction_systems))
    if (!sharpInterfaceVOFCorrector(_volume_fraction_system_names[system_i]))
      clampVolumeFractionSystem(*_volume_fraction_systems[system_i]);

  for (const auto & system : _volume_fraction_systems)
    system->computeGradients();

  for (const auto & system_name : _volume_fraction_system_names)
    if (auto * corrector = sharpInterfaceVOFCorrector(system_name))
      corrector->refreshPublishedRhoPhi();
}

void
ReducedPressurePIMPLESolve::clampVolumeFractionSystem(LinearSystem & system)
{
  auto & current_local_solution = *(system.system().current_local_solution);
  for (const auto i : make_range(current_local_solution.first_local_index(),
                                 current_local_solution.last_local_index()))
    current_local_solution.set(
        i,
        std::min(_volume_fraction_max_value,
                 std::max(_volume_fraction_min_value, current_local_solution(i))));
  current_local_solution.close();

  if (auto * previous_solution = system.solutionPreviousNewton())
  {
    for (const auto i :
         make_range(previous_solution->first_local_index(), previous_solution->last_local_index()))
      previous_solution->set(
          i,
          std::min(_volume_fraction_max_value,
                   std::max(_volume_fraction_min_value, (*previous_solution)(i))));
    previous_solution->close();
  }

  system.setSolution(current_local_solution);
}

std::pair<unsigned int, Real>
ReducedPressurePIMPLESolve::correctStartupContinuityOnce(const bool subtract_updated_pressure,
                                                         const bool recompute_face_mass_flux,
                                                         const SolverParams & solver_params)
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

  preparePressureCorrectorState(subtract_updated_pressure);

  Moose::PetscSupport::petscSetOptions(_pressure_petsc_options, solver_params);

  std::pair<unsigned int, Real> residuals{0, std::numeric_limits<Real>::quiet_NaN()};
  unsigned int total_linear_iterations = 0;
  for (const auto nonorthogonal_iteration : make_range(_num_pressure_nonorthogonal_correctors + 1))
  {
    const bool final_nonorthogonal_iteration =
        nonorthogonal_iteration == _num_pressure_nonorthogonal_correctors;

    residuals = solvePressureCorrector();
    total_linear_iterations += residuals.first;

    auto & solved_pressure_current_solution =
        *(_pressure_system.system().current_local_solution.get());
    _pressure_system.setSolution(solved_pressure_current_solution);
    _pressure_system.computeGradients();
    _rc_uo->cachePressureEquationFlux();

    if (final_nonorthogonal_iteration && recompute_face_mass_flux)
    {
      _rc_uo->computeFaceMassFlux();

      if (sharp_rc)
        sharp_rc->applyAdditionalFaceMassFluxCorrection();
    }
  }
  residuals.first = total_linear_iterations;

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

std::pair<unsigned int, Real>
ReducedPressurePIMPLESolve::correctVelocityOnce(const bool subtract_updated_pressure,
                                                const bool recompute_face_mass_flux,
                                                const SolverParams & solver_params)
{
  storePressurePreviousOuterIterationState();
  preparePressureCorrectorState(subtract_updated_pressure);

  const auto residuals =
      applyPressureCorrectionStage(recompute_face_mass_flux, true, solver_params);

  return residuals;
}

void
ReducedPressurePIMPLESolve::publishPressureCorrectedState(const bool recompute_face_mass_flux)
{
  auto & pressure_current_solution = *(_pressure_system.system().current_local_solution.get());
  _pressure_system.setSolution(pressure_current_solution);
  _pressure_system.computeGradients();
  _rc_uo->cachePressureEquationFlux();

  if (recompute_face_mass_flux)
  {
    _rc_uo->computeFaceMassFlux();
    if (auto * sharp_rc = sharpInterfaceRC())
      sharp_rc->applyAdditionalFaceMassFluxCorrection();
  }

  // The face flux and velocity writeback must keep using the cached pressure-equation flux
  // from the unrelaxed pressure solve. relaxPressureFieldForNextPredictor() refreshes
  // pressure gradients for the next predictor, but does not invalidate the cached final
  // pressure-equation flux.
  relaxPressureFieldForNextPredictor();

  _rc_uo->computeCellVelocity();

  _rc_uo->updateVelocityBoundaryState();

  correctMovingMeshFaceVelocityAndMakeRelative();
}

void
ReducedPressurePIMPLESolve::correctMovingMeshFaceVelocityAndMakeRelative()
{
  // This pressure-coupled path currently has no face-velocity or moving-mesh flux state.
  // RhieChowMassFlux explicitly reports no mesh-velocity support, so this is a no-op for the
  // stationary-mesh cases currently supported.
}
