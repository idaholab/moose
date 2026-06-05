#include "ReducedPressurePIMPLESolve.h"

#include "FEProblem.h"
#include "LinearSystem.h"
#include "MooseApp.h"
#include "PetscVectorReader.h"
#include "SegregatedSolverUtils.h"
#include "ConservativeSharpInterfaceRhieChowMassFlux.h"
#include "ConservativeSharpInterfaceCurvatureCalculator.h"
#include "ConservativeSharpInterfaceVOFMULESCorrector.h"
#include "TheWarehouse.h"
#include "libmesh/petsc_linear_solver.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <unordered_map>
using namespace libMesh;

namespace
{

void
closeVectorWithTrace(const std::string & label, NumericVector<Number> * vector)
{
  if (!vector)
  {
    std::cerr << label << " vector=null" << std::endl;
    return;
  }

  std::cerr << label << " close begin" << std::endl;
  vector->close();
  std::cerr << label << " close end" << std::endl;
}

struct SharpCellDebugKeys
{
  const char * cell;
  const char * velocity;
  const char * pre_writeback;
  const char * delta_velocity;
  const char * post_writeback;
  const char * hbya_live;
  const char * ainv_live;
  const char * predictor_live;
  const char * predictor_cached;
  const char * predictor_velocity_view;
  const char * hbya_velocity_view;
};

constexpr SharpCellDebugKeys worst_face_cell_debug_keys = {
    "_cell_id=",
    "_u",
    "_pre_writeback_u",
    "_pressure_delta_u",
    "_post_writeback_u",
    "_hbya_live",
    "_ainv_live",
    "_pred_base_live",
    "_pred_base_cached",
    "_pred_base_uview",
    "_hbya_uview"};

constexpr SharpCellDebugKeys pressure_debug_face_cell_debug_keys = {
    "_cell=",
    "_u",
    "_pre_u",
    "_delta_u",
    "_post_u",
    "_hbya_live",
    nullptr,
    "_pred_live",
    "_pred_cached",
    "_pred_uview",
    "_hbya_uview"};

void
appendSharpCellDebug(std::ostream & out,
                     const char * separator,
                     const char * label,
                     const ElemInfo * elem_info,
                     const std::vector<unsigned int> & momentum_system_numbers,
                     const ConservativeSharpInterfaceRhieChowMassFlux & sharp_rc,
                     const SharpCellDebugKeys & keys)
{
  if (!elem_info)
    return;

  out << separator << label << keys.cell << elem_info->elem()->id();
  for (const auto dim_i : index_range(momentum_system_numbers))
  {
    const auto momentum_dof = elem_info->dofIndices()[momentum_system_numbers[dim_i]][0];
    out << separator << label << keys.velocity << dim_i << '='
        << sharp_rc.debugCurrentVelocityComponent(*elem_info, dim_i) << separator << label
        << keys.pre_writeback << dim_i << '='
        << sharp_rc.debugLastWritebackPreVelocityComponent(*elem_info, dim_i) << separator << label
        << keys.delta_velocity << dim_i << '='
        << sharp_rc.debugLastWritebackPressureDeltaVelocityComponent(*elem_info, dim_i) << separator
        << label << keys.post_writeback << dim_i << '='
        << sharp_rc.debugLastWritebackPostVelocityComponent(*elem_info, dim_i) << separator << label
        << keys.hbya_live << dim_i << '=' << sharp_rc.debugCellHbyARaw(dim_i, momentum_dof);

    if (keys.ainv_live)
      out << separator << label << keys.ainv_live << dim_i << '='
          << sharp_rc.debugCellAinvRaw(dim_i, momentum_dof);

    out << separator << label << keys.predictor_live << dim_i << '='
        << sharp_rc.debugLivePredictorBaseRawComponent(*elem_info, dim_i) << separator << label
        << keys.predictor_cached << dim_i << '='
        << sharp_rc.debugCachedPredictorBaseRawComponent(*elem_info, dim_i) << separator << label
        << keys.predictor_velocity_view << dim_i << '='
        << sharp_rc.debugVelocityPredictorBaseRawComponent(*elem_info, dim_i) << separator
        << label << keys.hbya_velocity_view << dim_i << '='
        << sharp_rc.debugVelocityPredictorHbyAComponent(*elem_info, dim_i);
  }
}

} // namespace

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
  params.addParam<bool>(
      "volume_fraction_outer_corrections",
      false,
      "Deprecated compatibility switch. The reduced-pressure sharp-interface PIMPLE path now "
      "always refreshes the volume-fraction system(s) and alpha-owned rhoPhi on every outer "
      "correction to match interFoam's outer-loop architecture.");
  MooseEnum startup_pressure_initialization("none projection-only", "projection-only");
  params.addParam<MooseEnum>(
      "startup_pressure_initialization",
      startup_pressure_initialization,
      "Startup reduced-pressure initialization policy on the first time step. Use "
      "'projection-only' to mimic interFoam's initCorrectPhi-style startup projection without "
      "overwriting the user-supplied reduced-pressure field, or 'none' to skip startup pressure "
      "cleanup entirely.");
  params.addRangeCheckedParam<unsigned int>(
      "startup_flux_corrections",
      1,
      "startup_flux_corrections>0",
      "Number of pressure-only startup cleanup / projection corrections applied when "
      "startup_pressure_initialization is not 'none'.");
  params.addParam<unsigned int>(
      "num_pressure_nonorthogonal_correctors",
      0,
      "Number of additional non-final pressure equation solves inside each pressure-corrector "
      "stage. This follows reference solver's nNonOrthogonalCorrectors convention: 0 means one final "
      "pressure solve, 1 means one non-final solve followed by one final solve, etc.");
  params.addParam<unsigned int>(
      "n_nonorthogonal_correctors",
      "reference-solver-style alias for num_pressure_nonorthogonal_correctors.");
  params.addParam<bool>(
      "dump_pressure_outer_debug_csv",
      false,
      "Whether to dump the pre-pressure-solve reduced-pressure cell state and sharp-interface "
      "face-operator state to CSV for the first few outer iterations.");
  params.addParam<bool>(
      "print_pressure_hbya_parity_summary",
      false,
      "Whether to print one compact sharp-interface rhoPhi/HbyA/pressure-writeback parity summary "
      "line after each pressure-corrected state publication.");
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
  params.addParam<std::vector<unsigned int>>(
      "pressure_debug_face_ids",
      {},
      "Optional list of face ids to dump in the reduced-pressure debug trace at each stage.");
  params.addParam<std::vector<Point>>(
      "pressure_debug_face_points",
      {},
      "Optional list of face-centroid probe points used to resolve the nearest face ids for the "
      "reduced-pressure debug trace. These face ids are unioned with pressure_debug_face_ids.");
  params.addParam<bool>(
      "audit_momentum_predictor_rebuild",
      false,
      "Whether to print a pre-solve momentum predictor rebuild audit, including a term-by-term "
      "residual breakdown of the assembled momentum operator.");
  params.addRangeCheckedParam<unsigned int>(
      "audit_momentum_predictor_rebuild_start_timestep",
      1,
      "audit_momentum_predictor_rebuild_start_timestep>0",
      "First timestep index included in the momentum predictor rebuild audit.");
  params.addRangeCheckedParam<unsigned int>(
      "audit_momentum_predictor_rebuild_end_timestep",
      std::numeric_limits<unsigned int>::max(),
      "audit_momentum_predictor_rebuild_end_timestep>0",
      "Last timestep index included in the momentum predictor rebuild audit.");
  params.addParamNamesToGroup(
      "volume_fraction_systems volume_fraction_equation_relaxation volume_fraction_petsc_options "
      "volume_fraction_petsc_options_iname volume_fraction_petsc_options_value "
      "volume_fraction_absolute_tolerance volume_fraction_l_tol volume_fraction_l_abs_tol "
      "volume_fraction_l_max_its should_solve_volume_fractions volume_fraction_min_value "
      "volume_fraction_max_value volume_fraction_subcycles volume_fraction_max_courant "
      "adjust_momentum_pressure_time_step momentum_pressure_max_courant "
      "volume_fraction_outer_corrections "
      "startup_pressure_initialization startup_flux_corrections "
      "num_pressure_nonorthogonal_correctors n_nonorthogonal_correctors "
      "dump_pressure_outer_debug_csv print_pressure_hbya_parity_summary "
      "dump_pressure_outer_debug_start_timestep "
      "dump_pressure_outer_debug_end_timestep dump_pressure_outer_debug_max_outer_iterations "
      "pressure_debug_face_ids pressure_debug_face_points "
      "audit_momentum_predictor_rebuild audit_momentum_predictor_rebuild_start_timestep "
      "audit_momentum_predictor_rebuild_end_timestep",
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
    _print_pressure_hbya_parity_summary(getParam<bool>("print_pressure_hbya_parity_summary")),
    _dump_pressure_outer_debug_start_timestep(
        getParam<unsigned int>("dump_pressure_outer_debug_start_timestep")),
    _dump_pressure_outer_debug_end_timestep(
        getParam<unsigned int>("dump_pressure_outer_debug_end_timestep")),
    _dump_pressure_outer_debug_max_outer_iterations(
        getParam<unsigned int>("dump_pressure_outer_debug_max_outer_iterations")),
    _pressure_debug_face_ids([&]()
                             {
                               std::unordered_set<dof_id_type> ids;
                               for (const auto id :
                                    getParam<std::vector<unsigned int>>("pressure_debug_face_ids"))
                                 ids.insert(static_cast<dof_id_type>(id));
                               return ids;
                             }()),
    _pressure_debug_face_points(getParam<std::vector<Point>>("pressure_debug_face_points")),
    _audit_momentum_predictor_rebuild(getParam<bool>("audit_momentum_predictor_rebuild")),
    _audit_momentum_predictor_rebuild_start_timestep(
        getParam<unsigned int>("audit_momentum_predictor_rebuild_start_timestep")),
    _audit_momentum_predictor_rebuild_end_timestep(
        getParam<unsigned int>("audit_momentum_predictor_rebuild_end_timestep")),
    _startup_flux_corrections(getParam<unsigned int>("startup_flux_corrections")),
    _num_pressure_nonorthogonal_correctors([&]()
                                           {
                                             const bool canonical_set = parameters().isParamSetByUser(
                                                 "num_pressure_nonorthogonal_correctors");
                                             const bool alias_set =
                                                 parameters().isParamSetByUser(
                                                     "n_nonorthogonal_correctors");
                                             const auto canonical_value =
                                                 getParam<unsigned int>(
                                                     "num_pressure_nonorthogonal_correctors");

                                             if (!alias_set)
                                               return canonical_value;

                                             const auto alias_value =
                                                 getParam<unsigned int>(
                                                     "n_nonorthogonal_correctors");
                                             if (canonical_set && alias_value != canonical_value)
                                               paramError(
                                                   "n_nonorthogonal_correctors",
                                                   "Set only one of n_nonorthogonal_correctors "
                                                   "and num_pressure_nonorthogonal_correctors, "
                                                   "or set them to the same value.");

                                             return alias_value;
                                           }())
{
  _startup_pressure_initialization =
      getParam<MooseEnum>("startup_pressure_initialization").operator std::string();

  if (_pin_pressure)
    paramError("pin_pressure",
               "ReducedPressurePIMPLE supports the OpenFOAM-style pressure boundary-condition "
               "path only; pressure pinning is not supported.");

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
        mooseError("ReducedPressurePIMPLESolve found multiple ConservativeSharpInterfaceVOFMULESCorrector "
                   "objects for system '",
                   system_name,
                   "'.");
      corrector_match = corrector;
    }

  if (!corrector_match)
  {
    static bool reported_missing_corrector = false;
    if (!reported_missing_corrector)
    {
      reported_missing_corrector = true;
      _console << name() << ": no ConservativeSharpInterfaceVOFMULESCorrector found for system '"
               << system_name << "'. Available thread-0 user objects:";
      for (const auto & obj : objs)
      {
        _console << " " << obj->name();
        if (const auto * corrector = dynamic_cast<ConservativeSharpInterfaceVOFMULESCorrector *>(obj))
          _console << "(ConservativeSharpInterfaceVOFMULESCorrector system=" << corrector->systemName()
                   << ")";
      }
      _console << std::endl;
    }
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

RhieChowMassFlux::MaxCourantAudit
ReducedPressurePIMPLESolve::momentumPressureCourantAudit(const Real dt) const
{
  if (!_rc_uo || dt <= 0.0)
    return RhieChowMassFlux::MaxCourantAudit();

  return _rc_uo->maxCourantAudit(dt);
}

std::string
ReducedPressurePIMPLESolve::momentumPressureWorstFaceSharpDiagnostics(
    const RhieChowMassFlux::MaxCourantAudit & audit) const
{
  if (!audit.has_worst_face)
    return "";

  auto * sharp_rc = sharpInterfaceRC();
  if (!sharp_rc)
    return "";

  const FaceInfo * worst_face = nullptr;
  for (const auto * fi : sharp_rc->flowFacesForAudit())
    if (fi && fi->id() == audit.worst_face_id)
    {
      worst_face = fi;
      break;
    }

  if (!worst_face)
    return "";

  ConservativeSharpInterfaceVOFMULESCorrector * vof_corrector = nullptr;
  if (_has_volume_fraction_systems && !_volume_fraction_system_names.empty())
    vof_corrector = sharpInterfaceVOFCorrector(_volume_fraction_system_names.front());

  const auto face_measure = worst_face->faceArea() * worst_face->faceCoord();
  const auto signum = [](const Real value)
  {
    return value > libMesh::TOLERANCE ? 1 : (value < -libMesh::TOLERANCE ? -1 : 0);
  };
  const auto safe_ratio = [](const Real numerator, const Real denominator)
  {
    return std::abs(denominator) > libMesh::TOLERANCE ? std::abs(numerator) / std::abs(denominator)
                                                      : 0.0;
  };

  std::ostringstream out;
  const Real raw_rc_mass_flux = sharp_rc->rawRhieChowMassFlux(*worst_face);
  const Real vof_alpha_phi_working_before =
      vof_corrector ? vof_corrector->alphaPhiWorkingBeforeIntegrated(*worst_face) : 0.0;
  const Real vof_alpha_phi_target =
      vof_corrector ? vof_corrector->alphaPhiTargetIntegrated(*worst_face) : 0.0;
  const Real vof_alpha_phi_raw_delta =
      vof_corrector ? vof_corrector->alphaPhiRawCorrectionIntegrated(*worst_face) : 0.0;
  const Real vof_alpha_phi_limited_delta =
      vof_corrector ? vof_corrector->alphaPhiLimitedDeltaIntegrated(*worst_face) : 0.0;
  const Real vof_alpha_phi_lambda =
      vof_corrector ? vof_corrector->alphaPhiAcceptedLambda(*worst_face) : 0.0;
  const Real vof_alpha_phi_after = sharp_rc->vofAlphaPhiLimitedIntegrated(*worst_face);
  const Real vof_rho_phi = sharp_rc->vofRhoPhiIntegrated(*worst_face);
  const Real predictor_convective_mass_flux =
      sharp_rc->storedPredictorConvectiveMassFlux(*worst_face);
  const Real predictor_convective_mass_flux_integrated = predictor_convective_mass_flux * face_measure;
  const Real corrected_face_phi = sharp_rc->storedCorrectedFacePhi(*worst_face);
  const Real outer_phi = sharp_rc->storedOuterIterationPhi(*worst_face);
  const int phi_sign = signum(outer_phi);
  const int alpha_phi_sign = signum(vof_alpha_phi_after);
  const int rho_phi_sign = signum(vof_rho_phi);
  const bool phi_alpha_sign_consistent =
      phi_sign == 0 || alpha_phi_sign == 0 || phi_sign == alpha_phi_sign;
  const bool phi_rho_sign_consistent =
      phi_sign == 0 || rho_phi_sign == 0 || phi_sign == rho_phi_sign;
  const Real alpha_min_local =
      std::min(sharp_rc->debugElemAlpha(*worst_face, Moose::currentState()),
               sharp_rc->debugNeighborAlpha(*worst_face, Moose::currentState()));
  const Real alpha_max_local =
      std::max(sharp_rc->debugElemAlpha(*worst_face, Moose::currentState()),
               sharp_rc->debugNeighborAlpha(*worst_face, Moose::currentState()));
  out << ", sn_grad_rho="
      << sharp_rc->debugFaceNormalDensityGradient(*worst_face, Moose::currentState())
      << ", sn_grad_rho_orthogonal_part="
      << sharp_rc->debugFaceNormalDensityGradientOrthogonalPart(*worst_face, Moose::currentState())
      << ", sn_grad_rho_base_part="
      << sharp_rc->debugFaceNormalDensityGradientBasePart(*worst_face, Moose::currentState())
      << ", sn_grad_rho_correction_part="
      << sharp_rc->debugFaceNormalDensityGradientCorrectionPart(*worst_face, Moose::currentState())
      << ", sn_grad_rho_limited_correction_part="
      << sharp_rc->debugFaceNormalDensityGradientLimitedCorrectionPart(*worst_face,
                                                                       Moose::currentState())
      << ", elem_alpha=" << sharp_rc->debugElemAlpha(*worst_face, Moose::currentState())
      << ", neighbor_alpha=" << sharp_rc->debugNeighborAlpha(*worst_face, Moose::currentState())
      << ", elem_rho=" << sharp_rc->debugElemDensity(*worst_face, Moose::currentState())
      << ", neighbor_rho=" << sharp_rc->debugNeighborDensity(*worst_face, Moose::currentState())
      << ", vof_alpha_phi_integrated="
      << vof_alpha_phi_after
      << ", vof_alpha_phi_working_before=" << vof_alpha_phi_working_before
      << ", vof_alpha_phi_target=" << vof_alpha_phi_target
      << ", vof_alpha_phi_raw_delta=" << vof_alpha_phi_raw_delta
      << ", vof_alpha_phi_limited_delta=" << vof_alpha_phi_limited_delta
      << ", vof_alpha_phi_lambda=" << vof_alpha_phi_lambda
      << ", vof_rho_phi_integrated="
      << vof_rho_phi
      << ", vof_rho_phi_base_integrated="
      << sharp_rc->vofBaseGasRhoPhiIntegrated(*worst_face)
      << ", vof_rho_phi_alpha_correction_integrated="
      << sharp_rc->vofAlphaCorrectionRhoPhiIntegrated(*worst_face)
      << ", alpha_local_min=" << alpha_min_local
      << ", alpha_local_max=" << alpha_max_local
      << ", phi_alpha_sign_consistent=" << phi_alpha_sign_consistent
      << ", phi_rho_sign_consistent=" << phi_rho_sign_consistent
      << ", alpha_target_over_working_ratio="
      << safe_ratio(vof_alpha_phi_target, vof_alpha_phi_working_before)
      << ", alpha_limited_over_raw_ratio="
      << safe_ratio(vof_alpha_phi_limited_delta, vof_alpha_phi_raw_delta)
      << ", rho_over_alpha_ratio=" << safe_ratio(vof_rho_phi, vof_alpha_phi_after)
      << ", predictor_mass_over_rho_ratio="
      << safe_ratio(predictor_convective_mass_flux_integrated, vof_rho_phi)
      << ", face_rho_g=" << sharp_rc->debugGasDensityFace(*worst_face, Moose::currentState())
      << ", face_rho_l=" << sharp_rc->debugLiquidDensityFace(*worst_face, Moose::currentState())
      << ", hydro_raw=" << sharp_rc->debugHydrostaticFaceMassFluxDensityRaw(*worst_face)
      << ", raw_rc_mass_flux=" << raw_rc_mass_flux
      << ", outer_phi=" << outer_phi
      << ", outer_rho_phi=" << sharp_rc->storedOuterIterationRhoPhiIntegrated(*worst_face)
      << ", predictor_convective_phi=" << sharp_rc->storedPredictorConvectivePhi(*worst_face)
      << ", predictor_convective_mass_flux="
      << predictor_convective_mass_flux
      << ", predictor_convective_mass_flux_integrated=" << predictor_convective_mass_flux_integrated
      << ", corrected_face_phi=" << corrected_face_phi
      << ", pressure_predictor_base_phi_internal="
      << sharp_rc->storedPressurePredictorBasePhi(*worst_face)
      << ", pressure_predictor_base_phi_reference="
      << -sharp_rc->storedPressurePredictorBasePhi(*worst_face)
      << ", transient_phi=" << sharp_rc->storedTransientProjectionFlux(*worst_face)
      << ", cap_hydro_phi=" << sharp_rc->storedCapillaryHydrostaticFlux(*worst_face)
      << ", phig_phi=" << sharp_rc->storedPhigFlux(*worst_face)
      << ", predictor_phi=" << sharp_rc->storedPredictorOperatorPhi(*worst_face)
      << ", pressure_eq_phi=" << sharp_rc->storedPressureEquationVolumetricFlux(*worst_face)
      << ", pressure_corr_phi=" << sharp_rc->storedPressureCorrectionPhi(*worst_face)
      << ", face_gC=" << worst_face->gC();

  out << ", sharp_pred_cache_used=" << sharp_rc->debugUsingCachedPredictorOperator();

  appendSharpCellDebug(out,
                       ", ",
                       "elem",
                       worst_face->elemInfo(),
                       _momentum_system_numbers,
                       *sharp_rc,
                       worst_face_cell_debug_keys);
  appendSharpCellDebug(out,
                       ", ",
                       "neighbor",
                       worst_face->neighborInfo(),
                       _momentum_system_numbers,
                       *sharp_rc,
                       worst_face_cell_debug_keys);

  const Elem * const worst_elem = worst_face->elemPtr();
  const Elem * const worst_neighbor = worst_face->neighborPtr();
  bool started_stencil = false;
  for (const auto * fi : sharp_rc->flowFacesForAudit())
  {
    if (!fi)
      continue;

    const bool touches_elem = worst_elem && (fi->elemPtr() == worst_elem || fi->neighborPtr() == worst_elem);
    const bool touches_neighbor =
        worst_neighbor && (fi->elemPtr() == worst_neighbor || fi->neighborPtr() == worst_neighbor);
    if (!touches_elem && !touches_neighbor)
      continue;

    if (!started_stencil)
    {
      out << ", local_stencil_faces=[";
      started_stencil = true;
    }
    else
      out << ';';

    out << fi->id() << "{c=" << fi->faceCentroid() << ",n=" << fi->normal()
        << ",phi=" << sharp_rc->storedCorrectedFacePhi(*fi)
        << ",outer_phi=" << sharp_rc->storedOuterIterationPhi(*fi)
        << ",outer_rho_phi=" << sharp_rc->storedOuterIterationRhoPhiIntegrated(*fi)
        << ",vof_alpha_phi=" << sharp_rc->vofAlphaPhiLimitedIntegrated(*fi)
        << ",vof_alpha_before="
        << (vof_corrector ? vof_corrector->alphaPhiWorkingBeforeIntegrated(*fi) : 0.0)
        << ",vof_alpha_target="
        << (vof_corrector ? vof_corrector->alphaPhiTargetIntegrated(*fi) : 0.0)
        << ",vof_alpha_raw="
        << (vof_corrector ? vof_corrector->alphaPhiRawCorrectionIntegrated(*fi) : 0.0)
        << ",vof_alpha_limited_delta="
        << (vof_corrector ? vof_corrector->alphaPhiLimitedDeltaIntegrated(*fi) : 0.0)
        << ",vof_lambda="
        << (vof_corrector ? vof_corrector->alphaPhiAcceptedLambda(*fi) : 0.0)
        << ",alpha_target_over_working="
        << safe_ratio(vof_corrector ? vof_corrector->alphaPhiTargetIntegrated(*fi) : 0.0,
                      vof_corrector ? vof_corrector->alphaPhiWorkingBeforeIntegrated(*fi) : 0.0)
        << ",alpha_limited_over_raw="
        << safe_ratio(vof_corrector ? vof_corrector->alphaPhiLimitedDeltaIntegrated(*fi) : 0.0,
                      vof_corrector ? vof_corrector->alphaPhiRawCorrectionIntegrated(*fi) : 0.0)
        << ",vof_rho_phi=" << sharp_rc->vofRhoPhiIntegrated(*fi)
        << ",vof_base=" << sharp_rc->vofBaseGasRhoPhiIntegrated(*fi)
        << ",vof_alpha_corr=" << sharp_rc->vofAlphaCorrectionRhoPhiIntegrated(*fi)
        << ",rho_over_alpha="
        << safe_ratio(sharp_rc->vofRhoPhiIntegrated(*fi), sharp_rc->vofAlphaPhiLimitedIntegrated(*fi))
        << ",pred_mass=" << sharp_rc->storedPredictorConvectiveMassFlux(*fi)
        << ",pred_mass_over_rho="
        << safe_ratio(sharp_rc->storedPredictorConvectiveMassFlux(*fi) * fi->faceArea() * fi->faceCoord(),
                      sharp_rc->vofRhoPhiIntegrated(*fi))
        << ",raw_rc=" << sharp_rc->rawRhieChowMassFlux(*fi) << '}';
  }
  if (started_stencil)
    out << ']';

  return out.str();
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

  const bool light_trace_timestep =
      _problem.timeStep() <= 5 || (_problem.timeStep() % 25 == 0);

  if (light_trace_timestep)
    std::cerr << "[ReducedPressurePIMPLESolve] solve begin"
              << " ts=" << _problem.timeStep() << " time=" << _problem.time()
              << " dt=" << _problem.dt() << std::endl;

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

  if (_problem.timeStep() == 1)
    std::cerr << "[ReducedPressurePIMPLESolve] volume-fraction flags"
              << " has_vf=" << _has_volume_fraction_systems
              << " should_solve_vf=" << _should_solve_volume_fractions
              << " vf_names=" << _volume_fraction_system_names.size()
              << " vf_systems=" << _volume_fraction_systems.size() << std::endl;

  if (_cht.enabled() && _should_solve_energy)
    _cht.initializeCHTCouplingFields();

  if (auto * sharp_rc = sharpInterfaceRC())
  {
    sharp_rc->clearVOFTransportState();
  }

  if (startupPressureInitializationEnabled() && _problem.timeStep() == 1)
  {
    for (auto * system : _momentum_systems)
    {
      std::cerr << "[ReducedPressurePIMPLESolve] startup sync begin system=" << system->name()
                << std::endl;
      synchronizeSystemState(*system);
      std::cerr << "[ReducedPressurePIMPLESolve] startup sync end system=" << system->name()
                << std::endl;
    }
    std::cerr << "[ReducedPressurePIMPLESolve] startup sync begin system="
              << _pressure_system.name() << std::endl;
    synchronizeSystemState(_pressure_system);
    std::cerr << "[ReducedPressurePIMPLESolve] startup sync end system=" << _pressure_system.name()
              << std::endl;
    for (auto * system : _volume_fraction_systems)
    {
      std::cerr << "[ReducedPressurePIMPLESolve] startup sync begin system=" << system->name()
                << std::endl;
      synchronizeSystemState(*system);
      std::cerr << "[ReducedPressurePIMPLESolve] startup sync end system=" << system->name()
                << std::endl;
    }

    std::cerr << "[ReducedPressurePIMPLESolve] startup execute begin" << std::endl;
    _problem.execute(EXEC_NONLINEAR);
    std::cerr << "[ReducedPressurePIMPLESolve] startup execute end" << std::endl;
  }

  if (_should_solve_pressure)
  {
    if (_problem.timeStep() == 1)
      std::cerr << "[ReducedPressurePIMPLESolve] startup pressure init begin" << std::endl;
    initializeStartupPressureField(solver_params);
    if (_problem.timeStep() == 1)
      std::cerr << "[ReducedPressurePIMPLESolve] startup pressure init end" << std::endl;
  }

  while (simple_iteration_counter < _num_iterations)
  {
    simple_iteration_counter++;
    _current_outer_iteration = simple_iteration_counter;

    if (_problem.timeStep() == 1)
      std::cerr << "[ReducedPressurePIMPLESolve] outer iteration begin"
                << " outer=" << _current_outer_iteration << std::endl;

    if (_should_solve_momentum)
      // Keep the full nonlinear history on the previous outer-corrector state
      // for the whole current outer loop. The stock momentum solve shifts this
      // stack every predictor solve; for parity work we only want to advance it
      // once per outer SIMPLE iteration.
      advanceMomentumOuterIterationHistory();

    // Mirror interFoam's outer-loop choreography by doing the alpha subcycling
    // and mixture/rhoPhi refresh inside every outer correction, just before
    // the momentum-pressure coupling work. This keeps rhoPhi consistent with
    // the outer-corrector state instead of freezing one alpha update for a
    // later sequence of momentum-pressure repredictions.
    if (_has_volume_fraction_systems && _should_solve_volume_fractions)
    {
      if (_problem.timeStep() == 1)
        std::cerr << "[ReducedPressurePIMPLESolve] entering volume-fraction outer block"
                  << " outer=" << _current_outer_iteration << std::endl;

      // Keep the true timestep-old alpha in solutionOld(), but advance the
      // nonlinear-state stack once per outer iteration so we have a separate
      // previous-outer iterate available, analogous to interFoam's prevIter().
      advanceVolumeFractionOuterIterationHistory();

      if (auto * sharp_rc = sharpInterfaceRC())
      {
        sharp_rc->clearVOFTransportState();
        const bool use_previous_timestep_transport_flux =
            _problem.timeStep() == 1 && _current_outer_iteration == 1;
        sharp_rc->freezeVOFTransportState(use_previous_timestep_transport_flux);
      }

      _problem.execute(EXEC_NONLINEAR);
      if (_problem.timeStep() == 1)
        std::cerr << "[ReducedPressurePIMPLESolve] before solveVolumeFractionSystems"
                  << " outer=" << _current_outer_iteration << std::endl;
      Moose::PetscSupport::petscSetOptions(_volume_fraction_petsc_options, solver_params);
      const auto vf_residuals = solveVolumeFractionSystems(solver_params);
      if (_problem.timeStep() == 1)
        std::cerr << "[ReducedPressurePIMPLESolve] after solveVolumeFractionSystems"
                  << " outer=" << _current_outer_iteration
                  << " residual_count=" << vf_residuals.size() << std::endl;

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
    {
      if (_problem.timeStep() == 1)
        std::cerr << "[ReducedPressurePIMPLESolve] calling correctVelocity" << std::endl;
      ns_residuals[pressure_index] = correctVelocity(true, true, solver_params);
      if (light_trace_timestep)
        std::cerr << "[ReducedPressurePIMPLESolve] correctVelocity returned"
                  << " residual=" << ns_residuals[pressure_index].second << std::endl;
    }

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

    if (_problem.timeStep() == 1)
      std::cerr << "[ReducedPressurePIMPLESolve] executing EXEC_NONLINEAR at outer-loop tail"
                << std::endl;
    _problem.execute(EXEC_NONLINEAR);
    if (_problem.timeStep() == 1)
      std::cerr << "[ReducedPressurePIMPLESolve] EXEC_NONLINEAR complete at outer-loop tail"
                << std::endl;

    converged = NS::FV::converged(ns_residuals, ns_abs_tols);
    if (light_trace_timestep)
      std::cerr << "[ReducedPressurePIMPLESolve] outer iteration convergence check"
                << " converged=" << converged << std::endl;
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

  if (light_trace_timestep)
    std::cerr << "[ReducedPressurePIMPLESolve] solve end converged=" << converged << std::endl;

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

bool
ReducedPressurePIMPLESolve::auditMomentumPredictorRebuild() const
{
  return _audit_momentum_predictor_rebuild &&
         _problem.timeStep() >= _audit_momentum_predictor_rebuild_start_timestep &&
         _problem.timeStep() <= _audit_momentum_predictor_rebuild_end_timestep;
}

void
ReducedPressurePIMPLESolve::addMomentumPredictorExplicitForcing(const unsigned int system_i,
                                                                NumericVector<Number> & rhs)
{
  if (auto * sharp_rc = sharpInterfaceRC();
      sharp_rc && sharp_rc->splitMomentumPredictorOperator())
    sharp_rc->addMomentumPredictorExplicitForcing(system_i, rhs);
}

void
ReducedPressurePIMPLESolve::addMomentumPredictorBodyForceForcing(const unsigned int system_i,
                                                                 NumericVector<Number> & rhs)
{
  if (auto * sharp_rc = sharpInterfaceRC();
      sharp_rc && sharp_rc->splitMomentumPredictorOperator())
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

ConservativeSharpInterfaceCurvatureCalculator *
ReducedPressurePIMPLESolve::sharpInterfaceCurvature() const
{
  std::vector<UserObject *> objs;
  _problem.theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribThread>(0)
      .queryInto(objs);
  ConservativeSharpInterfaceCurvatureCalculator * curvature_match = nullptr;
  for (const auto & obj : objs)
    if (auto * curvature = dynamic_cast<ConservativeSharpInterfaceCurvatureCalculator *>(obj))
    {
      if (curvature_match)
        mooseError("ReducedPressurePIMPLESolve found multiple ConservativeSharpInterfaceCurvatureCalculator "
                   "objects in the problem. The current implementation requires a single "
                   "sharp-interface curvature producer.");
      curvature_match = curvature;
    }

  return curvature_match;
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

  std::cerr << "[ReducedPressurePIMPLESolve] startup assemble predictor begin" << std::endl;

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
    std::cerr << "[ReducedPressurePIMPLESolve] startup assemble predictor system_i=" << system_i
              << " begin" << std::endl;
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
    closeVectorWithTrace("startup predictor current_local before assembly",
                         momentum_system.current_local_solution.get());
    closeVectorWithTrace("startup predictor solution before assembly", &solution);
    closeVectorWithTrace("startup predictor old before assembly",
                         &_momentum_systems[system_i]->solutionOld());
    closeVectorWithTrace("startup predictor previous newton before assembly",
                         _momentum_systems[system_i]->solutionPreviousNewton());
    closeVectorWithTrace("startup predictor rhs before assembly", &rhs);
    _problem.computeLinearSystemSys(momentum_system, mmat, rhs, /*compute_grads*/ true);
    closeVectorWithTrace("startup predictor rhs after assembly", &rhs);
    applyMomentumEquationRelaxation(mmat, rhs, solution, *diff_diagonal);

    if (_rc_uo && _rc_uo->splitMomentumPredictorOperator())
    {
      predictor_diagonal_raw = solution.zero_clone();
      predictor_diagonal_raw->close();
      auto * petsc_mat = dynamic_cast<PetscMatrix<Number> *>(momentum_system.matrix);
      mooseAssert(petsc_mat,
                  "ReducedPressurePIMPLESolve startup predictor caching requires PETSc matrices.");
      std::cerr << "[ReducedPressurePIMPLESolve] startup predictor raw diagonal begin" << std::endl;
      petsc_mat->get_diagonal(*predictor_diagonal_raw);
      predictor_diagonal_raw->close();
      std::cerr << "[ReducedPressurePIMPLESolve] startup predictor raw diagonal end" << std::endl;
      _rc_uo->cacheStartupPredictorDiagonal(system_i, *predictor_diagonal_raw);
    }

    if (_rc_uo && _rc_uo->splitMomentumPredictorOperator())
    {
      std::cerr << "[ReducedPressurePIMPLESolve] startup predictor rhs base clone begin"
                << std::endl;
      predictor_rhs_base = rhs.clone();
      predictor_rhs_base->close();
      std::cerr << "[ReducedPressurePIMPLESolve] startup predictor rhs base clone end"
                << std::endl;

      std::cerr << "[ReducedPressurePIMPLESolve] startup predictor body force begin" << std::endl;
      predictor_body_force = rhs.clone();
      predictor_body_force->zero();
      predictor_body_force->close();
      addMomentumPredictorBodyForceForcing(system_i, *predictor_body_force);
      predictor_body_force->close();
      std::cerr << "[ReducedPressurePIMPLESolve] startup predictor body force end" << std::endl;

      std::cerr << "[ReducedPressurePIMPLESolve] startup predictor explicit forcing begin"
                << std::endl;
      addMomentumPredictorExplicitForcing(system_i, rhs);

      predictor_explicit_force = rhs.clone();
      predictor_explicit_force->add(-1.0, *predictor_rhs_base);
      predictor_explicit_force->close();
      std::cerr << "[ReducedPressurePIMPLESolve] startup predictor explicit forcing end"
                << std::endl;
    }

    momentum_system.update();

    if (_rc_uo)
      _rc_uo->cacheMomentumPredictorOperator(system_i,
                                             predictor_rhs_base.get(),
                                             predictor_explicit_force.get(),
                                             predictor_body_force.get());
    std::cerr << "[ReducedPressurePIMPLESolve] startup assemble predictor system_i=" << system_i
              << " end" << std::endl;
  }

  momentum_solver.reuse_preconditioner(false);
  std::cerr << "[ReducedPressurePIMPLESolve] startup assemble predictor end" << std::endl;
}

void
ReducedPressurePIMPLESolve::initializeStartupPressureField(const SolverParams & solver_params)
{
  if (!startupPressureInitializationEnabled() || _problem.timeStep() != 1)
    return;

  if (!_should_solve_pressure)
    return;

  _console << "Applying startup continuity / CorrectPhi projection before PIMPLE iterations"
           << std::endl;

  // Closest MOOSE equivalent of interFoam's initCorrectPhi: honor the current
  // reduced-pressure field, assemble the momentum predictor coefficients, and
  // run pressure-only startup continuity corrections before the first outer
  // iteration.
  if (!_momentum_systems.empty() && _rc_uo)
  {
    assembleMomentumPredictorOnly();
    std::cerr << "[ReducedPressurePIMPLESolve] startup init face mass flux begin" << std::endl;
    _rc_uo->initFaceMassFlux();
    std::cerr << "[ReducedPressurePIMPLESolve] startup init face mass flux end" << std::endl;
    std::cerr << "[ReducedPressurePIMPLESolve] startup continuity corrections begin" << std::endl;
    performStartupContinuityCorrections(solver_params);
    std::cerr << "[ReducedPressurePIMPLESolve] startup continuity corrections end" << std::endl;
    std::cerr << "[ReducedPressurePIMPLESolve] startup final pressure sync begin" << std::endl;
    synchronizeSystemState(_pressure_system);
    std::cerr << "[ReducedPressurePIMPLESolve] startup final pressure sync end" << std::endl;
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
  std::cerr << "[ReducedPressurePIMPLESolve] startup pressure init execute begin" << std::endl;
  _problem.execute(EXEC_NONLINEAR);
  std::cerr << "[ReducedPressurePIMPLESolve] startup pressure init execute end" << std::endl;
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
    std::cerr << "[ReducedPressurePIMPLESolve] startup continuity correction it=" << startup_it
              << " begin" << std::endl;
    (void)startup_it;
    (void)correctStartupContinuityOnce(true, true, solver_params);
    std::cerr << "[ReducedPressurePIMPLESolve] startup continuity correction it=" << startup_it
              << " end" << std::endl;
  }

  std::cerr << "[ReducedPressurePIMPLESolve] startup continuity execute begin" << std::endl;
  _problem.execute(EXEC_NONLINEAR);
  std::cerr << "[ReducedPressurePIMPLESolve] startup continuity execute end" << std::endl;
}

void
ReducedPressurePIMPLESolve::preparePressureCorrectorState(const bool subtract_updated_pressure)
{
  if (_problem.timeStep() == 1)
    std::cerr << "[ReducedPressurePIMPLESolve] preparePressureCorrectorState begin"
              << " outer=" << _current_outer_iteration << " piso=" << _current_piso_iteration
              << " subtract_updated_pressure=" << subtract_updated_pressure << std::endl;

  // Refresh the curvature producer before the pressure predictor stage so every
  // downstream face functor sees the latest smoothed normals / curvature.
  if (auto * curvature = sharpInterfaceCurvature())
  {
    if (_problem.timeStep() == 1)
      std::cerr << "[ReducedPressurePIMPLESolve] updating curvature maps" << std::endl;
    curvature->updateCurvatureMaps(_print_fields);
  }

  if (_problem.timeStep() == 1)
    std::cerr << "[ReducedPressurePIMPLESolve] computing HbyA" << std::endl;
  _rc_uo->computeHbyA(subtract_updated_pressure, _print_fields);

  if (auto * sharp_rc = sharpInterfaceRC())
  {
    if (_problem.timeStep() == 1)
      std::cerr << "[ReducedPressurePIMPLESolve] updating additional pressure flux functors"
                << std::endl;
    sharp_rc->updateAdditionalPressureFluxFunctors(subtract_updated_pressure, _print_fields);
  }

  // Mirror reference solver's correctUphiBCs -> constrainPressure ordering more closely:
  // refresh the patch velocity / target-flux state from the latest momentum
  // predictor before assembling the constrained pressure boundary gradient.
  _rc_uo->updateVelocityBoundaryState();

  if (_problem.timeStep() == 1)
    std::cerr << "[ReducedPressurePIMPLESolve] updating pressure boundary gradients" << std::endl;
  _rc_uo->updatePressureBoundaryNormalGradients(/* apply_reference_adjustment = */ false);

  if (_problem.timeStep() == 1)
    std::cerr << "[ReducedPressurePIMPLESolve] preparePressureCorrectorState end" << std::endl;
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
ReducedPressurePIMPLESolve::dumpPressureOuterDebugState(const std::string & stage_label)
{
  if (_problem.timeStep() < _dump_pressure_outer_debug_start_timestep ||
      _problem.timeStep() > _dump_pressure_outer_debug_end_timestep ||
      _current_outer_iteration > _dump_pressure_outer_debug_max_outer_iterations)
    return;

  if (!_pressure_debug_face_ids.empty() || !_pressure_debug_face_points.empty())
    dumpPressureDebugFaces(stage_label);

  if (!_dump_pressure_outer_debug_csv)
    return;

  if (_problem.timeStep() == 1)
    std::cerr << "[ReducedPressurePIMPLESolve] dumpPressureOuterDebugState begin"
              << " stage=" << stage_label << " outer=" << _current_outer_iteration
              << " piso=" << _current_piso_iteration << std::endl;

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
           "corr_reference_delta_u,corr_reference_delta_v,corr_reference_delta_w,"
           "corr_smooth_delta_u,corr_smooth_delta_v,corr_smooth_delta_w,"
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
                   : ConservativeSharpInterfaceRhieChowMassFlux::PressureCorrectionReconstructionDebug{};
      const auto predictor_force_debug =
          sharp_rc ? sharp_rc->momentumPredictorExplicitForceDebug(*elem_info, Moose::currentState())
                   : ConservativeSharpInterfaceRhieChowMassFlux::MomentumPredictorExplicitForceDebug{};

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
          hbya_value = sharp_rc->debugCellHbyARaw(dim_i, momentum_dof);
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
          ainv_value = sharp_rc->debugCellAinvRaw(dim_i, momentum_dof);
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
          const Real ainv_value = sharp_rc->debugCellAinvRaw(dim_i, momentum_dof);
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
      for (const auto value : corr_debug.reference_delta_velocity)
        out << ',' << value;
      for (const auto value : corr_debug.smooth_delta_velocity)
        out << ',' << value;
      for (const auto value : corr_debug.delta_velocity)
        out << ',' << value;

      out << '\n';
    }
  }

  if (auto * sharp_rc = sharpInterfaceRC())
    sharp_rc->dumpPressureCorrectorFaceDebugCSV(file_base + iter_label + "_pressure_faces.csv");

  if (_problem.timeStep() == 1)
    std::cerr << "[ReducedPressurePIMPLESolve] dumpPressureOuterDebugState end"
              << " stage=" << stage_label << std::endl;
}

void
ReducedPressurePIMPLESolve::printPressureHbyAParitySummary(const std::string & stage_label) const
{
  if (!_print_pressure_hbya_parity_summary ||
      _problem.timeStep() < _dump_pressure_outer_debug_start_timestep ||
      _problem.timeStep() > _dump_pressure_outer_debug_end_timestep ||
      _current_outer_iteration > _dump_pressure_outer_debug_max_outer_iterations)
    return;

  const auto * const sharp_rc = sharpInterfaceRC();
  if (!sharp_rc)
    return;

  std::size_t face_count = 0;
  Real max_abs_outer_rho_phi_minus_vof = 0.0;
  Real max_abs_phi_hbya_style_mismatch = 0.0;
  Real max_abs_final_phi_mismatch = 0.0;
  Real max_abs_writeback_phi_mismatch = 0.0;
  Real max_abs_phig = 0.0;
  const FaceInfo * worst_rho_phi_face = nullptr;

  Point corner_min_max;
  corner_min_max(0) = std::numeric_limits<Real>::max();
  corner_min_max(1) = std::numeric_limits<Real>::lowest();
  corner_min_max(2) = 0.0;
  const auto dim = _momentum_system_numbers.size();

  for (const auto * fi : sharp_rc->flowFacesForAudit())
    if (fi)
    {
      const Point centroid = fi->faceCentroid();
      corner_min_max(0) = std::min(corner_min_max(0), centroid(0));
      corner_min_max(1) = std::max(corner_min_max(1), centroid(1));
      if (dim > 2)
        corner_min_max(2) = std::max(corner_min_max(2), centroid(2));
    }

  const ElemInfo * top_left_elem = nullptr;
  Real top_left_dist_sq = std::numeric_limits<Real>::max();
  std::unordered_set<dof_id_type> visited_elem_ids;

  auto consider_elem = [&](const ElemInfo * elem_info)
  {
    if (!elem_info || !elem_info->elem())
      return;

    if (!visited_elem_ids.insert(elem_info->elem()->id()).second)
      return;

    const Real dist_sq = (elem_info->centroid() - corner_min_max).norm_sq();
    if (dist_sq < top_left_dist_sq)
    {
      top_left_dist_sq = dist_sq;
      top_left_elem = elem_info;
    }
  };

  for (const auto * fi : sharp_rc->flowFacesForAudit())
  {
    if (!fi)
      continue;

    ++face_count;

    const Real vof_rho_phi = sharp_rc->vofRhoPhiIntegrated(*fi);
    const Real outer_rho_phi = sharp_rc->storedOuterIterationRhoPhiIntegrated(*fi);
    const Real outer_rho_phi_mismatch = std::abs(outer_rho_phi - vof_rho_phi);
    if (outer_rho_phi_mismatch > max_abs_outer_rho_phi_minus_vof)
    {
      max_abs_outer_rho_phi_minus_vof = outer_rho_phi_mismatch;
      worst_rho_phi_face = fi;
    }

    const Real reference_phi_hbya_before_phig =
        sharp_rc->storedPredictorOperatorPhi(*fi) + sharp_rc->storedTransientProjectionFlux(*fi);
    const Real reference_phi_hbya =
        reference_phi_hbya_before_phig + sharp_rc->storedPhigFlux(*fi);
    const Real current_phi_hbya = sharp_rc->storedPhiHbyAReferenceStyle(*fi);
    max_abs_phi_hbya_style_mismatch =
        std::max(max_abs_phi_hbya_style_mismatch,
                 std::abs(current_phi_hbya - reference_phi_hbya));

    const Real reference_final_phi =
        reference_phi_hbya + sharp_rc->storedPressureEquationVolumetricFlux(*fi);
    max_abs_final_phi_mismatch =
        std::max(max_abs_final_phi_mismatch,
                 std::abs(sharp_rc->storedCorrectedFacePhi(*fi) - reference_final_phi));

    const Real reference_writeback_phi =
        sharp_rc->storedPressureEquationVolumetricFlux(*fi) + sharp_rc->storedPhigFlux(*fi);
    max_abs_writeback_phi_mismatch =
        std::max(max_abs_writeback_phi_mismatch,
                 std::abs(sharp_rc->storedPressureCorrectionPhi(*fi) - reference_writeback_phi));
    max_abs_phig = std::max(max_abs_phig, std::abs(sharp_rc->storedPhigFlux(*fi)));

    consider_elem(fi->elemInfo());
    consider_elem(fi->neighborInfo());
  }

  Real top_left_velocity_magnitude = 0.0;
  dof_id_type top_left_elem_id = DofObject::invalid_id;
  if (top_left_elem)
  {
    top_left_elem_id = top_left_elem->elem()->id();
    for (const auto dim_i : make_range(dim))
    {
      const Real component = sharp_rc->debugCurrentVelocityComponent(*top_left_elem, dim_i);
      top_left_velocity_magnitude += component * component;
    }
    top_left_velocity_magnitude = std::sqrt(top_left_velocity_magnitude);
  }

  dof_id_type worst_rho_phi_face_id = DofObject::invalid_id;
  Point worst_rho_phi_centroid;
  Real worst_outer_phi = 0.0;
  Real worst_vof_transport_phi = 0.0;
  Real worst_outer_rho_phi = 0.0;
  Real worst_vof_rho_phi = 0.0;
  Real worst_vof_base_gas_rho_phi = 0.0;
  Real worst_vof_alpha_correction_rho_phi = 0.0;
  Real worst_alpha_phi = 0.0;
  Real worst_elem_alpha = 0.0;
  Real worst_neighbor_alpha = 0.0;
  if (worst_rho_phi_face)
  {
    worst_rho_phi_face_id = worst_rho_phi_face->id();
    worst_rho_phi_centroid = worst_rho_phi_face->faceCentroid();
    worst_outer_phi = sharp_rc->storedOuterIterationPhi(*worst_rho_phi_face);
    worst_vof_transport_phi = sharp_rc->storedVOFTransportPhi(*worst_rho_phi_face);
    worst_outer_rho_phi = sharp_rc->storedOuterIterationRhoPhiIntegrated(*worst_rho_phi_face);
    worst_vof_rho_phi = sharp_rc->vofRhoPhiIntegrated(*worst_rho_phi_face);
    worst_vof_base_gas_rho_phi = sharp_rc->vofBaseGasRhoPhiIntegrated(*worst_rho_phi_face);
    worst_vof_alpha_correction_rho_phi =
        sharp_rc->vofAlphaCorrectionRhoPhiIntegrated(*worst_rho_phi_face);
    worst_alpha_phi = sharp_rc->vofAlphaPhiLimitedIntegrated(*worst_rho_phi_face);
    worst_elem_alpha = sharp_rc->debugElemAlpha(*worst_rho_phi_face, Moose::currentState());
    worst_neighbor_alpha =
        sharp_rc->debugNeighborAlpha(*worst_rho_phi_face, Moose::currentState());
  }

  _console << std::setprecision(8) << name() << ": hbya_parity"
           << " stage=" << stage_label
           << " ts=" << _problem.timeStep()
           << " outer=" << _current_outer_iteration
           << " piso=" << _current_piso_iteration
           << " faces=" << face_count
           << " max|outerRhoPhi-vofRhoPhi|=" << max_abs_outer_rho_phi_minus_vof
           << " max|phiHbyA-currentRef|=" << max_abs_phi_hbya_style_mismatch
           << " max|finalPhi-ref|=" << max_abs_final_phi_mismatch
           << " max|writebackPhi-ref|=" << max_abs_writeback_phi_mismatch
           << " max|phig|=" << max_abs_phig
           << " worstRhoPhiFace=" << worst_rho_phi_face_id
           << " worstRhoPhiCentroid=" << worst_rho_phi_centroid
           << " worstOuterPhi=" << worst_outer_phi
           << " worstVOFTransportPhi=" << worst_vof_transport_phi
           << " worstOuterRhoPhi=" << worst_outer_rho_phi
           << " worstVOFRhoPhi=" << worst_vof_rho_phi
           << " worstVOFBaseGasRhoPhi=" << worst_vof_base_gas_rho_phi
           << " worstVOFAlphaCorrectionRhoPhi=" << worst_vof_alpha_correction_rho_phi
           << " worstAlphaPhi=" << worst_alpha_phi
           << " worstElemAlpha=" << worst_elem_alpha
           << " worstNeighborAlpha=" << worst_neighbor_alpha
           << " topLeftElem=" << top_left_elem_id
           << " topLeft|U|=" << top_left_velocity_magnitude
           << std::endl;
}

void
ReducedPressurePIMPLESolve::resolvePressureDebugFaceIds()
{
  if (_pressure_debug_face_ids_resolved)
    return;

  _pressure_debug_face_ids_resolved = true;

  auto * sharp_rc = sharpInterfaceRC();
  if (!sharp_rc || _pressure_debug_face_points.empty())
    return;

  for (const auto & probe_point : _pressure_debug_face_points)
  {
    const FaceInfo * nearest_face = nullptr;
    Real min_dist_sq = std::numeric_limits<Real>::max();

    for (const auto * fi : sharp_rc->flowFacesForAudit())
    {
      if (!fi)
        continue;

      const Real dist_sq = (fi->faceCentroid() - probe_point).norm_sq();
      if (dist_sq < min_dist_sq)
      {
        min_dist_sq = dist_sq;
        nearest_face = fi;
      }
    }

    if (!nearest_face)
      continue;

    _pressure_debug_face_ids.insert(nearest_face->id());
    _console << name() << ": pressure_debug_face_point " << probe_point
             << " resolved_to_face_id=" << nearest_face->id()
             << " centroid=" << nearest_face->faceCentroid()
             << " distance=" << std::sqrt(min_dist_sq) << std::endl;
  }
}

void
ReducedPressurePIMPLESolve::dumpPressureDebugFaces(const std::string & stage_label)
{
  resolvePressureDebugFaceIds();

  auto * sharp_rc = sharpInterfaceRC();
  if (!sharp_rc || _pressure_debug_face_ids.empty())
    return;

  std::ostringstream msg;
  msg << name() << ": pressure_debug_faces"
      << " stage=" << stage_label
      << " ts=" << _problem.timeStep()
      << " outer=" << _current_outer_iteration
      << " piso=" << _current_piso_iteration;

  bool appended = false;
  for (const auto * fi : sharp_rc->flowFacesForAudit())
  {
    if (!fi || !_pressure_debug_face_ids.count(fi->id()))
      continue;

    if (!appended)
    {
      msg << " faces=[";
      appended = true;
    }
    else
      msg << ';';

    const bool internal_face = fi->neighborInfo() != nullptr;
    const Real p_elem_matrix =
        internal_face ? _rc_uo->debugPressureElemMatrixContribution(*fi) : 0.0;
    const Real p_neighbor_matrix =
        internal_face ? _rc_uo->debugPressureNeighborMatrixContribution(*fi) : 0.0;
    const Real p_elem_rhs = internal_face ? _rc_uo->debugPressureElemRHSContribution(*fi) : 0.0;
    const Real reference_hbya_phi = sharp_rc->storedPredictorOperatorPhi(*fi);
    const Real reference_ddt_corr_phi = sharp_rc->storedTransientProjectionFlux(*fi);
    const Real reference_phig_phi = sharp_rc->storedPhigFlux(*fi);
    const Real reference_phi_hbya_before_phig = reference_hbya_phi + reference_ddt_corr_phi;
    const Real reference_phi_hbya = reference_phi_hbya_before_phig + reference_phig_phi;
    const Real reference_pressure_eq_phi = sharp_rc->storedPressureEquationVolumetricFlux(*fi);
    const Real reference_final_phi = reference_phi_hbya + reference_pressure_eq_phi;
    const Real reference_writeback_phi = reference_pressure_eq_phi + reference_phig_phi;

    msg << fi->id()
        << "{c=" << fi->faceCentroid()
        << ",outer_phi=" << sharp_rc->storedOuterIterationPhi(*fi)
        << ",vof_transport_phi=" << sharp_rc->storedVOFTransportPhi(*fi)
        << ",pred_conv_phi=" << sharp_rc->storedPredictorConvectivePhi(*fi)
        << ",pred_phi=" << sharp_rc->storedPredictorOperatorPhi(*fi)
        << ",pressure_predictor_base_phi_internal=" << sharp_rc->storedPressurePredictorBasePhi(*fi)
        << ",pressure_predictor_base_phi_reference=" << -sharp_rc->storedPressurePredictorBasePhi(*fi)
        << ",transient_phi=" << sharp_rc->storedTransientProjectionFlux(*fi)
        << ",cap_hydro_phi=" << sharp_rc->storedCapillaryHydrostaticFlux(*fi)
        << ",phig_phi=" << sharp_rc->storedPhigFlux(*fi)
        << ",reference_HbyA_phi=" << reference_hbya_phi
        << ",reference_ddtCorr_phi=" << reference_ddt_corr_phi
        << ",reference_phig_phi=" << reference_phig_phi
        << ",reference_phiHbyA_before_phig=" << reference_phi_hbya_before_phig
        << ",reference_phiHbyA=" << reference_phi_hbya
        << ",reference_pEqn_flux=" << reference_pressure_eq_phi
        << ",reference_final_phi=" << reference_final_phi
        << ",reference_writeback_phi=" << reference_writeback_phi
        << ",reference_final_phi_mismatch="
        << sharp_rc->storedCorrectedFacePhi(*fi) - reference_final_phi
        << ",reference_writeback_phi_mismatch="
        << sharp_rc->storedPressureCorrectionPhi(*fi) - reference_writeback_phi
        << ",sn_grad_rho=" << sharp_rc->debugFaceNormalDensityGradient(*fi, Moose::currentState())
        << ",sn_grad_rho_orthogonal_part="
        << sharp_rc->debugFaceNormalDensityGradientOrthogonalPart(*fi, Moose::currentState())
        << ",sn_grad_rho_base_part="
        << sharp_rc->debugFaceNormalDensityGradientBasePart(*fi, Moose::currentState())
        << ",sn_grad_rho_correction_part="
        << sharp_rc->debugFaceNormalDensityGradientCorrectionPart(*fi, Moose::currentState())
        << ",sn_grad_rho_limited_correction_part="
        << sharp_rc->debugFaceNormalDensityGradientLimitedCorrectionPart(*fi, Moose::currentState())
        << ",gh=" << sharp_rc->debugHydrostaticGh(*fi)
        << ",normal_density_weighted_ainv="
        << sharp_rc->debugFaceNormalDensityWeightedAinv(*fi)
        << ",normal_raw_ainv=" << sharp_rc->debugFaceNormalRawAinv(*fi, Moose::currentState())
        << ",hydro_mass_flux_density_raw="
        << sharp_rc->debugHydrostaticFaceMassFluxDensityRaw(*fi)
        << ",p_elem_matrix=" << p_elem_matrix
        << ",p_neighbor_matrix=" << p_neighbor_matrix
        << ",p_elem_rhs=" << p_elem_rhs
        << ",pressure_eq_phi=" << sharp_rc->storedPressureEquationVolumetricFlux(*fi)
        << ",pressure_corr_phi=" << sharp_rc->storedPressureCorrectionPhi(*fi)
        << ",corrected_face_phi=" << sharp_rc->storedCorrectedFacePhi(*fi)
        << ",raw_rc=" << sharp_rc->rawRhieChowMassFlux(*fi)
        << ",alpha_e=" << sharp_rc->debugElemAlpha(*fi, Moose::currentState())
        << ",alpha_n=" << sharp_rc->debugNeighborAlpha(*fi, Moose::currentState())
        << ",rho_e=" << sharp_rc->debugElemDensity(*fi, Moose::currentState())
        << ",rho_n=" << sharp_rc->debugNeighborDensity(*fi, Moose::currentState())
        << ",pred_cache=" << sharp_rc->debugUsingCachedPredictorOperator();

    appendSharpCellDebug(msg,
                         ",",
                         "elem",
                         fi->elemInfo(),
                         _momentum_system_numbers,
                         *sharp_rc,
                         pressure_debug_face_cell_debug_keys);
    appendSharpCellDebug(msg,
                         ",",
                         "neighbor",
                         fi->neighborInfo(),
                         _momentum_system_numbers,
                         *sharp_rc,
                         pressure_debug_face_cell_debug_keys);

    msg << '}';
  }

  if (appended)
    msg << ']';
  else
    msg << " faces=[]";

  std::cerr << "[" << msg.str() << "]" << std::endl;
  _console << msg.str() << std::endl;
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
  std::cerr << "[ReducedPressurePIMPLESolve] solveVolumeFractionSystems begin"
            << " timeStep=" << _problem.timeStep() << " dt=" << _problem.dt() << std::endl;
  _console << name() << ": entering solveVolumeFractionSystems"
           << " timeStep=" << _problem.timeStep() << " dt=" << _problem.dt() << std::endl;

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
    std::cerr << "[ReducedPressurePIMPLESolve] vf system"
              << " name=" << _volume_fraction_system_names[i]
              << " corrector_found=" << (corrector ? 1 : 0) << std::endl;
    _console << name() << ": volume-fraction system '" << _volume_fraction_system_names[i]
             << "' corrector_found=" << (corrector ? 1 : 0) << std::endl;
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
        // OpenFOAM's MULES path bounds alpha through the limited face fluxes instead of
        // projecting the solved field. Do not apply the inherited scalar lower limiter here.
        residuals[i] = solveAdvectedSystem(_volume_fraction_system_numbers[i],
                                           *system,
                                           _volume_fraction_equation_relaxation[i],
                                           _volume_fraction_linear_control,
                                           _volume_fraction_l_abs_tol,
                                           1.0,
                                           std::numeric_limits<Real>::min());
        system->computeGradients();
        auto * curvature = sharpInterfaceCurvature();
        if (curvature)
          curvature->updateCurvatureMaps(_print_fields);
        corrector->applyCorrection(subcycle_dt, subcycle_dt / global_dt, curvature);
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

  if (_dump_pressure_outer_debug_csv)
    for (const auto & system_name : _volume_fraction_system_names)
      if (auto * corrector = sharpInterfaceVOFCorrector(system_name))
      {
        const auto audit = corrector->rhoPhiConsistencyAudit();
        _console << name() << ": rhoPhi audit for '" << corrector->variableName()
                 << "' after alpha finalization: L2=" << audit.l2_norm
                 << ", max_abs=" << audit.max_abs_mismatch;
        if (audit.has_worst_face)
          _console << ", worst_face=" << audit.worst_face_id << " @ ("
                   << audit.worst_face_centroid(0) << ", " << audit.worst_face_centroid(1)
                   << ", " << audit.worst_face_centroid(2)
                   << "), stored=" << audit.stored_rho_phi
                   << ", reconstructed=" << audit.reconstructed_rho_phi
                   << ", phi=" << audit.volumetric_phi
                   << ", alphaPhi=" << audit.limited_alpha_flux
                   << ", rho_g=" << audit.gas_density << ", rho_l=" << audit.liquid_density;
        _console << std::endl;
      }

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

  if (auto * curvature = sharpInterfaceCurvature())
    curvature->updateCurvatureMaps(_print_fields);

  for (const auto & system_name : _volume_fraction_system_names)
    if (auto * corrector = sharpInterfaceVOFCorrector(system_name))
      corrector->refreshPublishedRhoPhi();
}

void
ReducedPressurePIMPLESolve::clampVolumeFractionSystem(LinearSystem & system)
{
  auto & current_local_solution = *(system.system().current_local_solution);
  for (const auto i :
       make_range(current_local_solution.first_local_index(), current_local_solution.last_local_index()))
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
    // reference solver's pressureCorrector publishes phi, relaxes p_rgh, and writes back
    // U on every pimple.correct() pressure-correction pass.
    residual = applyPressureCorrectionStage(false, true, solver_params);
    if (piso_iteration_counter == 0)
      first_stage_residual = residual.second;
    if (!shouldContinuePISOIterations(
            piso_iteration_counter, residual.second, first_stage_residual))
      break;
    piso_iteration_counter++;
  }

  _current_piso_iteration = 0;

  return residual;
}

std::pair<unsigned int, Real>
ReducedPressurePIMPLESolve::correctStartupContinuityOnce(const bool subtract_updated_pressure,
                                                         const bool recompute_face_mass_flux,
                                                         const SolverParams & solver_params)
{
  if (_problem.timeStep() == 1)
    std::cerr << "[ReducedPressurePIMPLESolve] correctStartupContinuityOnce begin"
              << " recompute_face_mass_flux=" << recompute_face_mass_flux << std::endl;

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
    // Projection-only startup cleanup should mimic a bare CorrectPhi-like flux repair.
    sharp_rc->setSuppressStartupPressurePredictorFluxSources(true);
    sharp_rc->setSuppressExplicitHydrostaticPressureFlux(true);
  }

  preparePressureCorrectorState(subtract_updated_pressure);

  Moose::PetscSupport::petscSetOptions(_pressure_petsc_options, solver_params);

  std::pair<unsigned int, Real> residuals{0, std::numeric_limits<Real>::quiet_NaN()};
  unsigned int total_linear_iterations = 0;
  for (const auto nonorthogonal_iteration :
       make_range(_num_pressure_nonorthogonal_correctors + 1))
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

  if (_problem.timeStep() == 1)
    std::cerr << "[ReducedPressurePIMPLESolve] correctStartupContinuityOnce end"
              << " its=" << residuals.first << " residual=" << residuals.second << std::endl;

  return residuals;
}

std::pair<unsigned int, Real>
ReducedPressurePIMPLESolve::correctVelocityOnce(const bool subtract_updated_pressure,
                                                const bool recompute_face_mass_flux,
                                                const SolverParams & solver_params)
{
  if (_problem.timeStep() == 1)
    std::cerr << "[ReducedPressurePIMPLESolve] correctVelocityOnce begin"
              << " recompute_face_mass_flux=" << recompute_face_mass_flux << std::endl;

  preparePressureCorrectorState(subtract_updated_pressure);
  dumpPressureOuterDebugState("pre_pressure_solve");

  const auto residuals = applyPressureCorrectionStage(recompute_face_mass_flux, true, solver_params);

  if (_problem.timeStep() == 1)
    std::cerr << "[ReducedPressurePIMPLESolve] correctVelocityOnce end"
              << " its=" << residuals.first << " residual=" << residuals.second << std::endl;

  return residuals;
}

std::pair<unsigned int, Real>
ReducedPressurePIMPLESolve::applyPressureCorrectionStage(const bool recompute_face_mass_flux,
                                                         const bool publish_pressure_corrected_state,
                                                         const SolverParams & solver_params)
{
  if (_problem.timeStep() == 1)
    std::cerr << "[ReducedPressurePIMPLESolve] applyPressureCorrectionStage begin"
              << " recompute_face_mass_flux=" << recompute_face_mass_flux
              << " publish_pressure_corrected_state=" << publish_pressure_corrected_state
              << std::endl;

  Moose::PetscSupport::petscSetOptions(_pressure_petsc_options, solver_params);

  std::pair<unsigned int, Real> residuals{0, std::numeric_limits<Real>::quiet_NaN()};
  unsigned int total_linear_iterations = 0;

  // Mirror reference solver's pimple.correctNonOrthogonal() loop. Non-final solves
  // update the pressure field and pressure-equation flux cache only; the
  // accepted phi/U state is published exactly once on the final solve.
  for (const auto nonorthogonal_iteration :
       make_range(_num_pressure_nonorthogonal_correctors + 1))
  {
    const bool final_nonorthogonal_iteration =
        nonorthogonal_iteration == _num_pressure_nonorthogonal_correctors;

    if (_problem.timeStep() == 1)
      std::cerr << "[ReducedPressurePIMPLESolve] pressure non-orthogonal solve"
                << " iteration=" << (nonorthogonal_iteration + 1)
                << " final=" << final_nonorthogonal_iteration << std::endl;

    residuals = solvePressureCorrector();
    total_linear_iterations += residuals.first;

    auto & pressure_current_solution = *(_pressure_system.system().current_local_solution.get());
    _pressure_system.setSolution(pressure_current_solution);

    _pressure_system.computeGradients();
    _rc_uo->cachePressureEquationFlux();

    if (!final_nonorthogonal_iteration)
      continue;

    if (recompute_face_mass_flux && !publish_pressure_corrected_state)
    {
      _rc_uo->computeFaceMassFlux();

      if (auto * sharp_rc = sharpInterfaceRC())
        sharp_rc->applyAdditionalFaceMassFluxCorrection();
    }

    if (publish_pressure_corrected_state)
      publishPressureCorrectedTransportState("post_pressure_writeback");
  }

  residuals.first = total_linear_iterations;

  if (_problem.timeStep() == 1)
    std::cerr << "[ReducedPressurePIMPLESolve] applyPressureCorrectionStage end"
              << " its=" << residuals.first << " residual=" << residuals.second << std::endl;

  return residuals;
}

void
ReducedPressurePIMPLESolve::publishPressureCorrectedTransportState(const std::string & stage_label)
{
  if (_problem.timeStep() == 1)
    std::cerr << "[ReducedPressurePIMPLESolve] publishing pressure-corrected transport state"
              << " stage=" << stage_label << std::endl;

  auto & pressure_current_solution = *(_pressure_system.system().current_local_solution.get());
  _pressure_system.setSolution(pressure_current_solution);
  _pressure_system.computeGradients();
  _rc_uo->cachePressureEquationFlux();

  if (_problem.timeStep() == 1)
    std::cerr << "[ReducedPressurePIMPLESolve] computing final corrected face mass flux"
              << std::endl;
  _rc_uo->computeFaceMassFlux();
  if (auto * sharp_rc = sharpInterfaceRC())
    sharp_rc->applyAdditionalFaceMassFluxCorrection();

  // Match reference solver's final pressure-corrector ordering:
  //   phi = phiHbyA + pEqn.flux();
  //   p_rgh.relax();
  //   U = HbyA + rAU*reconstruct((phig + pEqn.flux())/rAUf);
  //
  // The face flux and velocity writeback must keep using the cached pEqn.flux
  // from the unrelaxed pressure solve. relaxPressureFieldForNextPredictor()
  // refreshes pressure gradients for the next predictor, but does not invalidate
  // the cached final pressure-equation flux.
  relaxPressureFieldForNextPredictor();

  if (_problem.timeStep() == 1)
    std::cerr << "[ReducedPressurePIMPLESolve] computing pressure-corrected cell velocity"
              << std::endl;
  _rc_uo->computeCellVelocity();

  if (_problem.timeStep() == 1)
    std::cerr << "[ReducedPressurePIMPLESolve] updating velocity boundary state" << std::endl;
  _rc_uo->updateVelocityBoundaryState();

  reportReferenceContinuityErrors(stage_label);
  correctMovingMeshFaceVelocityAndMakeRelative();

  if (_problem.timeStep() == 1)
    std::cerr << "[ReducedPressurePIMPLESolve] dumping post-pressure debug state" << std::endl;
  dumpPressureOuterDebugState(stage_label);
  printPressureHbyAParitySummary(stage_label);
}

void
ReducedPressurePIMPLESolve::reportReferenceContinuityErrors(const std::string & stage_label)
{
  if (!_rc_uo)
    return;

  std::unordered_map<dof_id_type, Real> cell_integrated_divergence;
  std::unordered_map<dof_id_type, Real> cell_volume;
  auto has_pressure_dof = [this](const ElemInfo & elem_info)
  {
    return elem_info.dofIndices().size() > static_cast<std::size_t>(_pressure_sys_number) &&
           !elem_info.dofIndices()[_pressure_sys_number].empty();
  };

  for (const auto * fi : _rc_uo->flowFacesForAudit())
  {
    if (!fi)
      continue;

    const Real integrated_phi =
        _rc_uo->getVolumetricFaceFlux(*fi) * fi->faceArea() * fi->faceCoord();

    if (fi->elemPtr())
    {
      const auto & elem_info = *fi->elemInfo();
      if (has_pressure_dof(elem_info))
      {
        cell_integrated_divergence[fi->elemPtr()->id()] += integrated_phi;
        cell_volume[fi->elemPtr()->id()] = elem_info.volume() * elem_info.coordFactor();
      }
    }

    if (fi->neighborPtr())
    {
      const auto & neighbor_info = *fi->neighborInfo();
      if (has_pressure_dof(neighbor_info))
      {
        cell_integrated_divergence[fi->neighborPtr()->id()] -= integrated_phi;
        cell_volume[fi->neighborPtr()->id()] =
            neighbor_info.volume() * neighbor_info.coordFactor();
      }
    }
  }

  Real volume_sum = 0.0;
  Real local_divergence_integral = 0.0;
  Real global_divergence_integral = 0.0;
  for (const auto & [elem_id, integrated_divergence] : cell_integrated_divergence)
  {
    const auto volume_it = cell_volume.find(elem_id);
    if (volume_it == cell_volume.end() || volume_it->second <= libMesh::TOLERANCE)
      continue;

    volume_sum += volume_it->second;
    local_divergence_integral += std::abs(integrated_divergence);
    global_divergence_integral += integrated_divergence;
  }

  const Real dt = _problem.dt();
  const Real local_continuity_error =
      volume_sum > std::numeric_limits<Real>::epsilon()
          ? dt * local_divergence_integral / volume_sum
          : 0.0;
  const Real global_continuity_error =
      volume_sum > std::numeric_limits<Real>::epsilon()
          ? dt * global_divergence_integral / volume_sum
          : 0.0;
  _cumulative_continuity_error += global_continuity_error;

  _console << "time step continuity errors"
           << ": stage=" << stage_label
           << ", sum local = " << local_continuity_error
           << ", global = " << global_continuity_error
           << ", cumulative = " << _cumulative_continuity_error << std::endl;
}

void
ReducedPressurePIMPLESolve::correctMovingMeshFaceVelocityAndMakeRelative()
{
  // This parity path currently has no Uf/MRF/moving-mesh flux state. RhieChowMassFlux explicitly
  // reports no mesh-velocity support, so for the stationary-mesh cases this path supports,
  // reference solver's
  //   fvc::correctUf(Uf, U, phi, MRF);
  //   fvc::makeRelative(phi, U);
  // are exact no-ops.
}

void
ReducedPressurePIMPLESolve::relaxPressureFieldForNextPredictor()
{
  auto & pressure_current_solution = *(_pressure_system.system().current_local_solution.get());
  auto & pressure_old_solution = *(_pressure_system.solutionPreviousNewton());

  if (_problem.timeStep() == 1)
    std::cerr << "[ReducedPressurePIMPLESolve] relaxing pressure solution update" << std::endl;
  NS::FV::relaxSolutionUpdate(
      pressure_current_solution, pressure_old_solution, _pressure_variable_relaxation);

  pressure_old_solution = pressure_current_solution;
  _pressure_system.setSolution(pressure_current_solution);
  _pressure_system.computeGradients();
}
