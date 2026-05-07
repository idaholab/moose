#include "ReducedPressurePIMPLESolve.h"

#include "FEProblem.h"
#include "LinearSystem.h"
#include "MooseApp.h"
#include "PetscVectorReader.h"
#include "SegregatedSolverUtils.h"
#include "SharpInterfaceCurvatureCalculator.h"
#include "SharpInterfaceRhieChowMassFlux.h"
#include "SharpInterfaceVOFMULESCorrector.h"
#include "TheWarehouse.h"
#include "libmesh/petsc_linear_solver.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
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
      "momentum-pressure convergence loop.");
  params.setDocString("num_piso_iterations",
                      "The maximum number of additional inner pressure-correction-only PISO "
                      "stages performed without rebuilding the momentum matrix on each outer "
                      "correction. The loop exits early when the PISO residual tolerances are "
                      "satisfied.");
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
  params.addParam<bool>(
      "volume_fraction_outer_corrections",
      false,
      "Deprecated compatibility switch. The reduced-pressure sharp-interface PIMPLE path now "
      "always refreshes the volume-fraction system(s) and alpha-owned rhoPhi on every outer "
      "correction to match interFoam's outer-loop architecture.");
  MooseEnum startup_pressure_initialization("none projection-only equilibrium-seed",
                                            "projection-only");
  params.addParam<MooseEnum>(
      "startup_pressure_initialization",
      startup_pressure_initialization,
      "Startup reduced-pressure initialization policy on the first time step. Use "
      "'projection-only' to mimic interFoam's initCorrectPhi-style startup projection without "
      "overwriting the user-supplied reduced-pressure field, 'equilibrium-seed' to explicitly "
      "construct a quiescent reduced-pressure equilibrium before projection, or 'none' to skip "
      "startup pressure cleanup entirely.");
  params.addParam<bool>(
      "perform_startup_hydrostatic_initialization",
      false,
      "Deprecated compatibility switch. When explicitly set true it maps to "
      "startup_pressure_initialization=equilibrium-seed, and when explicitly set false it maps "
      "to startup_pressure_initialization=none.");
  params.addParam<bool>(
      "suppress_explicit_hydrostatic_flux_during_seeded_startup",
      false,
      "Whether to suppress the explicit sharp-interface hydrostatic pressure-equation source "
      "flux during an equilibrium-seed startup reconstruction on the first time step.");
  params.addRangeCheckedParam<unsigned int>(
      "startup_flux_corrections",
      1,
      "startup_flux_corrections>0",
      "Number of pressure-only startup cleanup / projection corrections applied when "
      "startup_pressure_initialization is not 'none'.");
  params.addParam<bool>(
      "dump_pressure_outer_debug_csv",
      false,
      "Whether to dump the pre-pressure-solve reduced-pressure cell state and sharp-interface "
      "face-operator state to CSV for the first few outer iterations.");
  params.addRangeCheckedParam<unsigned int>(
      "dump_pressure_outer_debug_start_timestep",
      1,
      "dump_pressure_outer_debug_start_timestep>0",
      "First timestep index included in the pressure debug CSV dumps.");
  params.addRangeCheckedParam<unsigned int>(
      "dump_pressure_outer_debug_end_timestep",
      std::numeric_limits<unsigned int>::max(),
      "dump_pressure_outer_debug_end_timestep>0",
      "Last timestep index included in the pressure debug CSV dumps.");
  params.addRangeCheckedParam<unsigned int>(
      "dump_pressure_outer_debug_max_outer_iterations",
      2,
      "dump_pressure_outer_debug_max_outer_iterations>0",
      "Maximum outer iteration index included in the pressure debug CSV dumps.");
  params.addParamNamesToGroup(
      "volume_fraction_systems volume_fraction_equation_relaxation volume_fraction_petsc_options "
      "volume_fraction_petsc_options_iname volume_fraction_petsc_options_value "
      "volume_fraction_absolute_tolerance volume_fraction_l_tol volume_fraction_l_abs_tol "
      "volume_fraction_l_max_its should_solve_volume_fractions volume_fraction_min_value "
      "volume_fraction_max_value volume_fraction_subcycles volume_fraction_max_courant "
      "adjust_momentum_pressure_time_step momentum_pressure_max_courant "
      "volume_fraction_outer_corrections "
      "startup_pressure_initialization perform_startup_hydrostatic_initialization "
      "suppress_explicit_hydrostatic_flux_during_seeded_startup startup_flux_corrections "
      "dump_pressure_outer_debug_csv dump_pressure_outer_debug_start_timestep "
      "dump_pressure_outer_debug_end_timestep dump_pressure_outer_debug_max_outer_iterations",
      "Volume Fraction Equations");
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
    _volume_fraction_max_courant(getParam<Real>("volume_fraction_max_courant")),
    _adjust_momentum_pressure_time_step(getParam<bool>("adjust_momentum_pressure_time_step")),
    _momentum_pressure_max_courant(getParam<Real>("momentum_pressure_max_courant")),
    _volume_fraction_outer_corrections(getParam<bool>("volume_fraction_outer_corrections")),
    _dump_pressure_outer_debug_csv(getParam<bool>("dump_pressure_outer_debug_csv")),
    _dump_pressure_outer_debug_start_timestep(
        getParam<unsigned int>("dump_pressure_outer_debug_start_timestep")),
    _dump_pressure_outer_debug_end_timestep(
        getParam<unsigned int>("dump_pressure_outer_debug_end_timestep")),
    _dump_pressure_outer_debug_max_outer_iterations(
        getParam<unsigned int>("dump_pressure_outer_debug_max_outer_iterations")),
    _suppress_explicit_hydrostatic_flux_during_seeded_startup(
        getParam<bool>("suppress_explicit_hydrostatic_flux_during_seeded_startup")),
    _startup_flux_corrections(getParam<unsigned int>("startup_flux_corrections"))
{
  _startup_pressure_initialization =
      getParam<MooseEnum>("startup_pressure_initialization").operator std::string();
  if (parameters().isParamSetByUser("perform_startup_hydrostatic_initialization"))
    _startup_pressure_initialization =
        getParam<bool>("perform_startup_hydrostatic_initialization") ? "equilibrium-seed" : "none";

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

bool
ReducedPressurePIMPLESolve::useEquilibriumStartupPressureInitialization() const
{
  return _startup_pressure_initialization == "equilibrium-seed";
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

  bool vof_rho_phi_ready =
      !(_problem.timeStep() == 1 && _has_volume_fraction_systems && _should_solve_volume_fractions);

  if (auto * sharp_rc = sharpInterfaceRC())
  {
    // interFoam's startup / initCorrectPhi path runs before any alpha subcycling has
    // published an alpha-authoritative rhoPhi. Keep the sharp-interface advection path
    // on the raw Rhie-Chow flux until the first outer-loop alpha solve populates rho_phi.
    sharp_rc->setUseVOFRhoPhi(vof_rho_phi_ready);
    sharp_rc->clearOuterIterationConvectiveState();
  }

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
    initializeStartupPressureField(solver_params);

  while (simple_iteration_counter < _num_iterations && !converged)
  {
    simple_iteration_counter++;
    _current_outer_iteration = simple_iteration_counter;

    if (_should_solve_momentum)
      // Keep the full nonlinear history on the previous outer-corrector state
      // for the whole current outer loop. The stock momentum solve shifts this
      // stack every predictor solve; for parity work we only want to advance it
      // once per outer SIMPLE iteration.
      advanceMomentumOuterIterationHistory();

    // When alpha/rhoPhi is not being solved in this executioner step, the
    // momentum predictor still needs a fresh convective state built from the
    // latest corrected flow fluxes. The startup path seeds a raw state before
    // any pressure cleanup, so resynchronize here on every outer iteration.
    if ((!_has_volume_fraction_systems || !_should_solve_volume_fractions) &&
        sharpInterfaceRC())
    {
      auto * sharp_rc = sharpInterfaceRC();
      sharp_rc->setUseVOFRhoPhi(false);
      sharp_rc->clearOuterIterationConvectiveState();
    }

    // Mirror interFoam's outer-loop choreography by doing the alpha subcycling
    // and mixture/rhoPhi refresh inside every outer correction, just before
    // the momentum-pressure coupling work. This keeps rhoPhi consistent with
    // the outer-corrector state instead of freezing one alpha update for a
    // later sequence of momentum-pressure repredictions.
    if (_has_volume_fraction_systems && _should_solve_volume_fractions)
    {
      // Keep the true timestep-old alpha in solutionOld(), but advance the
      // nonlinear-state stack once per outer iteration so we have a separate
      // previous-outer iterate available, analogous to interFoam's prevIter().
      advanceVolumeFractionOuterIterationHistory();

      if (auto * sharp_rc = sharpInterfaceRC())
      {
        sharp_rc->setUseVOFRhoPhi(false);
        sharp_rc->clearOuterIterationConvectiveState();
      }

      _problem.execute(EXEC_NONLINEAR);
      Moose::PetscSupport::petscSetOptions(_volume_fraction_petsc_options, solver_params);
      const auto vf_residuals = solveVolumeFractionSystems(solver_params);

      vof_rho_phi_ready = true;
      if (auto * sharp_rc = sharpInterfaceRC())
      {
        sharp_rc->freezeOuterIterationConvectiveState();
        sharp_rc->setUseVOFRhoPhi(vof_rho_phi_ready);
      }

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

    _console << "Passive scalar iteration " << ps_iteration_counter << " Residual norms:"
             << std::endl;

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
      sharp_rc && _current_outer_iteration > 1 && sharp_rc->useVOFRhoPhi() &&
      !sharp_rc->hasOuterIterationConvectiveState();

  // Once alpha transport has published and frozen an outer-iteration rhoPhi/phi
  // pair, keep the momentum predictor on that same convective branch for the
  // rest of the outer iteration. Only fall back to the raw Rhie-Chow family
  // when no frozen alpha-authoritative state exists yet.
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
  return false;
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
  if (_momentum_systems.empty())
    return;

  auto * const sharp_rc = sharpInterfaceRC();
  const bool restore_vof_rho_phi_for_predictor =
      sharp_rc && _current_outer_iteration > 1 && sharp_rc->useVOFRhoPhi() &&
      !sharp_rc->hasOuterIterationConvectiveState();

  // Mirror solveMomentumPredictor(): predictor assembly should stay on the
  // frozen outer-iteration convective state once alpha has published it.
  if (restore_vof_rho_phi_for_predictor)
    sharp_rc->setUseVOFRhoPhi(false);

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
    std::unique_ptr<NumericVector<Number>> predictor_diagonal_raw;
    std::unique_ptr<NumericVector<Number>> predictor_rhs_base;
    std::unique_ptr<NumericVector<Number>> predictor_explicit_force;
    std::unique_ptr<NumericVector<Number>> predictor_body_force;

    // Assemble and relax the momentum predictor exactly as in the main SIMPLE loop,
    // but stop before the linear solve so startup pressure correction can reuse the
    // same diagonal / HbyA operator without advancing momentum.
    _problem.computeLinearSystemSys(momentum_system, mmat, rhs, /*compute_grads*/ true);
    NS::FV::relaxMatrix(mmat, _momentum_equation_relaxation, *diff_diagonal);
    NS::FV::relaxRightHandSide(rhs, solution, *diff_diagonal);

    if (_rc_uo && _rc_uo->splitMomentumPredictorOperator())
    {
      predictor_diagonal_raw = solution.zero_clone();
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
      addMomentumPredictorBodyForceForcing(system_i, *predictor_body_force);
      predictor_body_force->close();

      addMomentumPredictorExplicitForcing(system_i, rhs);

      predictor_explicit_force = rhs.clone();
      *predictor_explicit_force = rhs;
      predictor_explicit_force->add(-1.0, *predictor_rhs_base);
      predictor_explicit_force->close();
    }

    momentum_system.update();

    if (_rc_uo)
      _rc_uo->cacheMomentumPredictorOperator(system_i,
                                             predictor_rhs_base.get(),
                                             predictor_explicit_force.get(),
                                             predictor_body_force.get());
  }

  momentum_solver.reuse_preconditioner(false);

  if (restore_vof_rho_phi_for_predictor)
    sharp_rc->setUseVOFRhoPhi(true);
}

void
ReducedPressurePIMPLESolve::initializeStartupPressureField(const SolverParams & solver_params)
{
  if (!startupPressureInitializationEnabled() || _problem.timeStep() != 1)
    return;

  if (!_should_solve_pressure)
    return;

  bool seeded_pressure = false;
  if (useEquilibriumStartupPressureInitialization())
  {
    _console << "Applying startup reduced-pressure equilibrium seed before PIMPLE iterations"
             << std::endl;
    if (auto * sharp_rc = sharpInterfaceRC())
      if (sharp_rc->seedHydrostaticPressure(_pressure_system, _pressure_pin_dof, _pressure_pin_value))
      {
        seeded_pressure = true;
        synchronizeSystemState(_pressure_system);
        _pressure_system.computeGradients();
        _problem.execute(EXEC_NONLINEAR);
      }

    if (!seeded_pressure)
      _console << "Startup equilibrium seed could not be constructed; falling back to "
                  "projection-only startup cleanup"
               << std::endl;
  }
  else
    _console << "Applying startup continuity / CorrectPhi projection before PIMPLE iterations"
             << std::endl;

  // Closest MOOSE equivalent of interFoam's initCorrectPhi: honor the current
  // reduced-pressure field (user-supplied or equilibrium-seeded), assemble the
  // momentum predictor coefficients, and run pressure-only startup continuity
  // corrections before the first outer iteration.
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
  {
    sharp_rc->applyAdditionalFaceMassFluxCorrection();
    sharp_rc->refreshPredictorConvectiveStateFromCurrentCorrectedFluxes();
  }

  _rc_uo->computeCellVelocity();

  _rc_uo->updateVelocityBoundaryState();
}

void
ReducedPressurePIMPLESolve::dumpPressureOuterDebugState(const std::string & stage_label)
{
  if (!_dump_pressure_outer_debug_csv ||
      _problem.timeStep() < _dump_pressure_outer_debug_start_timestep ||
      _problem.timeStep() > _dump_pressure_outer_debug_end_timestep ||
      _current_outer_iteration > _dump_pressure_outer_debug_max_outer_iterations)
    return;

  const std::string file_base = _app.getOutputFileBase();
  const std::string iter_label = "_ts" + std::to_string(_problem.timeStep()) + "_outer" +
                                 std::to_string(_current_outer_iteration) + "_piso" +
                                 std::to_string(_current_piso_iteration) + "_" + stage_label;

  {
    std::ofstream out(file_base + iter_label + "_pressure_cells.csv");
    if (!out)
      mooseError("Failed to open reduced-pressure cell debug CSV for outer iteration ",
                 _current_outer_iteration);

    out << std::setprecision(17);
    out << "elem_id,x,y,z,pressure_current,pressure_old,grad_p_x,grad_p_y,grad_p_z,"
           "u_current,v_current,w_current,"
           "HbyA_u,HbyA_v,HbyA_w,"
           "Ainv_u,Ainv_v,Ainv_w,"
           "neg_Ainv_gradp_u,neg_Ainv_gradp_v,neg_Ainv_gradp_w,"
           "predictor_face_based_pressure,"
           "predictor_pressure_force_u,predictor_pressure_force_v,predictor_pressure_force_w,"
           "predictor_body_force_u,predictor_body_force_v,predictor_body_force_w,"
           "predictor_cell_body_force_u,predictor_cell_body_force_v,predictor_cell_body_force_w,"
           "predictor_scalar_pressure_force_u,predictor_scalar_pressure_force_v,predictor_scalar_pressure_force_w,"
           "predictor_scalar_body_force_u,predictor_scalar_body_force_v,predictor_scalar_body_force_w,"
           "predictor_total_force_u,predictor_total_force_v,predictor_total_force_w,"
           "predictor_rhs_u,predictor_rhs_v,predictor_rhs_w,"
           "corr_faces,corr_singular,"
           "corr_matrix_00,corr_matrix_01,corr_matrix_02,"
           "corr_matrix_10,corr_matrix_11,corr_matrix_12,"
           "corr_matrix_20,corr_matrix_21,corr_matrix_22,"
           "corr_rhs_0,corr_rhs_1,corr_rhs_2,"
           "corr_solution_0,corr_solution_1,corr_solution_2,"
           "corr_delta_u,corr_delta_v,corr_delta_w\n";

    PetscVectorReader pressure_reader(*_pressure_system.system().current_local_solution);
    auto * pressure_old_solution = _pressure_system.solutionPreviousNewton();
    std::unique_ptr<PetscVectorReader> pressure_old_reader;
    if (pressure_old_solution)
      pressure_old_reader = std::make_unique<PetscVectorReader>(*pressure_old_solution);

    auto & pressure_gradient = _pressure_system.linearFVGradientContainer();
    std::vector<std::unique_ptr<PetscVectorReader>> grad_readers;
    grad_readers.reserve(pressure_gradient.size());
    for (const auto & component : pressure_gradient)
      grad_readers.push_back(std::make_unique<PetscVectorReader>(*component));

    std::vector<std::unique_ptr<PetscVectorReader>> momentum_readers;
    momentum_readers.reserve(_momentum_systems.size());
    for (const auto system_i : index_range(_momentum_systems))
    {
      auto & momentum_sys =
          libMesh::cast_ref<LinearImplicitSystem &>(_momentum_systems[system_i]->system());
      momentum_readers.push_back(std::make_unique<PetscVectorReader>(*momentum_sys.current_local_solution));
    }

    auto * sharp_rc = sharpInterfaceRC();

    for (const auto & elem_info : _problem.mesh().elemInfoVector())
    {
      if (elem_info->dofIndices().size() <= static_cast<std::size_t>(_pressure_sys_number) ||
          elem_info->dofIndices()[_pressure_sys_number].empty())
        continue;

      const auto dof = elem_info->dofIndices()[_pressure_sys_number][0];
      const Point centroid = elem_info->centroid();
      const auto corr_debug =
          sharp_rc ? sharp_rc->pressureCorrectionReconstructionDebug(*elem_info, Moose::currentState())
                   : SharpInterfaceRhieChowMassFlux::PressureCorrectionReconstructionDebug{};
      const auto predictor_force_debug =
          sharp_rc ? sharp_rc->momentumPredictorExplicitForceDebug(*elem_info, Moose::currentState())
                   : SharpInterfaceRhieChowMassFlux::MomentumPredictorExplicitForceDebug{};

      out << elem_info->elem()->id() << ',' << centroid(0) << ',' << centroid(1) << ','
          << centroid(2) << ',' << pressure_reader(dof) << ','
          << (pressure_old_reader ? (*pressure_old_reader)(dof) : 0.0);

      for (const auto dim_i : make_range(3))
      {
        const Real grad_value =
            dim_i < grad_readers.size() ? (*grad_readers[dim_i])(dof) : 0.0;
        out << ',' << grad_value;
      }

      for (const auto dim_i : make_range(3))
      {
        Real u_value = 0.0;
        if (dim_i < _momentum_systems.size() && sharp_rc)
        {
          const auto momentum_dof =
              elem_info->dofIndices()[_momentum_system_numbers[dim_i]][0];
          u_value = (*momentum_readers[dim_i])(momentum_dof);
        }
        out << ',' << u_value;
      }

      for (const auto dim_i : make_range(3))
      {
        Real hbya_value = 0.0;
        if (dim_i < _momentum_systems.size() && sharp_rc)
        {
          const auto momentum_dof =
              elem_info->dofIndices()[_momentum_system_numbers[dim_i]][0];
          hbya_value = sharp_rc->cellHbyARaw(dim_i, momentum_dof);
        }
        out << ',' << hbya_value;
      }

      for (const auto dim_i : make_range(3))
      {
        Real ainv_value = 0.0;
        if (dim_i < _momentum_systems.size() && sharp_rc)
        {
          const auto momentum_dof =
              elem_info->dofIndices()[_momentum_system_numbers[dim_i]][0];
          ainv_value = sharp_rc->cellAinvRaw(dim_i, momentum_dof);
        }
        out << ',' << ainv_value;
      }

      for (const auto dim_i : make_range(3))
      {
        Real neg_ainv_gradp_value = 0.0;
        if (dim_i < _momentum_systems.size() && sharp_rc)
        {
          const auto momentum_dof =
              elem_info->dofIndices()[_momentum_system_numbers[dim_i]][0];
          const Real ainv_value = sharp_rc->cellAinvRaw(dim_i, momentum_dof);
          const Real grad_value =
              dim_i < grad_readers.size() ? (*grad_readers[dim_i])(dof) : 0.0;
          neg_ainv_gradp_value = -ainv_value * grad_value;
        }
        out << ',' << neg_ainv_gradp_value;
      }

      out << ',' << (predictor_force_debug.face_based_pressure ? 1 : 0);
      for (const auto value : predictor_force_debug.pressure_force_density)
        out << ',' << value;
      for (const auto value : predictor_force_debug.body_force_density)
        out << ',' << value;
      for (const auto value : predictor_force_debug.cell_body_force_density)
        out << ',' << value;
      for (const auto value : predictor_force_debug.scalar_reconstructed_pressure_force_density)
        out << ',' << value;
      for (const auto value : predictor_force_debug.scalar_reconstructed_body_force_density)
        out << ',' << value;
      for (const auto value : predictor_force_debug.total_force_density)
        out << ',' << value;
      for (const auto value : predictor_force_debug.rhs_contribution)
        out << ',' << value;

      out << ',' << corr_debug.contributing_faces << ',' << (corr_debug.singular ? 1 : 0);
      for (const auto value : corr_debug.normal_matrix)
        out << ',' << value;
      for (const auto value : corr_debug.rhs)
        out << ',' << value;
      for (const auto value : corr_debug.solution)
        out << ',' << value;
      for (const auto value : corr_debug.delta_velocity)
        out << ',' << value;

      out << '\n';
    }
  }

  if (auto * sharp_rc = sharpInterfaceRC())
    sharp_rc->dumpPressureCorrectorFaceDebugCSV(file_base + iter_label + "_pressure_faces.csv");
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

unsigned int
ReducedPressurePIMPLESolve::computeVolumeFractionSubcycles() const
{
  unsigned int subcycles = _volume_fraction_subcycles;

  if (const auto * sharp_rc = sharpInterfaceRC())
  {
    const Real alpha_courant = sharp_rc->maxVolumeFractionCourant(_problem.dt());
    if (std::isfinite(alpha_courant) && alpha_courant > _volume_fraction_max_courant)
    {
      const auto required_subcycles = static_cast<unsigned int>(
          std::ceil(alpha_courant / _volume_fraction_max_courant));
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

std::vector<std::pair<unsigned int, Real>>
ReducedPressurePIMPLESolve::solveVolumeFractionSystems(const SolverParams & /*solver_params*/)
{
  std::vector<std::pair<unsigned int, Real>> residuals(
      _volume_fraction_system_names.size(), std::make_pair(0, 1.0));

  const Real global_dt = _problem.dt();
  const Real global_time = _problem.time();
  const Real global_time_old = _problem.timeOld();
  const unsigned int num_subcycles = computeVolumeFractionSubcycles();
  const Real subcycle_dt = global_dt / num_subcycles;
  const auto * sharp_rc = sharpInterfaceRC();
  const Real alpha_courant = sharp_rc ? sharp_rc->maxVolumeFractionCourant(global_dt) : 0.0;

  if (num_subcycles > _volume_fraction_subcycles)
    _console << name() << ": increasing alpha subcycles from " << _volume_fraction_subcycles
             << " to " << num_subcycles << " to keep alpha CFL <= "
             << _volume_fraction_max_courant << " at dt=" << global_dt << std::endl;

  if (_dump_pressure_outer_debug_csv && sharp_rc)
    _console << name() << ": alphaCo=" << alpha_courant << ", subcycles=" << num_subcycles
             << ", subcycle_dt=" << subcycle_dt
             << ", effective_alphaCo=" << alpha_courant / num_subcycles << std::endl;

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
    {
      if (_current_outer_iteration > 1)
        corrector->invalidateOuterCorrectionFluxSeed();
      corrector->resetSubcycleFluxes();
    }

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

    if (_dump_pressure_outer_debug_csv && corrector)
    {
      const auto audit = corrector->rhoPhiConsistencyAudit();
      _console << name() << ": rhoPhi audit for '" << corrector->variableName()
               << "' after alpha solve: L2=" << audit.l2_norm
               << ", max_abs=" << audit.max_abs_mismatch;
      if (audit.has_worst_face)
        _console << ", worst_face=" << audit.worst_face_id << " @ (" << audit.worst_face_centroid(0)
                 << ", " << audit.worst_face_centroid(1) << ", " << audit.worst_face_centroid(2)
                 << "), stored=" << audit.stored_rho_phi
                 << ", reconstructed=" << audit.reconstructed_rho_phi
                 << ", phi=" << audit.volumetric_phi
                 << ", alphaPhi=" << audit.limited_alpha_flux
                 << ", rho_g=" << audit.gas_density << ", rho_l=" << audit.liquid_density;
      _console << std::endl;
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
  Real first_stage_residual = std::numeric_limits<Real>::quiet_NaN();
  unsigned int piso_iteration_counter = 0;
  while (true)
  {
    _current_piso_iteration = piso_iteration_counter + 1;
    const bool subtract_updated_pressure = piso_iteration_counter == 0;
    preparePressureCorrectorState(subtract_updated_pressure);
    dumpPressureOuterDebugState("pre_pressure_solve");
    residual = applyPressureCorrectionStage(true, false, solver_params);
    if (piso_iteration_counter == 0)
      first_stage_residual = residual.second;
    if (!shouldContinuePISOIterations(
            piso_iteration_counter, residual.second, first_stage_residual))
      break;
    piso_iteration_counter++;
  }

  finalizePressureCorrectionStage();
  _current_piso_iteration = 0;

  return residual;
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
    // interFoam's initCorrectPhi is a flux-repair stage. It does not run the
    // full pEqn.H face-force path with phig. Keep startup cleanup on the bare
    // predictor flux, then restore the original reduced-pressure field after
    // extracting the continuity correction.
    sharp_rc->setSuppressStartupPressurePredictorFluxSources(true);
    sharp_rc->setSuppressExplicitHydrostaticPressureFlux(true);
  }

  preparePressureCorrectorState(subtract_updated_pressure);

  Moose::PetscSupport::petscSetOptions(_pressure_petsc_options, solver_params);

  const auto residuals = solvePressureCorrector();

  if (recompute_face_mass_flux)
  {
    _rc_uo->computeFaceMassFlux();

    if (sharp_rc)
      sharp_rc->applyAdditionalFaceMassFluxCorrection();
  }

  // Restore the user/equilibrium startup reduced-pressure field. Startup
  // continuity cleanup should repair phi like CorrectPhi/pcorr, not overwrite
  // the physical p_rgh field before the first real pressure equation.
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
  preparePressureCorrectorState(subtract_updated_pressure);
  dumpPressureOuterDebugState("pre_pressure_solve");

  const auto residuals = applyPressureCorrectionStage(recompute_face_mass_flux, false, solver_params);
  finalizePressureCorrectionStage();
  return residuals;
}

std::pair<unsigned int, Real>
ReducedPressurePIMPLESolve::applyPressureCorrectionStage(const bool recompute_face_mass_flux,
                                                         const bool relax_pressure_for_next_predictor,
                                                         const SolverParams & solver_params)
{

  Moose::PetscSupport::petscSetOptions(_pressure_petsc_options, solver_params);

  const auto residuals = solvePressureCorrector();

  auto & pressure_current_solution = *(_pressure_system.system().current_local_solution.get());
  auto & pressure_old_solution = *(_pressure_system.solutionPreviousNewton());

  _pressure_system.setSolution(pressure_current_solution);

  _pressure_system.computeGradients();

  if (recompute_face_mass_flux)
  {
    _rc_uo->computeFaceMassFlux();

    if (auto * sharp_rc = sharpInterfaceRC())
      sharp_rc->applyAdditionalFaceMassFluxCorrection();
  }

  if (relax_pressure_for_next_predictor)
  {
    if (auto * sharp_rc = sharpInterfaceRC())
      sharp_rc->computeProvisionalCellVelocity();
    else
      _rc_uo->computeCellVelocity();

    _rc_uo->updateVelocityBoundaryState();
    dumpPressureOuterDebugState("post_pressure_writeback");
  }

  // Mirror interFoam's pEqn.H ordering more closely: inner pressure-correction
  // stages should update phi/U from the exact pressure solve, and pressure
  // under-relaxation should only prepare the field for the next outer momentum
  // predictor after the final inner stage.
  if (relax_pressure_for_next_predictor)
  {
    NS::FV::relaxSolutionUpdate(
        pressure_current_solution, pressure_old_solution, _pressure_variable_relaxation);

    pressure_old_solution = pressure_current_solution;
    _pressure_system.setSolution(pressure_current_solution);
    _pressure_system.computeGradients();
  }

  return residuals;
}

void
ReducedPressurePIMPLESolve::finalizePressureCorrectionStage()
{
  auto & pressure_current_solution = *(_pressure_system.system().current_local_solution.get());
  auto & pressure_old_solution = *(_pressure_system.solutionPreviousNewton());

  if (auto * sharp_rc = sharpInterfaceRC())
    sharp_rc->computeProvisionalCellVelocity();
  else
    _rc_uo->computeCellVelocity();

  _rc_uo->updateVelocityBoundaryState();
  dumpPressureOuterDebugState("post_pressure_writeback");

  NS::FV::relaxSolutionUpdate(
      pressure_current_solution, pressure_old_solution, _pressure_variable_relaxation);

  pressure_old_solution = pressure_current_solution;
  _pressure_system.setSolution(pressure_current_solution);
  _pressure_system.computeGradients();
}
