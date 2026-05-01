#include "ReducedPressurePIMPLESolve.h"

#include "FEProblem.h"
#include "LinearSystem.h"
#include "SegregatedSolverUtils.h"
#include "SharpInterfaceCurvatureCalculator.h"
#include "SharpInterfaceRhieChowMassFlux.h"
#include "SharpInterfaceVOFMULESCorrector.h"
#include "TheWarehouse.h"
#include "libmesh/petsc_linear_solver.h"

#include <algorithm>
#include <fstream>
#include <iomanip>

using namespace libMesh;

namespace
{
struct StartupPressureCutAuditRow
{
  dof_id_type elem_id = DofObject::invalid_id;
  Point centroid;
  Real alpha = 0.0;
  Real rho_mixture = 0.0;
  Real reduced_pressure = 0.0;
  Real hydrostatic_shift = 0.0;
  Real total_pressure = 0.0;
};

struct MomentumProbeAuditPointSelection
{
  const ElemInfo * elem_info = nullptr;
  Point requested_point;
  Real distance = std::numeric_limits<Real>::max();
};

bool
elementIntersectsHorizontalCut(const Elem & elem, const unsigned int dim, const Real cut_y, const Real cut_z)
{
  Real min_y = std::numeric_limits<Real>::max();
  Real max_y = -std::numeric_limits<Real>::max();
  Real min_z = std::numeric_limits<Real>::max();
  Real max_z = -std::numeric_limits<Real>::max();

  for (const auto & node : elem.node_ref_range())
  {
    min_y = std::min(min_y, node(1));
    max_y = std::max(max_y, node(1));

    if (dim > 2)
    {
      min_z = std::min(min_z, node(2));
      max_z = std::max(max_z, node(2));
    }
  }

  const bool matches_y = cut_y >= min_y - TOLERANCE && cut_y <= max_y + TOLERANCE;
  const bool matches_z = dim <= 2 || (cut_z >= min_z - TOLERANCE && cut_z <= max_z + TOLERANCE);
  return matches_y && matches_z;
}

Real
clampUnitInterval(const Real value)
{
  return std::max(0.0, std::min(1.0, value));
}
} // namespace

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
  params.addParam<bool>(
      "audit_stage_diagnostics",
      false,
      "Whether to print stage-by-stage reduced-pressure / sharp-interface diagnostics on every "
      "outer iteration, independent of the handoff-debug outer counter.");
  params.addRangeCheckedParam<Real>(
      "audit_stage_diagnostics_start_time",
      0.0,
      "audit_stage_diagnostics_start_time>=0",
      "Start printing the per-outer stage diagnostics once the physical simulation time reaches "
      "this value.");
  params.addParam<bool>(
      "startup_pressure_cut_audit",
      false,
      "Whether to dump an explicit horizontal pressure cut directly from the executioner around "
      "the startup CorrectPhi / continuity cleanup path.");
  params.addParam<std::string>(
      "startup_pressure_cut_audit_file_base",
      "",
      "File base for the explicit startup pressure-cut audit CSVs. The executioner appends the "
      "stage label and '.csv'.");
  params.addParam<Real>(
      "startup_pressure_cut_audit_y",
      0.0,
      "Y coordinate of the explicit startup pressure-cut audit.");
  params.addParam<Real>(
      "startup_pressure_cut_audit_z",
      0.0,
      "Z coordinate of the explicit startup pressure-cut audit for 3D cases.");
  params.addParam<Real>(
      "startup_pressure_cut_audit_reference_pressure",
      0.0,
      "Reference total pressure added to the hydrostatic reconstruction in the explicit startup "
      "pressure-cut audit.");
  params.addParam<Real>(
      "startup_pressure_cut_audit_liquid_density",
      0.0,
      "Liquid density used to reconstruct total pressure in the explicit startup pressure-cut "
      "audit.");
  params.addParam<Real>(
      "startup_pressure_cut_audit_gas_density",
      0.0,
      "Gas density used to reconstruct total pressure in the explicit startup pressure-cut "
      "audit.");
  params.addParam<RealVectorValue>(
      "startup_pressure_cut_audit_gravity",
      RealVectorValue(0, 0, 0),
      "Gravity vector used to reconstruct total pressure in the explicit startup pressure-cut "
      "audit.");
  params.addParam<Point>(
      "startup_pressure_cut_audit_reference_pressure_point",
      Point(),
      "Reference point at which the hydrostatic contribution is zero in the explicit startup "
      "pressure-cut audit.");
  params.addParam<bool>("momentum_probe_audit",
                        false,
                        "Whether to write per-stage momentum-term probe diagnostics for selected "
                        "cells to a CSV file.");
  params.addParam<std::string>("momentum_probe_audit_file_base",
                               "",
                               "File base for the momentum probe audit CSV.");
  params.addParam<std::vector<Point>>(
      "momentum_probe_points",
      {},
      "Probe points whose nearest cell centroids are sampled in the momentum audit.");
  params.addRangeCheckedParam<Real>("momentum_probe_audit_start_time",
                                    0.0,
                                    "momentum_probe_audit_start_time>=0",
                                    "Only emit momentum-probe rows at or after this time.");
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
  params.addParam<bool>(
      "use_vof_rho_phi_during_momentum_predictor",
      false,
      "Debug hook that keeps alpha-consistent rhoPhi enabled during the sharp-interface "
      "momentum predictor after alpha has been solved. The default keeps the current parity "
      "branch, which temporarily falls back to the raw Rhie-Chow mass flux during the "
      "predictor.");
  params.addParamNamesToGroup(
      "volume_fraction_systems volume_fraction_equation_relaxation volume_fraction_petsc_options "
      "volume_fraction_petsc_options_iname volume_fraction_petsc_options_value "
      "volume_fraction_absolute_tolerance volume_fraction_l_tol volume_fraction_l_abs_tol "
      "volume_fraction_l_max_its should_solve_volume_fractions volume_fraction_min_value "
      "volume_fraction_max_value volume_fraction_subcycles volume_fraction_outer_corrections "
      "startup_pressure_initialization perform_startup_hydrostatic_initialization "
      "suppress_explicit_hydrostatic_flux_during_seeded_startup startup_flux_corrections",
      "Volume Fraction Equations");
  params.addParamNamesToGroup("startup_pressure_cut_audit startup_pressure_cut_audit_file_base "
                              "startup_pressure_cut_audit_y startup_pressure_cut_audit_z "
                              "startup_pressure_cut_audit_reference_pressure "
                              "startup_pressure_cut_audit_liquid_density "
                              "startup_pressure_cut_audit_gas_density "
                              "startup_pressure_cut_audit_gravity "
                              "startup_pressure_cut_audit_reference_pressure_point",
                              "Pressure Audit");
  params.addParamNamesToGroup("momentum_probe_audit momentum_probe_audit_file_base "
                              "momentum_probe_points momentum_probe_audit_start_time",
                              "Pressure Audit");
  params.addParamNamesToGroup("audit_outer_handoff_stages audit_outer_handoff_after_outer "
                              "audit_stage_diagnostics audit_stage_diagnostics_start_time "
                              "skip_momentum_predictor_after_outer freeze_alpha_after_outer "
                              "skip_pressure_velocity_writeback_from_outer "
                              "use_vof_rho_phi_during_momentum_predictor",
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
    _suppress_explicit_hydrostatic_flux_during_seeded_startup(
        getParam<bool>("suppress_explicit_hydrostatic_flux_during_seeded_startup")),
    _startup_flux_corrections(getParam<unsigned int>("startup_flux_corrections")),
    _audit_outer_handoff_stages(getParam<bool>("audit_outer_handoff_stages")),
    _audit_outer_handoff_after_outer(getParam<unsigned int>("audit_outer_handoff_after_outer")),
    _audit_stage_diagnostics(getParam<bool>("audit_stage_diagnostics")),
    _audit_stage_diagnostics_start_time(getParam<Real>("audit_stage_diagnostics_start_time")),
    _startup_pressure_cut_audit(getParam<bool>("startup_pressure_cut_audit")),
    _startup_pressure_cut_audit_file_base(getParam<std::string>("startup_pressure_cut_audit_file_base")),
    _startup_pressure_cut_audit_y(getParam<Real>("startup_pressure_cut_audit_y")),
    _startup_pressure_cut_audit_z(getParam<Real>("startup_pressure_cut_audit_z")),
    _startup_pressure_cut_audit_reference_pressure(
        getParam<Real>("startup_pressure_cut_audit_reference_pressure")),
    _startup_pressure_cut_audit_liquid_density(
        getParam<Real>("startup_pressure_cut_audit_liquid_density")),
    _startup_pressure_cut_audit_gas_density(getParam<Real>("startup_pressure_cut_audit_gas_density")),
    _startup_pressure_cut_audit_gravity(getParam<RealVectorValue>("startup_pressure_cut_audit_gravity")),
    _startup_pressure_cut_audit_reference_pressure_point(
        getParam<Point>("startup_pressure_cut_audit_reference_pressure_point")),
    _momentum_probe_audit(getParam<bool>("momentum_probe_audit")),
    _momentum_probe_audit_file_base(getParam<std::string>("momentum_probe_audit_file_base")),
    _momentum_probe_points(getParam<std::vector<Point>>("momentum_probe_points")),
    _momentum_probe_audit_start_time(getParam<Real>("momentum_probe_audit_start_time")),
    _skip_momentum_predictor_after_outer(
        getParam<unsigned int>("skip_momentum_predictor_after_outer")),
    _freeze_alpha_after_outer(getParam<unsigned int>("freeze_alpha_after_outer")),
    _skip_pressure_velocity_writeback_from_outer(
        getParam<unsigned int>("skip_pressure_velocity_writeback_from_outer")),
    _use_vof_rho_phi_during_momentum_predictor(
        getParam<bool>("use_vof_rho_phi_during_momentum_predictor"))
{
  _startup_pressure_initialization =
      getParam<MooseEnum>("startup_pressure_initialization").operator std::string();
  if (parameters().isParamSetByUser("perform_startup_hydrostatic_initialization"))
    _startup_pressure_initialization =
        getParam<bool>("perform_startup_hydrostatic_initialization") ? "equilibrium-seed" : "none";

  if (_volume_fraction_min_value > _volume_fraction_max_value)
    paramError("volume_fraction_max_value",
               "volume_fraction_max_value must be >= volume_fraction_min_value.");

  if (_startup_pressure_cut_audit)
  {
    if (_startup_pressure_cut_audit_file_base.empty())
      paramError("startup_pressure_cut_audit_file_base",
                 "startup_pressure_cut_audit_file_base must be provided when "
                 "startup_pressure_cut_audit=true.");

    if (_startup_pressure_cut_audit_liquid_density <= 0.0)
      paramError("startup_pressure_cut_audit_liquid_density",
                 "startup_pressure_cut_audit_liquid_density must be positive when "
                 "startup_pressure_cut_audit=true.");

    if (_startup_pressure_cut_audit_gas_density <= 0.0)
      paramError("startup_pressure_cut_audit_gas_density",
                 "startup_pressure_cut_audit_gas_density must be positive when "
                 "startup_pressure_cut_audit=true.");
  }

  if (_momentum_probe_audit)
  {
    if (_momentum_probe_audit_file_base.empty())
      paramError("momentum_probe_audit_file_base",
                 "momentum_probe_audit_file_base must be provided when momentum_probe_audit=true.");

    if (_momentum_probe_points.empty())
      paramError("momentum_probe_points",
                 "At least one point must be provided when momentum_probe_audit=true.");
  }

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

bool
ReducedPressurePIMPLESolve::startupPressureCutAuditEnabled() const
{
  return _startup_pressure_cut_audit;
}

bool
ReducedPressurePIMPLESolve::momentumProbeAuditEnabled() const
{
  return _momentum_probe_audit;
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
    const bool stage_diagnostics = shouldPrintStageDiagnostics(simple_iteration_counter);
    const bool probe_stage = momentumProbeAuditEnabled() &&
                             _problem.time() >= _momentum_probe_audit_start_time;

    if (stage_diagnostics)
      printOuterIterationDiagnostics(simple_iteration_counter, "outer_entry");
    else if (probe_stage)
      writeMomentumProbeAudit(simple_iteration_counter, "outer_entry");

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
      else
      {
        for (const auto i : index_range(_volume_fraction_systems))
          ns_residuals[volume_fraction_indices[i]] = std::make_pair(0u, 0.0);
        if (stage_diagnostics)
          _console << "Outer-loop handoff audit: outer_iter=" << simple_iteration_counter
                   << " freezing alpha/rhoPhi refresh after outer iteration "
                   << _freeze_alpha_after_outer << std::endl;
      }
    }

    if (stage_diagnostics)
      printOuterIterationDiagnostics(simple_iteration_counter, "after_alpha");
    else if (probe_stage)
      writeMomentumProbeAudit(simple_iteration_counter, "after_alpha");

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
        if (stage_diagnostics)
          _console << "Outer-loop handoff audit: outer_iter=" << simple_iteration_counter
                   << " skipping momentum predictor after outer iteration "
                   << _skip_momentum_predictor_after_outer << std::endl;
      }
    }
    else if (_should_solve_pressure && !_momentum_systems.empty() && _rc_uo)
      assembleMomentumPredictorOnly();

    if (stage_diagnostics)
      printOuterIterationDiagnostics(simple_iteration_counter, "after_predictor");
    else if (probe_stage)
      writeMomentumProbeAudit(simple_iteration_counter, "after_predictor");

    if (_should_solve_pressure)
      ns_residuals[pressure_index] = correctVelocity(true, true, solver_params);

    if (_print_fields || stage_diagnostics)
      printOuterIterationDiagnostics(simple_iteration_counter, "after_pressure");
    else if (probe_stage)
      writeMomentumProbeAudit(simple_iteration_counter, "after_pressure");

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
      sharp_rc && _current_outer_iteration > 1 && sharp_rc->useVOFRhoPhi() &&
      !sharp_rc->hasOuterIterationConvectiveState() &&
      !_use_vof_rho_phi_during_momentum_predictor;

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
  return shouldPrintStageDiagnostics(_current_outer_iteration);
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
      !sharp_rc->hasOuterIterationConvectiveState() &&
      !_use_vof_rho_phi_during_momentum_predictor;

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
    if (startupPressureCutAuditEnabled())
      writeStartupPressureCutAudit("startup_before_corrections");

    assembleMomentumPredictorOnly();
    _rc_uo->initFaceMassFlux();
    performStartupContinuityCorrections(solver_params);
    synchronizeSystemState(_pressure_system);

    if (startupPressureCutAuditEnabled())
      writeStartupPressureCutAudit("startup_after_corrections");
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

    if (startupPressureCutAuditEnabled())
      writeStartupPressureCutAudit("startup_after_correction_" + std::to_string(startup_it + 1));
  }

  _problem.execute(EXEC_NONLINEAR);
}

