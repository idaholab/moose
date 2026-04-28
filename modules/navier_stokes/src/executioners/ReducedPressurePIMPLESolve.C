#include "ReducedPressurePIMPLESolve.h"

#include "FEProblem.h"
#include "LinearSystem.h"
#include "SegregatedSolverUtils.h"
#include "SharpInterfaceCurvatureCalculator.h"
#include "SharpInterfaceRhieChowMassFlux.h"
#include "SharpInterfaceVOFMULESCorrector.h"
#include "TheWarehouse.h"

#include <algorithm>

using namespace libMesh;

InputParameters
ReducedPressurePIMPLESolve::validParams()
{
  InputParameters params = PIMPLESolve::validParams();
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
  params.addParam<bool>(
      "volume_fraction_outer_corrections",
      false,
      "Whether to solve the volume-fraction system(s) and refresh alpha-owned rhoPhi on every "
      "outer correction, analogous to interFoam's alphaOuterCorrectors option. When false, "
      "volume-fraction transport is performed only on the first outer iteration.");
  params.addParam<bool>(
      "perform_startup_hydrostatic_initialization",
      false,
      "Whether to run a pressure-only hydrostatic / startup cleanup stage on the first time step "
      "before the main PIMPLE iterations.");
  params.addParam<bool>(
      "suppress_explicit_hydrostatic_flux_during_seeded_startup",
      false,
      "Whether to suppress the explicit sharp-interface hydrostatic pressure-equation source "
      "flux during a seeded startup reconstruction on the first time step.");
  params.addRangeCheckedParam<unsigned int>(
      "startup_flux_corrections",
      1,
      "startup_flux_corrections>0",
      "Number of pressure-only startup cleanup corrections applied when "
      "perform_startup_hydrostatic_initialization=true.");
  params.addParam<bool>(
      "audit_outer_handoff_stages",
      false,
      "Whether to print stage-by-stage audits of the outer-loop handoff around the momentum "
      "predictor and pressure correction.");
  params.addRangeCheckedParam<unsigned int>(
      "audit_outer_handoff_after_outer",
      0,
      "audit_outer_handoff_after_outer>=0",
      "Start stage-by-stage outer-loop handoff audits after this completed outer iteration. "
      "For example, 2 audits the handoff into outer iteration 3.");
  params.addRangeCheckedParam<unsigned int>(
      "skip_momentum_predictor_after_outer",
      0,
      "skip_momentum_predictor_after_outer>=0",
      "Debug hook that skips the momentum predictor after the specified completed outer "
      "iteration. A value of 0 disables the hook.");
  params.addRangeCheckedParam<unsigned int>(
      "freeze_alpha_after_outer",
      0,
      "freeze_alpha_after_outer>=0",
      "Debug hook that freezes alpha/rhoPhi refreshes after the specified completed outer "
      "iteration. A value of 0 disables the hook.");
  params.addRangeCheckedParam<unsigned int>(
      "skip_pressure_velocity_writeback_from_outer",
      0,
      "skip_pressure_velocity_writeback_from_outer>=0",
      "Debug hook that keeps the pressure-corrected face flux but skips the reconstructed "
      "cell-velocity writeback from the specified outer iteration onward. A value of 0 "
      "disables the hook.");
  params.addParamNamesToGroup(
      "volume_fraction_systems volume_fraction_equation_relaxation volume_fraction_petsc_options "
      "volume_fraction_petsc_options_iname volume_fraction_petsc_options_value "
      "volume_fraction_absolute_tolerance volume_fraction_l_tol volume_fraction_l_abs_tol "
      "volume_fraction_l_max_its should_solve_volume_fractions volume_fraction_min_value "
      "volume_fraction_max_value volume_fraction_subcycles volume_fraction_outer_corrections "
      "perform_startup_hydrostatic_initialization "
      "suppress_explicit_hydrostatic_flux_during_seeded_startup startup_flux_corrections",
      "Volume Fraction Equations");
  params.addParamNamesToGroup("audit_outer_handoff_stages audit_outer_handoff_after_outer "
                              "skip_momentum_predictor_after_outer freeze_alpha_after_outer "
                              "skip_pressure_velocity_writeback_from_outer",
                              "Parity Debug");
  return params;
}