void
ReducedPressurePIMPLESolve::writeStartupPressureCutAudit(const std::string & label) const
{
  if (!startupPressureCutAuditEnabled())
    return;

  if (_problem.comm().size() > 1)
    mooseWarning("startup_pressure_cut_audit currently writes only locally-owned cells. Run the "
                 "audit in serial for a complete cut.");

  const auto & pressure_solution = *(_pressure_system.system().current_local_solution);
  const auto pressure_system_number = _pressure_system.number();

  const LinearSystem * alpha_system =
      _volume_fraction_systems.empty() ? nullptr : _volume_fraction_systems.front();
  const NumericVector<Number> * alpha_solution =
      alpha_system ? alpha_system->system().current_local_solution.get() : nullptr;
  const unsigned int alpha_system_number = alpha_system ? alpha_system->number() : invalid_uint;

  std::vector<StartupPressureCutAuditRow> rows;
  rows.reserve(_problem.mesh().elemInfoVector().size());

  const auto dim = _problem.mesh().dimension();
  for (const auto & elem_info : _problem.mesh().elemInfoVector())
  {
    if (!elem_info)
      continue;

    const auto * const elem = elem_info->elem();
    if (!elem || elem->processor_id() != _problem.processor_id())
      continue;

    if (!elementIntersectsHorizontalCut(*elem, dim, _startup_pressure_cut_audit_y, _startup_pressure_cut_audit_z))
      continue;

    const auto & dof_indices = elem_info->dofIndices();
    if (pressure_system_number >= dof_indices.size() || dof_indices[pressure_system_number].empty())
      continue;

    const auto pressure_dof = dof_indices[pressure_system_number][0];
    if (pressure_dof == DofObject::invalid_id)
      continue;

    StartupPressureCutAuditRow row;
    row.elem_id = elem->id();
    row.centroid = elem->vertex_average();
    row.reduced_pressure = pressure_solution(pressure_dof);

    if (alpha_solution && alpha_system_number < dof_indices.size() && !dof_indices[alpha_system_number].empty())
    {
      const auto alpha_dof = dof_indices[alpha_system_number][0];
      if (alpha_dof != DofObject::invalid_id)
        row.alpha = clampUnitInterval((*alpha_solution)(alpha_dof));
    }

    row.rho_mixture = row.alpha * _startup_pressure_cut_audit_liquid_density +
                      (1.0 - row.alpha) * _startup_pressure_cut_audit_gas_density;
    const Point offset = row.centroid - _startup_pressure_cut_audit_reference_pressure_point;
    row.hydrostatic_shift = _startup_pressure_cut_audit_reference_pressure +
                            row.rho_mixture *
                                (_startup_pressure_cut_audit_gravity(0) * offset(0) +
                                 _startup_pressure_cut_audit_gravity(1) * offset(1) +
                                 _startup_pressure_cut_audit_gravity(2) * offset(2));
    row.total_pressure = row.reduced_pressure + row.hydrostatic_shift;
    rows.push_back(row);
  }

  std::sort(rows.begin(),
            rows.end(),
            [](const StartupPressureCutAuditRow & lhs, const StartupPressureCutAuditRow & rhs)
            {
              if (lhs.centroid(0) != rhs.centroid(0))
                return lhs.centroid(0) < rhs.centroid(0);
              if (lhs.centroid(1) != rhs.centroid(1))
                return lhs.centroid(1) < rhs.centroid(1);
              return lhs.elem_id < rhs.elem_id;
            });

  if (_problem.comm().rank() != 0)
    return;

  std::ofstream out(_startup_pressure_cut_audit_file_base + "_" + label + ".csv");
  if (!out.is_open())
    mooseError("Unable to open startup pressure-cut audit file base '",
               _startup_pressure_cut_audit_file_base,
               "' for stage '",
               label,
               "'.");

  out << std::setprecision(17);
  out << "elem_id,x,y,z,alpha,rho_mixture,reduced_pressure,hydrostatic_shift,total_pressure\n";
  for (const auto & row : rows)
    out << row.elem_id << ',' << row.centroid(0) << ',' << row.centroid(1) << ','
        << row.centroid(2) << ',' << row.alpha << ',' << row.rho_mixture << ','
        << row.reduced_pressure << ',' << row.hydrostatic_shift << ',' << row.total_pressure
        << '\n';
}

void
ReducedPressurePIMPLESolve::writeMomentumProbeAudit(const unsigned int simple_iteration_counter,
                                                    const std::string & stage_label) const
{
  if (!momentumProbeAuditEnabled() || _problem.time() < _momentum_probe_audit_start_time)
    return;

  auto * const sharp_rc = sharpInterfaceRC();
  if (!sharp_rc)
    return;

  if (_problem.comm().size() > 1 && !_momentum_probe_audit_parallel_warning_emitted)
  {
    mooseWarning("momentum_probe_audit currently selects probe cells from locally-owned elements. "
                 "Run in serial for deterministic probe selection.");
    _momentum_probe_audit_parallel_warning_emitted = true;
  }

  std::vector<MomentumProbeAuditPointSelection> selections(_momentum_probe_points.size());
  for (const auto probe_i : index_range(_momentum_probe_points))
    selections[probe_i].requested_point = _momentum_probe_points[probe_i];

  for (const auto & elem_info : _problem.mesh().elemInfoVector())
  {
    if (!elem_info)
      continue;

    const auto * const elem = elem_info->elem();
    if (!elem || elem->processor_id() != _problem.processor_id())
      continue;

    const Point centroid = elem->vertex_average();
    for (const auto probe_i : index_range(_momentum_probe_points))
    {
      const Real distance = (centroid - _momentum_probe_points[probe_i]).norm();
      if (distance < selections[probe_i].distance)
      {
        selections[probe_i].elem_info = elem_info;
        selections[probe_i].distance = distance;
      }
    }
  }

  std::vector<const ElemInfo *> elem_infos;
  elem_infos.reserve(selections.size());
  for (const auto & selection : selections)
    elem_infos.push_back(selection.elem_info);

  std::vector<SharpInterfaceRhieChowMassFlux::MomentumProbeSample> samples;
  sharp_rc->collectMomentumProbeSamples(elem_infos, samples);

  const LinearSystem * alpha_system =
      _volume_fraction_systems.empty() ? nullptr : _volume_fraction_systems.front();
  const NumericVector<Number> * alpha_solution =
      alpha_system ? alpha_system->system().current_local_solution.get() : nullptr;
  const unsigned int alpha_system_number = alpha_system ? alpha_system->number() : invalid_uint;

  const auto mode =
      _momentum_probe_audit_header_written ? std::ios::out | std::ios::app : std::ios::out;
  std::ofstream out(_momentum_probe_audit_file_base + ".csv", mode);
  if (!out.is_open())
    mooseError("Unable to open momentum probe audit file base '",
               _momentum_probe_audit_file_base,
               "'.");

  out << std::setprecision(17);
  if (!_momentum_probe_audit_header_written)
  {
    out << "time,timestep,outer_iteration,stage,probe_index,requested_x,requested_y,requested_z,"
           "elem_id,centroid_x,centroid_y,centroid_z,distance,alpha,rho,pressure,"
           "grad_p_x,grad_p_y,grad_p_z,pressure_force_x,pressure_force_y,pressure_force_z,"
           "body_force_x,body_force_y,body_force_z,total_force_x,total_force_y,total_force_z,"
           "hbya_raw_x,hbya_raw_y,hbya_raw_z,predictor_velocity_x,predictor_velocity_y,"
           "predictor_velocity_z,pressure_delta_u_x,pressure_delta_u_y,pressure_delta_u_z,"
           "writeback_velocity_x,writeback_velocity_y,writeback_velocity_z,current_velocity_x,"
           "current_velocity_y,current_velocity_z\n";
    _momentum_probe_audit_header_written = true;
  }

  for (const auto probe_i : index_range(samples))
  {
    const auto & selection = selections[probe_i];
    const auto & sample = samples[probe_i];
    Real alpha = 0.0;
    if (sample.valid && alpha_solution && selection.elem_info &&
        alpha_system_number < selection.elem_info->dofIndices().size())
    {
      const auto & alpha_dofs = selection.elem_info->dofIndices()[alpha_system_number];
      if (!alpha_dofs.empty())
        alpha = clampUnitInterval((*alpha_solution)(alpha_dofs[0]));
    }

    out << _problem.time() << ',' << _problem.timeStep() << ',' << simple_iteration_counter << ','
        << stage_label << ',' << probe_i << ',' << selection.requested_point(0) << ','
        << selection.requested_point(1) << ',' << selection.requested_point(2) << ',';

    if (!sample.valid)
    {
      out << DofObject::invalid_id << ",0,0,0," << selection.distance
          << ",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n";
      continue;
    }

    out << sample.elem_id << ',' << sample.centroid(0) << ',' << sample.centroid(1) << ','
        << sample.centroid(2) << ',' << selection.distance << ',' << alpha << ',' << sample.rho
        << ',' << sample.pressure << ',' << sample.grad_p(0) << ',' << sample.grad_p(1) << ','
        << sample.grad_p(2) << ',' << sample.pressure_force(0) << ','
        << sample.pressure_force(1) << ',' << sample.pressure_force(2) << ','
        << sample.body_force(0) << ',' << sample.body_force(1) << ','
        << sample.body_force(2) << ',' << sample.total_force(0) << ','
        << sample.total_force(1) << ',' << sample.total_force(2) << ',' << sample.hbya_raw(0)
        << ',' << sample.hbya_raw(1) << ',' << sample.hbya_raw(2) << ','
        << sample.predictor_velocity(0) << ',' << sample.predictor_velocity(1) << ','
        << sample.predictor_velocity(2) << ',' << sample.pressure_coupled_delta_velocity(0) << ','
        << sample.pressure_coupled_delta_velocity(1) << ','
        << sample.pressure_coupled_delta_velocity(2) << ',' << sample.writeback_velocity(0) << ','
        << sample.writeback_velocity(1) << ',' << sample.writeback_velocity(2) << ','
        << sample.current_velocity(0) << ',' << sample.current_velocity(1) << ','
        << sample.current_velocity(2) << '\n';
  }
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
           << "): time=" << _problem.time() << " dt=" << _problem.dt()
           << " timestep=" << _problem.timeStep() << std::endl;

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

  for (const auto & system_name : _volume_fraction_system_names)
    if (auto * corrector = sharpInterfaceVOFCorrector(system_name))
    {
      const auto liquid_volume_audit = corrector->liquidVolumeAudit();
      _console << "  VOF liquid-volume audit " << corrector->systemName()
               << ": liquid_volume=" << liquid_volume_audit.current
               << " delta_timestep_old="
               << liquid_volume_audit.current - liquid_volume_audit.timestep_old;
      if (std::abs(liquid_volume_audit.timestep_old) > libMesh::TOLERANCE)
        _console << " rel_delta_timestep_old="
                 << (liquid_volume_audit.current - liquid_volume_audit.timestep_old) /
                        liquid_volume_audit.timestep_old;
      if (liquid_volume_audit.has_previous_outer)
      {
        _console << " delta_prev_outer="
                 << liquid_volume_audit.current - liquid_volume_audit.previous_outer;
        if (std::abs(liquid_volume_audit.previous_outer) > libMesh::TOLERANCE)
          _console << " rel_delta_prev_outer="
                   << (liquid_volume_audit.current - liquid_volume_audit.previous_outer) /
                          liquid_volume_audit.previous_outer;
      }
      _console << std::endl;

      const auto rho_phi_audit = corrector->rhoPhiConsistencyAudit();
      _console << "  VOF rhoPhi audit " << corrector->systemName()
               << ": |rhoPhi-rhoPhi(phi,alphaPhi)|_2=" << rho_phi_audit.l2_norm
               << " max|mismatch|=" << rho_phi_audit.max_abs_mismatch;
      if (rho_phi_audit.has_worst_face)
        _console << " worst_face_id=" << rho_phi_audit.worst_face_id
                 << " centroid=" << rho_phi_audit.worst_face_centroid
                 << " stored_rho_phi=" << rho_phi_audit.stored_rho_phi
                 << " reconstructed_rho_phi=" << rho_phi_audit.reconstructed_rho_phi
                 << " volumetric_phi=" << rho_phi_audit.volumetric_phi
                 << " limited_alpha_flux=" << rho_phi_audit.limited_alpha_flux
                 << " gas_density=" << rho_phi_audit.gas_density
                 << " liquid_density=" << rho_phi_audit.liquid_density;
      _console << std::endl;
    }

  if (auto * sharp_rc = sharpInterfaceRC())
  {
    sharp_rc->printPressureCoupledVelocityCorrectionAudit(stage_label);
    if (stage_label == "after_alpha" && sharp_rc->useFaceBasedPredictorBodyForce())
      sharp_rc->auditRepresentativePredictorBodyForce();
  }

  writeMomentumProbeAudit(simple_iteration_counter, stage_label);
}