ReducedPressurePIMPLESolve::ReducedPressurePIMPLESolve(Executioner & ex)
  : PIMPLESolve(ex),
    _volume_fraction_system_names(getParam<std::vector<SolverSystemName>>("volume_fraction_systems")),
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
    _volume_fraction_outer_corrections(getParam<bool>("volume_fraction_outer_corrections")),
    _perform_startup_hydrostatic_initialization(
        getParam<bool>("perform_startup_hydrostatic_initialization")),
    _suppress_explicit_hydrostatic_flux_during_seeded_startup(
        getParam<bool>("suppress_explicit_hydrostatic_flux_during_seeded_startup")),
    _startup_flux_corrections(getParam<unsigned int>("startup_flux_corrections")),
    _audit_outer_handoff_stages(getParam<bool>("audit_outer_handoff_stages")),
    _audit_outer_handoff_after_outer(getParam<unsigned int>("audit_outer_handoff_after_outer")),
    _skip_momentum_predictor_after_outer(
        getParam<unsigned int>("skip_momentum_predictor_after_outer")),
    _freeze_alpha_after_outer(getParam<unsigned int>("freeze_alpha_after_outer")),
    _skip_pressure_velocity_writeback_from_outer(
        getParam<unsigned int>("skip_pressure_velocity_writeback_from_outer"))
{
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

SharpInterfaceVOFMULESCorrector *
ReducedPressurePIMPLESolve::sharpInterfaceVOFCorrector(const SolverSystemName & system_name) const
{
  std::vector<UserObject *> objs;
  _problem.theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribThread>(0)
      .queryInto(objs);

  SharpInterfaceVOFMULESCorrector * corrector_match = nullptr;
  for (const auto & obj : objs)
    if (auto * corrector = dynamic_cast<SharpInterfaceVOFMULESCorrector *>(obj);
        corrector && corrector->systemName() == system_name)
    {
      if (corrector_match)
        mooseError("ReducedPressurePIMPLESolve found multiple SharpInterfaceVOFMULESCorrector "
                   "objects for system '",
                   system_name,
                   "'.");
      corrector_match = corrector;
    }

  return corrector_match;
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

  bool vof_rho_phi_ready =
      !(_problem.timeStep() == 1 && _has_volume_fraction_systems && _should_solve_volume_fractions);

  if (auto * sharp_rc = sharpInterfaceRC())
    // interFoam's startup / initCorrectPhi path runs before any alpha subcycling has
    // published an alpha-authoritative rhoPhi. Keep the sharp-interface advection path
    // on the raw Rhie-Chow flux until the first outer-loop alpha solve populates rho_phi.
    sharp_rc->setUseVOFRhoPhi(vof_rho_phi_ready);

  if (_perform_startup_hydrostatic_initialization && _problem.timeStep() == 1)
  {
    for (auto * system : _momentum_systems)
      synchronizeSystemState(*system);
    synchronizeSystemState(_pressure_system);
    for (auto * system : _volume_fraction_systems)
      synchronizeSystemState(*system);

    _problem.execute(EXEC_NONLINEAR);
  }

  bool startup_reconstruction_complete = false;
  if (_should_solve_pressure)
  {
    startup_reconstruction_complete = initializeHydrostaticPressureField(solver_params);
    performStartupFluxCorrections(solver_params, startup_reconstruction_complete);
  }

  while (simple_iteration_counter < _num_iterations && !converged)
  {
    simple_iteration_counter++;
    _current_outer_iteration = simple_iteration_counter;

    if (shouldAuditOuterHandoff(simple_iteration_counter))
      printOuterIterationDiagnostics(simple_iteration_counter, "outer_entry");

    if (_should_solve_momentum)
      // Keep the full nonlinear history on the previous outer-corrector state
      // for the whole current outer loop. The stock momentum solve shifts this
      // stack every predictor solve; for parity work we only want to advance it
      // once per outer SIMPLE iteration.
      advanceMomentumOuterIterationHistory();

    // Mirror interFoam's outer-loop choreography by doing the alpha subcycling
    // and mixture/rhoPhi refresh inside the outer loop, just before the
    // momentum-pressure coupling work. On the first time step this is also the
    // point where rho_phi becomes available for downstream sharp-interface flux
    // queries. For the sharp-interface reduced-pressure path we always refresh
    // alpha/rhoPhi on every outer corrector so later momentum predictors do not
    // run against stale density transport.
    if (_has_volume_fraction_systems && _should_solve_volume_fractions &&
        (simple_iteration_counter == 1 || _volume_fraction_outer_corrections || sharpInterfaceRC()))
    {
      const bool freeze_alpha =
          _freeze_alpha_after_outer && simple_iteration_counter > _freeze_alpha_after_outer;

      if (!freeze_alpha)
      {
        // Keep the true timestep-old alpha in solutionOld(), but advance the
        // nonlinear-state stack once per outer iteration so we have a separate
        // previous-outer iterate available, analogous to interFoam's prevIter().
        advanceVolumeFractionOuterIterationHistory();

        if (auto * sharp_rc = sharpInterfaceRC())
          sharp_rc->setUseVOFRhoPhi(false);

        _problem.execute(EXEC_NONLINEAR);
        Moose::PetscSupport::petscSetOptions(_volume_fraction_petsc_options, solver_params);
        const auto vf_residuals = solveVolumeFractionSystems(solver_params);

        vof_rho_phi_ready = true;
        if (auto * sharp_rc = sharpInterfaceRC())
          sharp_rc->setUseVOFRhoPhi(vof_rho_phi_ready);

        _problem.execute(EXEC_NONLINEAR);
        for (const auto i : index_range(vf_residuals))
          ns_residuals[volume_fraction_indices[i]] = vf_residuals[i];
      }
      else
      {
        for (const auto i : index_range(_volume_fraction_systems))
          ns_residuals[volume_fraction_indices[i]] = std::make_pair(0u, 0.0);
        if (_audit_outer_handoff_stages)
          _console << "Outer-loop handoff audit: outer_iter=" << simple_iteration_counter
                   << " freezing alpha/rhoPhi refresh after outer iteration "
                   << _freeze_alpha_after_outer << std::endl;
      }
    }

    if (shouldAuditOuterHandoff(simple_iteration_counter))
      printOuterIterationDiagnostics(simple_iteration_counter, "after_alpha");

    if (_should_solve_momentum)
      Moose::PetscSupport::petscSetOptions(_momentum_petsc_options, solver_params);

    if (_should_solve_pressure && simple_iteration_counter == 1)
      _pressure_system.computeGradients();

    _console << "Iteration " << simple_iteration_counter << " Initial residual norms:" << std::endl;

    if (_should_solve_momentum)
    {
      const bool skip_momentum =
          _skip_momentum_predictor_after_outer &&
          simple_iteration_counter > _skip_momentum_predictor_after_outer;
      if (!skip_momentum)
      {
        auto momentum_residual = solveMomentumPredictor();
        for (const auto system_i : index_range(momentum_residual))
          ns_residuals[momentum_indices[system_i]] = momentum_residual[system_i];
      }
      else
      {
        for (const auto system_i : index_range(_momentum_systems))
          ns_residuals[momentum_indices[system_i]] = std::make_pair(0u, 0.0);
        if (_audit_outer_handoff_stages)
          _console << "Outer-loop handoff audit: outer_iter=" << simple_iteration_counter
                   << " skipping momentum predictor after outer iteration "
                   << _skip_momentum_predictor_after_outer << std::endl;
      }
    }

    if (shouldAuditOuterHandoff(simple_iteration_counter))
      printOuterIterationDiagnostics(simple_iteration_counter, "after_predictor");

    if (_should_solve_pressure)
      ns_residuals[pressure_index] = correctVelocity(true, true, solver_params);

    if (_print_fields || shouldAuditOuterHandoff(simple_iteration_counter))
      printOuterIterationDiagnostics(simple_iteration_counter, "after_pressure");

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
             << " Initial residual norms:" << std::endl;

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
  auto * const sharp_rc = sharpInterfaceRC();
  const bool restore_vof_rho_phi_for_predictor =
      sharp_rc && _current_outer_iteration > 1 && sharp_rc->useVOFRhoPhi();

  if (restore_vof_rho_phi_for_predictor)
    sharp_rc->setUseVOFRhoPhi(false);

  auto nonlinear_state_snapshots = snapshotMomentumNonlinearSolutionStates();
  auto residuals = LinearAssemblySegregatedSolve::solveMomentumPredictor();
  restoreMomentumNonlinearSolutionStates(nonlinear_state_snapshots);

  if (restore_vof_rho_phi_for_predictor)
    sharp_rc->setUseVOFRhoPhi(true);

  return residuals;
}

bool
ReducedPressurePIMPLESolve::auditMomentumPredictorRebuild() const
{
  return shouldAuditOuterHandoff(_current_outer_iteration);
}

void
ReducedPressurePIMPLESolve::addMomentumPredictorExplicitForcing(const unsigned int system_i,
                                                                NumericVector<Number> & rhs)
{
  if (auto * sharp_rc = sharpInterfaceRC();
      sharp_rc && sharp_rc->splitMomentumPredictorOperator())
    sharp_rc->addMomentumPredictorExplicitForcing(system_i, rhs);
}

SharpInterfaceRhieChowMassFlux *
ReducedPressurePIMPLESolve::sharpInterfaceRC() const
{
  return dynamic_cast<SharpInterfaceRhieChowMassFlux *>(_rc_uo);
}

SharpInterfaceCurvatureCalculator *
ReducedPressurePIMPLESolve::sharpInterfaceCurvature() const
{
  std::vector<UserObject *> objs;
  _problem.theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribThread>(0)
      .queryInto(objs);
  SharpInterfaceCurvatureCalculator * curvature_match = nullptr;
  for (const auto & obj : objs)
    if (auto * curvature = dynamic_cast<SharpInterfaceCurvatureCalculator *>(obj))
    {
      if (curvature_match)
        mooseError("ReducedPressurePIMPLESolve found multiple SharpInterfaceCurvatureCalculator "
                   "objects in the problem. The current implementation requires a single "
                   "sharp-interface curvature producer.");
      curvature_match = curvature;
    }

  return curvature_match;
}

void
ReducedPressurePIMPLESolve::synchronizeSystemState(LinearSystem & system) const
{
  system.solutionOld() = *(system.system().current_local_solution);
  system.solutionOld().close();

  for (unsigned int state = 1;
       system.hasSolutionState(state, Moose::SolutionIterationType::Nonlinear);
       ++state)
  {
    auto & nonlinear_state = system.solutionState(state, Moose::SolutionIterationType::Nonlinear);
    nonlinear_state = system.solutionOld();
    nonlinear_state.close();
  }
}

void
ReducedPressurePIMPLESolve::assembleMomentumPredictorOnly()
{
  if (!_should_solve_momentum)
    return;

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
    NumericVector<Number> & solution = *(momentum_system.solution);
    NumericVector<Number> & rhs = *(momentum_system.rhs);
    SparseMatrix<Number> & mmat = *(momentum_system.matrix);

    auto diff_diagonal = solution.zero_clone();

    // Assemble and relax the momentum predictor exactly as in the main SIMPLE loop,
    // but stop before the linear solve so startup pressure correction can reuse the
    // same diagonal / HbyA operator without advancing momentum.
    _problem.computeLinearSystemSys(momentum_system, mmat, rhs, /*compute_grads*/ true);
    NS::FV::relaxMatrix(mmat, _momentum_equation_relaxation, *diff_diagonal);
    NS::FV::relaxRightHandSide(rhs, solution, *diff_diagonal);
    momentum_system.update();

    if (_rc_uo)
      _rc_uo->cacheMomentumPredictorOperator(system_i);
  }

  momentum_solver.reuse_preconditioner(false);
}

bool
ReducedPressurePIMPLESolve::initializeHydrostaticPressureField(const SolverParams & solver_params)
{
  if (!_perform_startup_hydrostatic_initialization || _problem.timeStep() != 1)
    return false;

  if (!_should_solve_pressure)
    return false;

  bool seeded_pressure = false;
  if (auto * sharp_rc = sharpInterfaceRC())
    if (sharp_rc->seedHydrostaticPressure(_pressure_system, _pressure_pin_dof, _pressure_pin_value))
    {
      seeded_pressure = true;
      synchronizeSystemState(_pressure_system);
      _pressure_system.computeGradients();
      _problem.execute(EXEC_NONLINEAR);
    }

  _console << "Seeding reduced-pressure hydrostatic field before SIMPLE iterations" << std::endl;

  if (seeded_pressure)
  {
    // Closest MOOSE equivalent of interFoam's initCorrectPhi: assemble the
    // momentum predictor coefficients, then run pressure-only startup
    // continuity corrections with full pressure updates before the first
    // SIMPLE outer iteration.
    if (_should_solve_momentum && _rc_uo)
    {
      assembleMomentumPredictorOnly();
      _rc_uo->initFaceMassFlux();
      performStartupContinuityCorrections(solver_params);
      synchronizeSystemState(_pressure_system);
    }
    _problem.execute(EXEC_NONLINEAR);
    return true;
  }

  Moose::PetscSupport::petscSetOptions(_pressure_petsc_options, solver_params);
  correctVelocityOnce(true, true, solver_params);

  _problem.execute(EXEC_NONLINEAR);
  return false;
}

void
ReducedPressurePIMPLESolve::performStartupContinuityCorrections(const SolverParams & solver_params)
{
  if (!_perform_startup_hydrostatic_initialization || _problem.timeStep() != 1)
    return;

  if (!_should_solve_pressure || !_should_solve_momentum || !_rc_uo)
    return;

  _console << "Applying startup continuity / CorrectPhi corrections" << std::endl;
  Moose::PetscSupport::petscSetOptions(_pressure_petsc_options, solver_params);

  for (const auto startup_it : make_range(_startup_flux_corrections))
  {
    (void)startup_it;
    (void)correctStartupContinuityOnce(true, true, solver_params);
  }

  _problem.execute(EXEC_NONLINEAR);
}

void
ReducedPressurePIMPLESolve::performStartupFluxCorrections(const SolverParams & solver_params,
                                                          const bool startup_reconstruction_complete)
{
  if (!_perform_startup_hydrostatic_initialization || _problem.timeStep() != 1)
    return;

  if (!_should_solve_pressure)
    return;

  if (startup_reconstruction_complete)
    return;

  _console << "Applying startup hydrostatic / flux cleanup corrections" << std::endl;
  Moose::PetscSupport::petscSetOptions(_pressure_petsc_options, solver_params);
  for (const auto startup_it : make_range(_startup_flux_corrections))
  {
    (void)startup_it;
    correctVelocityOnce(true, true, solver_params);
  }

  _problem.execute(EXEC_NONLINEAR);
}

void
ReducedPressurePIMPLESolve::preparePressureCorrectorState(const bool subtract_updated_pressure)
{
  // Refresh the curvature producer before the pressure predictor stage so every
  // downstream face functor sees the latest smoothed normals / curvature.
  if (auto * curvature = sharpInterfaceCurvature())
    curvature->updateCurvatureMaps(_print_fields);

  _rc_uo->computeHbyA(subtract_updated_pressure, _print_fields);

  if (auto * sharp_rc = sharpInterfaceRC())
    sharp_rc->updateAdditionalPressureFluxFunctors(subtract_updated_pressure, _print_fields);

  _rc_uo->updatePressureBoundaryNormalGradients(_pin_pressure);
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

  if (auto * sharp_rc = sharpInterfaceRC())
  {
    sharp_rc->applyAdditionalCellVelocityCorrection();
    if (_print_fields || sharp_rc->suppressExplicitHydrostaticPressureFlux())
      sharp_rc->auditRepresentativeHorizontalFaceReconstruction();
  }

  _rc_uo->updateVelocityBoundaryState();
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
      auto & nonlinear_state = system->solutionState(state, Moose::SolutionIterationType::Nonlinear);
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
ReducedPressurePIMPLESolve::advanceMomentumOuterIterationHistory() const
{
  advanceSystemOuterIterationHistory(_momentum_systems);
}

void
ReducedPressurePIMPLESolve::advanceVolumeFractionOuterIterationHistory() const
{
  advanceSystemOuterIterationHistory(_volume_fraction_systems);
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
    for (unsigned int state = 1;
         _momentum_systems[system_i]->hasSolutionState(state, Moose::SolutionIterationType::Nonlinear);
         ++state)
    {
      const auto & nonlinear_state =
          _momentum_systems[system_i]->solutionState(state, Moose::SolutionIterationType::Nonlinear);
      auto snapshot = nonlinear_state.zero_clone();
      *snapshot = nonlinear_state;
      snapshot->close();
      snapshots[system_i].push_back(std::move(snapshot));
    }

  return snapshots;
}

void
ReducedPressurePIMPLESolve::printOuterIterationDiagnostics(const unsigned int simple_iteration_counter,
                                                           const std::string & stage_label) const
{
  _console << "Outer iteration " << simple_iteration_counter << " state audit (" << stage_label
           << "):" << std::endl;

  for (const auto system_i : index_range(_momentum_systems))
  {
    const auto & current_solution = *(_momentum_systems[system_i]->system().current_local_solution);
    _console << "  Momentum system " << _momentum_systems[system_i]->name()
             << " |state0|=" << current_solution.l2_norm()
             << " |state0-stateOld|="
             << current_solution.l2_norm_diff(_momentum_systems[system_i]->solutionOld());

    if (_momentum_systems[system_i]->hasSolutionState(1, Moose::SolutionIterationType::Nonlinear))
      _console << " |state0-state1|="
               << current_solution.l2_norm_diff(_momentum_systems[system_i]->solutionState(
                      1, Moose::SolutionIterationType::Nonlinear));

    if (_momentum_systems[system_i]->hasSolutionState(2, Moose::SolutionIterationType::Nonlinear))
      _console << " |state1-state2|="
               << _momentum_systems[system_i]
                      ->solutionState(1, Moose::SolutionIterationType::Nonlinear)
                      .l2_norm_diff(_momentum_systems[system_i]->solutionState(
                          2, Moose::SolutionIterationType::Nonlinear));

    if (auto * previous_solution = _momentum_systems[system_i]->solutionPreviousNewton())
      _console << " |state0-statePrev|=" << current_solution.l2_norm_diff(*previous_solution);

    _console << std::endl;
  }

  for (const auto system_i : index_range(_volume_fraction_systems))
  {
    const auto & current_solution =
        *(_volume_fraction_systems[system_i]->system().current_local_solution);
    _console << "  Volume-fraction system " << _volume_fraction_systems[system_i]->name()
             << " |state0-stateOld|="
             << current_solution.l2_norm_diff(_volume_fraction_systems[system_i]->solutionOld());

    if (_volume_fraction_systems[system_i]->hasSolutionState(1, Moose::SolutionIterationType::Nonlinear))
      _console << " |state0-state1|="
               << current_solution.l2_norm_diff(_volume_fraction_systems[system_i]->solutionState(
                      1, Moose::SolutionIterationType::Nonlinear));

    if (_volume_fraction_systems[system_i]->hasSolutionState(2, Moose::SolutionIterationType::Nonlinear))
      _console << " |state1-state2|="
               << _volume_fraction_systems[system_i]
                      ->solutionState(1, Moose::SolutionIterationType::Nonlinear)
                      .l2_norm_diff(_volume_fraction_systems[system_i]->solutionState(
                          2, Moose::SolutionIterationType::Nonlinear));

    if (auto * previous_solution = _volume_fraction_systems[system_i]->solutionPreviousNewton())
      _console << " |state0-statePrev|=" << current_solution.l2_norm_diff(*previous_solution);

    _console << std::endl;
  }

  const auto & pressure_current_solution = *(_pressure_system.system().current_local_solution);
  _console << "  Pressure system " << _pressure_system.name()
           << " |state0|=" << pressure_current_solution.l2_norm()
           << " |state0-stateOld|="
           << pressure_current_solution.l2_norm_diff(_pressure_system.solutionOld());
  if (auto * previous_pressure = _pressure_system.solutionPreviousNewton())
    _console << " |state0-statePrev|="
             << pressure_current_solution.l2_norm_diff(*previous_pressure);
  _console << std::endl;

  if (_rc_uo)
  {
    _console << "  Boundary mass-flux imbalance=" << _rc_uo->boundaryMassFluxImbalance()
             << " max|boundary flux|=" << _rc_uo->maxBoundaryMassFluxMagnitude()
             << " |phi|_2=" << _rc_uo->faceMassFluxL2Norm() << std::endl;

    const auto flux_consistency = _rc_uo->faceMassFluxConsistencyAudit();
    _console << "  Face-mass-flux consistency |phi-phi(U)|_2=" << flux_consistency.l2_norm
             << " |internal|_2=" << flux_consistency.internal_l2_norm
             << " |boundary|_2=" << flux_consistency.boundary_l2_norm
             << " max|phi-phi(U)|=" << flux_consistency.max_abs_mismatch
             << " max|internal|=" << flux_consistency.max_abs_internal_mismatch
             << " max|boundary|=" << flux_consistency.max_abs_boundary_mismatch;

    if (flux_consistency.has_worst_face)
      _console << " worst_face_id=" << flux_consistency.worst_face_id
               << " worst_face_boundary=" << flux_consistency.worst_face_is_boundary
               << " centroid=" << flux_consistency.worst_face_centroid
               << " normal=" << flux_consistency.worst_face_normal;

    _console << std::endl;
  }

  if (auto * sharp_rc = sharpInterfaceRC())
  {
    sharp_rc->printPressureCoupledVelocityCorrectionAudit(stage_label);
    if (stage_label == "after_alpha" && sharp_rc->useFaceBasedPredictorBodyForce())
      sharp_rc->auditRepresentativePredictorBodyForce();
  }
}

bool
ReducedPressurePIMPLESolve::shouldAuditOuterHandoff(const unsigned int simple_iteration_counter) const
{
  return _audit_outer_handoff_stages &&
         simple_iteration_counter > _audit_outer_handoff_after_outer;
}

std::vector<std::pair<unsigned int, Real>>
ReducedPressurePIMPLESolve::solveVolumeFractionSystems(const SolverParams & /*solver_params*/)
{
  std::vector<std::pair<unsigned int, Real>> residuals(
      _volume_fraction_system_names.size(), std::make_pair(0, 1.0));

  const Real global_dt = _problem.dt();
  const Real global_time = _problem.time();
  const Real global_time_old = _problem.timeOld();
  const Real subcycle_dt = global_dt / _volume_fraction_subcycles;

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

    for (const auto subcycle : make_range(_volume_fraction_subcycles))
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
        residuals[i] = solveAdvectedSystem(_volume_fraction_system_numbers[i],
                                           *system,
                                           _volume_fraction_equation_relaxation[i],
                                           _volume_fraction_linear_control,
                                           _volume_fraction_l_abs_tol,
                                           1.0,
                                           _volume_fraction_min_value);
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

  clampVolumeFractionSystems();

  for (const auto & system : _volume_fraction_systems)
    system->computeGradients();

  return residuals;
}

void
ReducedPressurePIMPLESolve::clampVolumeFractionSystems()
{
  for (auto * system : _volume_fraction_systems)
  {
    auto & current_local_solution = *(system->system().current_local_solution);
    for (const auto i : make_range(current_local_solution.first_local_index(),
                                   current_local_solution.last_local_index()))
      current_local_solution.set(
          i,
          std::min(_volume_fraction_max_value,
                   std::max(_volume_fraction_min_value, current_local_solution(i))));
    current_local_solution.close();

    if (auto * previous_solution = system->solutionPreviousNewton())
    {
      for (const auto i :
           make_range(previous_solution->first_local_index(), previous_solution->last_local_index()))
        previous_solution->set(
            i,
            std::min(_volume_fraction_max_value,
                     std::max(_volume_fraction_min_value, (*previous_solution)(i))));
      previous_solution->close();
    }

    system->setSolution(current_local_solution);
  }
}

std::pair<unsigned int, Real>
ReducedPressurePIMPLESolve::correctVelocity(const bool /*subtract_updated_pressure*/,
                                            const bool /*recompute_face_mass_flux*/,
                                            const SolverParams & solver_params)
{
  std::pair<unsigned int, Real> residual;
  unsigned int piso_iteration_counter = 0;

  while (piso_iteration_counter <= _num_piso_iterations)
  {
    // Treat every local pressure-corrector pass as a full pEqn-style update:
    // rebuild the predictor from the current pressure state and update phi/U
    // on every sub-iteration.
    residual = correctVelocityOnce(true, true, solver_params);
    piso_iteration_counter++;
  }

  return residual;
}

std::pair<unsigned int, Real>
ReducedPressurePIMPLESolve::correctStartupContinuityOnce(const bool subtract_updated_pressure,
                                                         const bool recompute_face_mass_flux,
                                                         const SolverParams & solver_params)
{
  preparePressureCorrectorState(subtract_updated_pressure);

  Moose::PetscSupport::petscSetOptions(_pressure_petsc_options, solver_params);

  const auto residuals = solvePressureCorrector();

  auto & pressure_current_solution = *(_pressure_system.system().current_local_solution.get());
  auto & pressure_old_solution = *(_pressure_system.solutionPreviousNewton());

  // initCorrectPhi uses a dedicated pcorr field and applies the full continuity
  // correction. We do the closest equivalent here by installing the startup
  // pressure update without SIMPLE under-relaxation.
  pressure_old_solution = pressure_current_solution;
  _pressure_system.setSolution(pressure_current_solution);

  _pressure_system.computeGradients();

  if (recompute_face_mass_flux)
  {
    _rc_uo->computeFaceMassFlux();

    if (auto * sharp_rc = sharpInterfaceRC())
    {
      sharp_rc->applyAdditionalFaceMassFluxCorrection();
      if (_print_fields || sharp_rc->suppressExplicitHydrostaticPressureFlux())
        sharp_rc->auditRepresentativeHorizontalFaceReconstruction();
    }
  }

  return residuals;
}

std::pair<unsigned int, Real>
ReducedPressurePIMPLESolve::correctVelocityOnce(const bool subtract_updated_pressure,
                                                const bool recompute_face_mass_flux,
                                                const SolverParams & solver_params)
{
  const bool audit_stage = shouldAuditOuterHandoff(_current_outer_iteration);
  if (auto * sharp_rc = sharpInterfaceRC())
    sharp_rc->clearPressureCoupledVelocityCorrectionAudit();

  const bool skip_pressure_velocity_writeback =
      _current_outer_iteration &&
      _skip_pressure_velocity_writeback_from_outer &&
      _current_outer_iteration >= _skip_pressure_velocity_writeback_from_outer;

  preparePressureCorrectorState(subtract_updated_pressure);

  Moose::PetscSupport::petscSetOptions(_pressure_petsc_options, solver_params);

  const auto residuals = solvePressureCorrector();

  auto & pressure_current_solution = *(_pressure_system.system().current_local_solution.get());
  auto & pressure_old_solution = *(_pressure_system.solutionPreviousNewton());

  _pressure_system.setSolution(pressure_current_solution);

  _pressure_system.computeGradients();

  if (audit_stage)
    printOuterIterationDiagnostics(_current_outer_iteration, "after_pressure_solve");

  if (recompute_face_mass_flux)
  {
    _rc_uo->computeFaceMassFlux();

    if (auto * sharp_rc = sharpInterfaceRC())
      sharp_rc->applyAdditionalFaceMassFluxCorrection();

    if (audit_stage)
      printOuterIterationDiagnostics(_current_outer_iteration, "after_face_mass_flux");
  }

  if (!skip_pressure_velocity_writeback)
  {
    _rc_uo->computeCellVelocity();

    if (audit_stage)
      printOuterIterationDiagnostics(_current_outer_iteration, "after_base_cell_velocity");

    if (auto * sharp_rc = sharpInterfaceRC())
    {
      sharp_rc->applyAdditionalCellVelocityCorrection();
      if (audit_stage)
        sharp_rc->auditRepresentativeHorizontalFaceReconstruction();
      if (audit_stage)
        printOuterIterationDiagnostics(_current_outer_iteration, "after_sharp_cell_velocity");
      if (_print_fields || sharp_rc->suppressExplicitHydrostaticPressureFlux())
        sharp_rc->auditRepresentativeHorizontalFaceReconstruction();
    }

    _rc_uo->updateVelocityBoundaryState();

    if (audit_stage)
      printOuterIterationDiagnostics(_current_outer_iteration, "after_boundary_refresh");
  }
  else if (_audit_outer_handoff_stages)
    _console << "Outer-loop handoff audit: outer_iter=" << _current_outer_iteration
             << " skipping post-pEqn cell-velocity writeback from outer iteration "
             << _skip_pressure_velocity_writeback_from_outer << std::endl;

  // Mirror interFoam's pEqn.H ordering: use the exact pressure correction to
  // update phi/U, then under-relax pressure only for the next momentum
  // predictor.
  NS::FV::relaxSolutionUpdate(
      pressure_current_solution, pressure_old_solution, _pressure_variable_relaxation);

  pressure_old_solution = pressure_current_solution;
  _pressure_system.setSolution(pressure_current_solution);
  _pressure_system.computeGradients();

  return residuals;
}