bool
ReducedPressurePIMPLESolve::shouldPrintStageDiagnostics(
    const unsigned int simple_iteration_counter) const
{
  return (_audit_outer_handoff_stages &&
          simple_iteration_counter > _audit_outer_handoff_after_outer) ||
         (_audit_stage_diagnostics && _problem.time() >= _audit_stage_diagnostics_start_time);
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
    {
      sharp_rc->applyAdditionalFaceMassFluxCorrection();
      if (_print_fields || sharp_rc->suppressExplicitHydrostaticPressureFlux())
        sharp_rc->auditRepresentativeHorizontalFaceReconstruction();
    }
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
  const bool audit_stage = shouldPrintStageDiagnostics(_current_outer_iteration);
  const bool probe_stage =
      momentumProbeAuditEnabled() && _problem.time() >= _momentum_probe_audit_start_time;
  if (auto * sharp_rc = sharpInterfaceRC())
    sharp_rc->clearPressureCoupledVelocityCorrectionAudit();

  const bool skip_pressure_velocity_writeback =
      _current_outer_iteration &&
      _skip_pressure_velocity_writeback_from_outer &&
      _current_outer_iteration >= _skip_pressure_velocity_writeback_from_outer;

  preparePressureCorrectorState(subtract_updated_pressure);

  if (audit_stage)
    _rc_uo->auditPressureBoundaryGradientState("after_boundary_gradient_update");

  Moose::PetscSupport::petscSetOptions(_pressure_petsc_options, solver_params);

  const auto residuals = solvePressureCorrector();

  auto & pressure_current_solution = *(_pressure_system.system().current_local_solution.get());
  auto & pressure_old_solution = *(_pressure_system.solutionPreviousNewton());

  _pressure_system.setSolution(pressure_current_solution);

  _pressure_system.computeGradients();

  if (audit_stage)
    printOuterIterationDiagnostics(_current_outer_iteration, "after_pressure_solve");
  else if (probe_stage)
    writeMomentumProbeAudit(_current_outer_iteration, "after_pressure_solve");

  if (recompute_face_mass_flux)
  {
    _rc_uo->computeFaceMassFlux();

    if (auto * sharp_rc = sharpInterfaceRC())
      sharp_rc->applyAdditionalFaceMassFluxCorrection();

    if (audit_stage)
      printOuterIterationDiagnostics(_current_outer_iteration, "after_face_mass_flux");
    else if (probe_stage)
      writeMomentumProbeAudit(_current_outer_iteration, "after_face_mass_flux");
  }

  if (!skip_pressure_velocity_writeback)
  {
    if (auto * sharp_rc = sharpInterfaceRC())
      sharp_rc->computeProvisionalCellVelocity();
    else
      _rc_uo->computeCellVelocity();

    if (audit_stage)
      printOuterIterationDiagnostics(_current_outer_iteration, "after_pressure_velocity_writeback");
    else if (probe_stage)
      writeMomentumProbeAudit(_current_outer_iteration, "after_pressure_velocity_writeback");

    if (auto * sharp_rc = sharpInterfaceRC())
    {
      if (audit_stage)
        sharp_rc->auditRepresentativeHorizontalFaceReconstruction();
      if (_print_fields || sharp_rc->suppressExplicitHydrostaticPressureFlux())
        sharp_rc->auditRepresentativeHorizontalFaceReconstruction();
    }

    _rc_uo->updateVelocityBoundaryState();

    if (audit_stage)
      _rc_uo->auditPressureBoundaryGradientState("after_boundary_refresh");

    if (audit_stage)
      printOuterIterationDiagnostics(_current_outer_iteration, "after_boundary_refresh");
    else if (probe_stage)
      writeMomentumProbeAudit(_current_outer_iteration, "after_boundary_refresh");
  }
  else if (audit_stage)
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

  if (audit_stage)
    _rc_uo->auditPressureBoundaryGradientState("after_pressure_relaxation");

  return residuals;
}
