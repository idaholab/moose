#include "WCNSLinearFVConservativeSharpInterfaceFlowPhysics.h"

#include "GeneralUserObject.h"
#include "Executioner.h"
#include "INSFVMomentumAdvection.h"
#include "INSFVTimeKernel.h"
#include "MapConversionUtils.h"
#include "MooseMesh.h"
#include "MooseUtils.h"
#include "NS.h"
#include "NSFVBase.h"
#include "ConservativeSharpInterfaceRhieChowMassFlux.h"
#include "TheWarehouse.h"

#include <algorithm>
#include <cctype>

registerWCNSFVFlowPhysicsBaseTasks("NavierStokesApp",
                                   WCNSLinearFVConservativeSharpInterfaceFlowPhysics);
registerMooseAction(
    "NavierStokesApp", WCNSLinearFVConservativeSharpInterfaceFlowPhysics, "add_linear_fv_kernel");
registerMooseAction(
    "NavierStokesApp", WCNSLinearFVConservativeSharpInterfaceFlowPhysics, "add_linear_fv_bc");
registerMooseAction(
    "NavierStokesApp",
    WCNSLinearFVConservativeSharpInterfaceFlowPhysics,
    "add_functor_material");

namespace
{
bool
containsDynamicContactAngleModel(const std::vector<std::string> & models)
{
  return std::any_of(models.begin(), models.end(), [](const std::string & model)
  {
    std::string lower = model;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c)
    {
      return static_cast<char>(std::tolower(c));
    });
    return lower == "dynamic";
  });
}

bool
blocksOverlap(const std::set<SubdomainID> & lhs, const std::set<SubdomainID> & rhs)
{
  if (lhs.empty() || rhs.empty() || lhs.count(Moose::ANY_BLOCK_ID) || rhs.count(Moose::ANY_BLOCK_ID))
    return true;

  auto lhs_it = lhs.begin();
  auto rhs_it = rhs.begin();
  while (lhs_it != lhs.end() && rhs_it != rhs.end())
  {
    if (*lhs_it < *rhs_it)
      ++lhs_it;
    else if (*rhs_it < *lhs_it)
      ++rhs_it;
    else
      return true;
  }

  return false;
}

std::string
sanitizeFunctorLabel(const std::string & input)
{
  std::string result = input;
  std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c)
  {
    return std::isalnum(c) ? static_cast<char>(c) : '_';
  });
  return result;
}
}

InputParameters
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::validParams()
{
  InputParameters params = WCNSFVFlowPhysicsBase::validParams();

  params.addClassDescription(
      "Define a linear-FV segregated sharp-interface flow solve whose primary momentum unknowns "
      "are the conservative rho*u components.");
  params.set<std::vector<std::string>>("velocity_variable") =
      std::vector<std::string>(NS::momentum_vector, NS::momentum_vector + 3);

  params.addParam<bool>(
      "orthogonality_correction", false, "Whether to use orthogonality correction");
  params.renameParam("orthogonality_correction", "use_nonorthogonal_correction", "");
  params.set<unsigned short>("ghost_layers") = 1;

  // Large-density-ratio sharp-interface work should default to a reduced / dynamic pressure solve.
  params.set<bool>("solve_for_dynamic_pressure") = true;
  params.transferParam<MooseEnum>(RhieChowMassFlux::validParams(), "pressure_projection_method");
  params.transferParam<bool>(RhieChowMassFlux::validParams(),
                             "use_cached_momentum_predictor_operator");
  params.set<bool>("use_cached_momentum_predictor_operator") = true;

  MooseEnum pressure_formulation("reduced total", "reduced");
  params.addParam<MooseEnum>(
      "pressure_formulation",
      pressure_formulation,
      "Whether the solved pressure variable is a reduced pressure or the total pressure.");

  params.addParam<bool>(
      "add_transient_projection_flux",
      true,
      "Whether to add an extra pressure-equation face-flux divergence for the transient "
      "projection.");
  params.addParam<bool>(
      "add_capillary_hydrostatic_flux",
      true,
      "Whether to add an extra pressure-equation face-flux divergence for capillary / hydrostatic "
      "predictor terms.");
  params.transferParam<bool>(ConservativeSharpInterfaceRhieChowMassFlux::validParams(),
                             "apply_pressure_velocity_writeback");
  params.transferParam<bool>(ConservativeSharpInterfaceRhieChowMassFlux::validParams(),
                             "apply_pressure_face_flux_correction");
  params.addParam<bool>(
      "use_interfoam_predictor_contract",
      true,
      "Sharp-interface reduced-pressure flow uses the live unsplit momentum predictor and does "
      "not enable any sharp-only split/direct predictor forcing variants.");
  params.addParam<bool>(
      "add_momentum_continuity_error_sink",
      false,
      "Whether to add the conservative momentum continuity-error sink conditioning term. This is "
      "not part of standard interFoam's momentum equation.");
  params.transferParam<MooseFunctorName>(ConservativeSharpInterfaceRhieChowMassFlux::validParams(),
                                         "vof_rho_phi_functor");
  params.transferParam<MooseFunctorName>(ConservativeSharpInterfaceRhieChowMassFlux::validParams(),
                                         "volume_fraction_functor");
  params.transferParam<Real>(ConservativeSharpInterfaceRhieChowMassFlux::validParams(),
                             "near_interface_lower");
  params.transferParam<Real>(ConservativeSharpInterfaceRhieChowMassFlux::validParams(),
                             "near_interface_upper");
  params.transferParam<MooseFunctorName>(ConservativeSharpInterfaceRhieChowMassFlux::validParams(),
                                         "vof_alpha_phi_limited_functor");
  params.transferParam<MooseFunctorName>(ConservativeSharpInterfaceRhieChowMassFlux::validParams(),
                                         "liquid_density_functor");
  params.transferParam<MooseFunctorName>(ConservativeSharpInterfaceRhieChowMassFlux::validParams(),
                                         "gas_density_functor");

  params.addParam<MooseFunctorName>(
      "transient_projection_face_acceleration",
      "",
      "Optional face-based transient projection correction functor in acceleration-like units.");
  params.addParam<MooseFunctorName>(
      "surface_tension_face_acceleration",
      "",
      "Optional face-based surface-tension contribution functor in acceleration-like units. If "
      "left empty and the sharp-interface geometry functors are enabled, this physics object will "
      "auto-generate the functor name and pass it to the Rhie-Chow object.");
  params.addParam<MooseFunctorName>(
      "surface_tension_cell_acceleration",
      "",
      "Optional cell-based surface-tension contribution functor in acceleration-like units. If "
      "left empty and the sharp-interface geometry functors are enabled, this physics object will "
      "auto-generate the functor name and pass it to the Rhie-Chow object.");
  params.addParam<MooseFunctorName>(
      "hydrostatic_density_gradient_face_acceleration",
      "",
      "Optional face-based hydrostatic density-gradient contribution functor in acceleration-like "
      "units. If left empty and the sharp-interface geometry functors are enabled, this physics "
      "object will auto-generate the functor name and pass it to the Rhie-Chow object.");
  params.addParam<MooseFunctorName>(
      "hydrostatic_density_gradient_cell_acceleration",
      "",
      "Optional cell-based hydrostatic density-gradient contribution functor in acceleration-like "
      "units. If left empty and the sharp-interface geometry functors are enabled, this physics "
      "object will auto-generate the functor name and pass it to the Rhie-Chow object.");
  params.addParam<MooseFunctorName>(
      "surface_tension_momentum_source_x",
      "",
      "Optional x-component capillary momentum-source density functor for the reduced-pressure "
      "momentum predictor. If left empty and the sharp-interface geometry functors are enabled, "
      "this physics object will auto-generate the functor name.");
  params.addParam<MooseFunctorName>(
      "surface_tension_momentum_source_y",
      "",
      "Optional y-component capillary momentum-source density functor for the reduced-pressure "
      "momentum predictor. If left empty and the sharp-interface geometry functors are enabled, "
      "this physics object will auto-generate the functor name.");
  params.addParam<MooseFunctorName>(
      "surface_tension_momentum_source_z",
      "",
      "Optional z-component capillary momentum-source density functor for the reduced-pressure "
      "momentum predictor. If left empty and the sharp-interface geometry functors are enabled, "
      "this physics object will auto-generate the functor name.");
  params.addParam<MooseFunctorName>(
      "hydrostatic_momentum_source_x",
      "",
      "Optional x-component hydrostatic momentum-source density functor for the reduced-pressure "
      "momentum predictor. If left empty and the sharp-interface geometry functors are enabled, "
      "this physics object will auto-generate the functor name.");
  params.addParam<MooseFunctorName>(
      "hydrostatic_momentum_source_y",
      "",
      "Optional y-component hydrostatic momentum-source density functor for the reduced-pressure "
      "momentum predictor. If left empty and the sharp-interface geometry functors are enabled, "
      "this physics object will auto-generate the functor name.");
  params.addParam<MooseFunctorName>(
      "hydrostatic_momentum_source_z",
      "",
      "Optional z-component hydrostatic momentum-source density functor for the reduced-pressure "
      "momentum predictor. If left empty and the sharp-interface geometry functors are enabled, "
      "this physics object will auto-generate the functor name.");

  params.addParam<bool>(
      "create_geometry_functors",
      true,
      "Whether to automatically add the sharp-interface geometry functor material that produces "
      "face normals and capillary / hydrostatic face accelerations.");
  params.addParam<MooseFunctorName>(
      "volume_fraction_functor",
      "",
      "Volume-fraction / phase-fraction functor used by the geometry material and curvature "
      "producer. Leave empty to skip automatic creation of sharp-interface geometry objects.");
  params.addParam<MooseFunctorName>(
      "surface_tension_coefficient",
      "0",
      "Surface-tension coefficient functor or numeric constant string used by the geometry "
      "material.");
  params.addParam<MooseFunctorName>(
      "curvature_functor",
      "",
      "Optional curvature functor consumed by the geometry material. If left empty and the "
      "curvature producer is enabled, this physics object will auto-generate the curvature "
      "functor from the curvature calculator user object.");
  params.addParam<bool>(
      "create_curvature_producer",
      true,
      "Whether to automatically add the curvature producer user object when a volume_fraction_"
      "functor is supplied.");
  params.addParam<MooseEnum>(
      "curvature_delta_n_mode",
      MooseEnum("mesh_scaled_openfoam fixed", "mesh_scaled_openfoam"),
      "How the curvature producer chooses the normal regularization delta_n.");
  params.addParam<Real>(
      "curvature_delta_n_scale",
      1e-8,
      "Scale factor used by the curvature producer when curvature_delta_n_mode = "
      "mesh_scaled_openfoam. The effective delta_n becomes scale / cbrt(average cell volume), "
      "matching OpenFOAM's default interfaceProperties setup.");
  params.addParam<Real>(
      "curvature_delta_n_fixed_value",
      1e-8,
      "Fixed delta_n value used by the curvature producer when curvature_delta_n_mode = fixed.");
  params.addParam<bool>(
      "use_openfoam_simple_curvature",
      true,
      "Use the baseline OpenFOAM curvature expression K = -div(nHatf). This should remain true "
      "for parity with the shipped solver path.");
  params.addParam<unsigned int>(
      "n_alpha_smooth_curvature",
      0,
      "Optional number of curvature-input smoothing sweeps performed by the curvature producer "
      "before building grad(alpha). A value of 0 leaves the curvature producer on the baseline "
      "path, while positive values provide additional alpha smoothing for difficult capillary "
      "cases.");

  params.addParam<std::vector<BoundaryName>>(
      "contact_angle_boundaries",
      {},
      "Boundary names or IDs on which wall contact-angle correction should be applied inside the "
      "curvature producer.");
  params.addParam<std::vector<std::string>>(
      "contact_angle_models",
      {},
      "Optional per-boundary contact-angle model names. Entries equal to 'dynamic' activate the "
      "dynamic wall-contact-angle material on the corresponding boundary. Boundaries without a "
      "dynamic entry may still use static_contact_angles_degrees.");
  params.addParam<std::vector<Real>>(
      "static_contact_angles_degrees",
      {},
      "Static wall contact angles in degrees, one per selected boundary. A single value is "
      "broadcast to all selected boundaries.");
  params.addParam<MooseFunctorName>(
      "wall_contact_angle_degrees_functor",
      "",
      "Optional face-aware functor returning wall contact angle in degrees. When supplied, it "
      "overrides static_contact_angles_degrees and disables automatic creation of the dynamic "
      "wall-contact-angle functor material.");
  params.addParam<Real>(
      "contact_angle_small_det",
      1e-12,
      "Positive floor used when the OpenFOAM-style wall-contact-angle determinant 1 - "
      "(nHat.nWall)^2 becomes very small.");

  params.addParam<bool>(
      "create_dynamic_contact_angle_functor_material",
      true,
      "Whether to automatically add a dynamic wall-contact-angle functor material when at least "
      "one contact_angle_models entry is 'dynamic' and no explicit wall_contact_angle_degrees_"
      "functor is provided.");
  params.addParam<std::vector<Real>>(
      "equilibrium_contact_angles_deg",
      {},
      "Per-boundary equilibrium contact angles theta0 in degrees for dynamic contact-angle "
      "boundaries.");
  params.addParam<std::vector<Real>>(
      "advancing_contact_angles_deg",
      {},
      "Per-boundary advancing contact angles thetaA in degrees for dynamic contact-angle "
      "boundaries.");
  params.addParam<std::vector<Real>>(
      "receding_contact_angles_deg",
      {},
      "Per-boundary receding contact angles thetaR in degrees for dynamic contact-angle "
      "boundaries.");
  params.addParam<std::vector<Real>>(
      "contact_angle_velocity_scales",
      {},
      "Per-boundary velocity scales uTheta for the dynamic wall-contact-angle law.");
  params.addParam<MooseFunctorName>(
      "dynamic_contact_angle_wall_velocity_functor",
      "",
      "Optional wall-velocity vector functor used by the dynamic wall-contact-angle law.");
  params.addParam<RealVectorValue>(
      "dynamic_contact_angle_default_wall_velocity",
      RealVectorValue(),
      "Constant fallback wall velocity used by the dynamic wall-contact-angle law.");
  params.addParam<Real>(
      "dynamic_contact_angle_parallel_direction_small",
      1e-12,
      "Positive regularization added when normalizing the wall-parallel interface direction in "
      "the dynamic wall-contact-angle law.");
  params.addParam<Real>(
      "dynamic_contact_angle_u_theta_small",
      1e-12,
      "Positive floor below which the dynamic wall-contact-angle law falls back to theta0.");

  params.addParam<Real>(
      "geometry_delta_n",
      1e-8,
      "Regularization used in the geometry material when constructing interface unit normals if "
      "no precomputed unit-normal functor is supplied.");
  params.addParam<Real>(
      "geometry_minimum_density",
      1e-12,
      "Positive floor used in the geometry material when dividing by density.");
  params.addParam<bool>(
      "clip_volume_fraction_for_geometry",
      true,
      "Whether to clip the volume fraction in the geometry material before forming indicators.");
  params.addParam<Real>(
      "geometry_alpha_lower_bound", 0.0, "Lower clipping bound for the volume fraction.");
  params.addParam<Real>(
      "geometry_alpha_upper_bound", 1.0, "Upper clipping bound for the volume fraction.");
  params.addParam<Real>(
      "near_interface_lower",
      0.01,
      "Lower threshold for the OpenFOAM-like near-interface indicator.");
  params.addParam<Real>(
      "near_interface_upper",
      0.99,
      "Upper threshold for the OpenFOAM-like near-interface indicator.");

  params.addParamNamesToGroup(
      "pressure_formulation add_transient_projection_flux "
      "add_capillary_hydrostatic_flux apply_pressure_velocity_writeback "
      "apply_pressure_face_flux_correction transient_projection_face_acceleration "
      "use_interfoam_predictor_contract "
      "surface_tension_face_acceleration surface_tension_cell_acceleration "
      "hydrostatic_density_gradient_face_acceleration "
      "hydrostatic_density_gradient_cell_acceleration",
      "Sharp Interface Pressure Correction");

  params.addParamNamesToGroup(
      "surface_tension_momentum_source_x surface_tension_momentum_source_y "
      "surface_tension_momentum_source_z hydrostatic_momentum_source_x "
      "hydrostatic_momentum_source_y hydrostatic_momentum_source_z",
      "Sharp Interface Momentum Predictor");

  params.addParamNamesToGroup(
      "create_geometry_functors volume_fraction_functor surface_tension_coefficient "
      "curvature_functor create_curvature_producer curvature_delta_n_mode "
      "curvature_delta_n_scale curvature_delta_n_fixed_value use_openfoam_simple_curvature "
      "n_alpha_smooth_curvature contact_angle_boundaries contact_angle_models "
      "static_contact_angles_degrees wall_contact_angle_degrees_functor contact_angle_small_det "
      "create_dynamic_contact_angle_functor_material equilibrium_contact_angles_deg "
      "advancing_contact_angles_deg receding_contact_angles_deg "
      "contact_angle_velocity_scales dynamic_contact_angle_wall_velocity_functor "
      "dynamic_contact_angle_default_wall_velocity "
      "dynamic_contact_angle_parallel_direction_small dynamic_contact_angle_u_theta_small "
      "geometry_delta_n geometry_minimum_density clip_volume_fraction_for_geometry "
      "geometry_alpha_lower_bound geometry_alpha_upper_bound near_interface_lower "
      "near_interface_upper",
      "Sharp Interface Geometry");

  return params;
}

WCNSLinearFVConservativeSharpInterfaceFlowPhysics::WCNSLinearFVConservativeSharpInterfaceFlowPhysics(
    const InputParameters & parameters)
  : WCNSFVFlowPhysicsBase(parameters),
    _non_orthogonal_correction(getParam<bool>("use_nonorthogonal_correction")),
    _pressure_formulation(getParam<MooseEnum>("pressure_formulation")),
    _add_transient_projection_flux(getParam<bool>("add_transient_projection_flux")),
    _add_capillary_hydrostatic_flux(getParam<bool>("add_capillary_hydrostatic_flux")),
    _create_geometry_functors(getParam<bool>("create_geometry_functors")),
    _create_curvature_producer(getParam<bool>("create_curvature_producer")),
    _create_dynamic_contact_angle_functor_material(
        getParam<bool>("create_dynamic_contact_angle_functor_material"))
{
  if (_porous_medium_treatment)
    paramError("porous_medium_treatment", "Porous media unsupported");

  if (!_has_flow_equations)
    mooseError("This sharp-interface flow physics requires flow equations to be enabled.");

  if (_hydraulic_separators.size())
    paramError(
        "hydraulic_separator_sidesets", "Flow separators are not supported yet for linear FV.");

  if (getParam<bool>("pin_pressure"))
    paramError("pin_pressure",
               "Pressure pinning is implemented in the executioner for the linear FV segregated "
               "solves.");

  if (_pressure_formulation == "reduced" && !_solve_for_dynamic_pressure)
    paramError("solve_for_dynamic_pressure",
               "pressure_formulation = 'reduced' requires solve_for_dynamic_pressure = true.");

  if (_pressure_formulation == "total" && _solve_for_dynamic_pressure)
    paramError("solve_for_dynamic_pressure",
               "pressure_formulation = 'total' requires solve_for_dynamic_pressure = false.");

  if (!getParam<bool>("use_interfoam_predictor_contract"))
    paramError("use_interfoam_predictor_contract",
               "The sharp-interface reduced-pressure path now follows the interFoam-style live "
               "unsplit predictor contract only.");

  const auto & contact_angle_models = getParam<std::vector<std::string>>("contact_angle_models");
  const bool has_dynamic_model = containsDynamicContactAngleModel(contact_angle_models);

  if (has_dynamic_model && !getParam<MooseFunctorName>("wall_contact_angle_degrees_functor").empty())
    paramError("wall_contact_angle_degrees_functor",
               "wall_contact_angle_degrees_functor cannot be supplied together with a dynamic "
               "entry in contact_angle_models. Provide either the explicit wall-angle functor or "
               "the automatic dynamic-contact-angle setup, but not both.");

  if (has_dynamic_model && getParam<std::vector<BoundaryName>>("contact_angle_boundaries").empty())
    paramError("contact_angle_boundaries",
               "contact_angle_boundaries must be supplied when contact_angle_models contains a "
               "dynamic entry.");

  if (has_dynamic_model && !_create_curvature_producer)
    paramError("create_curvature_producer",
               "Automatic dynamic wall-contact-angle creation requires create_curvature_producer = "
               "true so the provisional face unit normal is available.");

  if (has_dynamic_model && getParam<MooseFunctorName>("volume_fraction_functor").empty())
    paramError("volume_fraction_functor",
               "Automatic dynamic wall-contact-angle creation requires volume_fraction_functor so "
               "the curvature producer can be built.");
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::initializePhysicsAdditional()
{
  WCNSFVFlowPhysicsBase::initializePhysicsAdditional();

  getProblem().needSolutionState(2, Moose::SolutionIterationType::Nonlinear);

  if (!isParamSetByUser("system_names"))
  {
    if (dimension() == 1)
      _system_names = {"u_system", "pressure_system"};
    else if (dimension() == 2)
      _system_names = {"u_system", "v_system", "pressure_system"};
    else if (dimension() == 3)
      _system_names = {"u_system", "v_system", "w_system", "pressure_system"};
  }
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addSolverVariables()
{
  if (!_has_flow_equations)
    return;

  for (const auto d : make_range(dimension()))
    saveSolverVariableName(_velocity_names[d]);
  saveSolverVariableName(_pressure_name);

  for (const auto d : make_range(dimension()))
  {
    if (!shouldCreateVariable(_velocity_names[d], _blocks, /*error if aux*/ true))
      reportPotentiallyMissedParameters({"system_names"}, "MooseLinearVariableFVReal");
    else if (_define_variables)
    {
      const std::string variable_type = "MooseLinearVariableFVReal";
      auto params = getFactory().getValidParams(variable_type);
      assignBlocks(params, _blocks);
      params.set<SolverSystemName>("solver_sys") = getSolverSystem(_velocity_names[d]);
      getProblem().addVariable(variable_type, _velocity_names[d], params);
    }
    else
      paramError("velocity_variable",
                 "Variable (" + _velocity_names[d] +
                 ") supplied to the WCNSLinearFVConservativeSharpInterfaceFlowPhysics does not "
                 "exist!");

  }

  if (!shouldCreateVariable(_pressure_name, _blocks, /*error if aux*/ true))
    reportPotentiallyMissedParameters({"system_names"}, "MooseLinearVariableFVReal");
  else if (_define_variables)
  {
    const std::string pressure_type = "MooseLinearVariableFVReal";
    auto params = getFactory().getValidParams(pressure_type);
    assignBlocks(params, _blocks);
    params.set<SolverSystemName>("solver_sys") = getSolverSystem(_pressure_name);
    getProblem().addVariable(pressure_type, _pressure_name, params);
  }
  else
    paramError("pressure_variable",
               "Variable (" + _pressure_name +
                   ") supplied to the WCNSLinearFVConservativeSharpInterfaceFlowPhysics does not "
                   "exist!");
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addFVKernels()
{
  if (!_has_flow_equations)
    return;

  const bool use_face_based_reduced_pressure_predictor =
      _solve_for_dynamic_pressure && _pressure_formulation == "reduced" &&
      _add_capillary_hydrostatic_flux;

  addPressureCorrectionKernels();

  if (isTransient())
    addMomentumTimeKernels();

  addMomentumFluxKernels();
  if (isTransient())
    if (useMomentumContinuityErrorSink())
      addMomentumConditioningKernels();
  if (!use_face_based_reduced_pressure_predictor)
    addMomentumPressureKernels();

  if (_friction_types.size())
    addMomentumFrictionKernels();

  addMomentumGravityKernels();
  if (use_face_based_reduced_pressure_predictor)
    addMomentumFaceBasedReducedPressureKernels();
  else
    addMomentumReducedPressureKernels();

  if (getParam<bool>("boussinesq_approximation"))
    addMomentumBoussinesqKernels();
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addUserObjects()
{
  addCurvatureUserObject();
  addRhieChowUserObjects();
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addPressureCorrectionKernels()
{
  {
    const std::string kernel_type = "LinearFVAnisotropicDiffusion";
    const std::string kernel_name = prefix() + "p_diffusion";

    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") = _pressure_name;
    params.set<MooseFunctorName>("diffusion_tensor") = "pressure_Ainv";
    params.set<bool>("use_nonorthogonal_correction") = _non_orthogonal_correction;

    getProblem().addLinearFVKernel(kernel_type, kernel_name, params);
  }

  {
    const std::string kernel_type = "LinearFVDivergence";
    const std::string kernel_name = prefix() + "HbyA_divergence";

    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") = _pressure_name;
    params.set<MooseFunctorName>("face_flux") = "pressure_predictor_flux";
    params.set<bool>("force_boundary_execution") = true;

    getProblem().addLinearFVKernel(kernel_type, kernel_name, params);
  }

  // The sharp path now follows the base live operator contract:
  // HbyA stays in the base predictor functor, and any explicit sharp phig
  // branches are added to phiHbyA by the Rhie-Chow user object.
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addMomentumTimeKernels()
{
  const std::string kernel_type = "LinearFVTimeDerivative";
  const std::string kernel_name = prefix() + "ins_momentum_time";

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>("factor") = "1";

  for (const auto d : make_range(dimension()))
  {
    params.set<LinearVariableName>("variable") = _velocity_names[d];
    if (shouldCreateTimeDerivative(_velocity_names[d], _blocks, false))
      getProblem().addLinearFVKernel(kernel_type, kernel_name + "_" + NS::directions[d], params);
  }
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addMomentumFluxKernels()
{
  const std::string u_names[3] = {"u", "v", "w"};
  const std::string kernel_type = "LinearWCNSFVConservativeMomentumFlux";
  const std::string kernel_name = prefix() + "ins_momentum_flux_";
  const bool use_venkat_deferred_correction = _momentum_advection_interpolation == "venkatakrishnan";
  const InterpolationMethodName advected_interp_method_name =
      prefix() + "momentum_advection_deferred_correction";

  if (use_venkat_deferred_correction && !getProblem().hasFVInterpolationMethod(advected_interp_method_name))
  {
    const std::string method_type = "FVAdvectedVenkatakrishnanDeferredCorrection";
    InputParameters method_params = getFactory().getValidParams(method_type);
    getProblem().addFVInterpolationMethod(method_type, advected_interp_method_name, method_params);
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);

  if (!_turbulence_physics)
    params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;
  else
    params.set<MooseFunctorName>(NS::mu) = NS::mu_eff;
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<Real>("minimum_density") = getParam<Real>("geometry_minimum_density");
  if (shouldCreateGeometryFunctorMaterial())
    params.set<MooseFunctorName>("density_gradient_functor") =
        generatedGeometryFunctorName("density_gradient");

  params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
  params.set<MooseFunctorName>("mass_flux_functor") = momentumTransportMassFluxFunctorName();

  params.set<MooseEnum>("advected_interp_method") = _momentum_advection_interpolation;
  if (use_venkat_deferred_correction)
    params.set<InterpolationMethodName>("advected_interp_method_name") =
        advected_interp_method_name;
  params.set<bool>("use_nonorthogonal_correction") = _non_orthogonal_correction;
  params.set<bool>("use_deviatoric_terms") = includeSymmetrizedViscousStress();

  for (unsigned int i = 0; i < dimension(); ++i)
    params.set<SolverVariableName>(u_names[i]) = _velocity_names[i];

  for (const auto d : make_range(dimension()))
  {
    params.set<LinearVariableName>("variable") = _velocity_names[d];
    params.set<MooseEnum>("momentum_component") = NS::directions[d];
    getProblem().addLinearFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addMomentumConditioningKernels()
{
  const std::string kernel_type = "LinearFVContinuityErrorSink";
  const std::string kernel_name = prefix() + "ins_momentum_continuity_error_sink_";

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>("coeff") = "conservative_continuity_error_over_density";

  for (const auto d : make_range(dimension()))
  {
    params.set<LinearVariableName>("variable") = _velocity_names[d];
    getProblem().addLinearFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addMomentumPressureKernels()
{
  const std::string kernel_type = "LinearFVMomentumPressure";
  const std::string kernel_name = prefix() + "ins_momentum_pressure_";

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<VariableName>("pressure") = _pressure_name;

  for (const auto d : make_range(dimension()))
  {
    params.set<MooseEnum>("momentum_component") = NS::directions[d];
    params.set<LinearVariableName>("variable") = _velocity_names[d];
    getProblem().addLinearFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addMomentumGravityKernels()
{
  if (parameters().isParamValid("gravity") && !_solve_for_dynamic_pressure)
  {
    const std::string kernel_type = "LinearFVSource";
    const std::string kernel_name = prefix() + "ins_momentum_gravity_";

    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);

    const auto gravity_vector = getParam<RealVectorValue>("gravity");
    const std::vector<std::string> comp_axis({"x", "y", "z"});

    for (const auto d : make_range(dimension()))
      if (gravity_vector(d) != 0)
      {
        params.set<LinearVariableName>("variable") = _velocity_names[d];
        params.set<MooseFunctorName>("source_density") = "rho_g_" + comp_axis[d];
        getProblem().addLinearFVKernel(kernel_type, kernel_name + comp_axis[d], params);
      }
  }
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addMomentumReducedPressureKernels()
{
  if (!_solve_for_dynamic_pressure || _pressure_formulation != "reduced" ||
      !_add_capillary_hydrostatic_flux)
    return;

  const std::string kernel_type = "LinearFVSource";
  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);

  const std::vector<std::string> comp_axis({"x", "y", "z"});
  const auto resolve_source_name = [this](const std::string & param_name,
                                          const std::string & generated_base_name)
  {
    const auto explicit_name = getParam<MooseFunctorName>(param_name);
    if (!explicit_name.empty())
      return explicit_name;

    return shouldCreateGeometryFunctorMaterial() ? generatedGeometryFunctorName(generated_base_name)
                                                 : MooseFunctorName("");
  };

  for (const auto d : make_range(dimension()))
  {
    params.set<LinearVariableName>("variable") = _velocity_names[d];

    const auto capillary_source_name = resolve_source_name("surface_tension_momentum_source_" +
                                                               comp_axis[d],
                                                           "surface_tension_momentum_source_" +
                                                               comp_axis[d]);
    if (!capillary_source_name.empty())
    {
      params.set<MooseFunctorName>("source_density") = capillary_source_name;
      getProblem().addLinearFVKernel(kernel_type,
                                     prefix() + "ins_momentum_capillary_source_" + comp_axis[d],
                                     params);
    }

    const auto hydrostatic_source_name = resolve_source_name("hydrostatic_momentum_source_" +
                                                                 comp_axis[d],
                                                             "hydrostatic_momentum_source_" +
                                                                 comp_axis[d]);
    if (!hydrostatic_source_name.empty())
    {
      params.set<MooseFunctorName>("source_density") = hydrostatic_source_name;
      getProblem().addLinearFVKernel(kernel_type,
                                     prefix() + "ins_momentum_hydrostatic_source_" + comp_axis[d],
                                     params);
    }
  }

}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addMomentumFaceBasedReducedPressureKernels()
{
  const std::string kernel_type = "LinearFVFaceBasedMomentumPressure";
  const std::string kernel_name = prefix() + "ins_momentum_reduced_pressure_";

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<VariableName>(NS::pressure) = _pressure_name;
  params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();

  for (const auto d : make_range(dimension()))
  {
    params.set<MooseEnum>("momentum_component") = NS::directions[d];
    params.set<LinearVariableName>("variable") = _velocity_names[d];
    getProblem().addLinearFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addMomentumFrictionKernels()
{
  unsigned int num_friction_blocks = _friction_blocks.size();
  unsigned int num_used_blocks = num_friction_blocks ? num_friction_blocks : 1;

  const std::string kernel_type = "LinearFVMomentumFriction";
  InputParameters params = getFactory().getValidParams(kernel_type);

  for (const auto block_i : make_range(num_used_blocks))
  {
    std::string block_name = "";
    if (num_friction_blocks)
    {
      params.set<std::vector<SubdomainName>>("block") = _friction_blocks[block_i];
      block_name = Moose::stringify(_friction_blocks[block_i]);
    }
    else
    {
      assignBlocks(params, _blocks);
      block_name = std::to_string(block_i);
    }

    for (const auto d : make_range(dimension()))
    {
      params.set<LinearVariableName>("variable") = _velocity_names[d];
      params.set<MooseEnum>("momentum_component") = NS::directions[d];
      for (unsigned int type_i = 0; type_i < _friction_types[block_i].size(); ++type_i)
      {
        const auto upper_name = MooseUtils::toUpper(_friction_types[block_i][type_i]);
        if (upper_name == "DARCY")
        {
          params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;
          params.set<MooseFunctorName>("Darcy_name") = _friction_coeffs[block_i][type_i];
        }
        else
          paramError("friction_types",
                     "Friction type '",
                     _friction_types[block_i][type_i],
                     "' is not implemented");
      }

      getProblem().addLinearFVKernel(kernel_type,
                                     prefix() + "momentum_friction_" + block_name + "_" +
                                         NS::directions[d],
                                     params);
    }
  }
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addMomentumBoussinesqKernels()
{
  if (_compressibility == "weakly-compressible")
    paramError("boussinesq_approximation",
               "We cannot use boussinesq approximation while running in weakly-compressible mode!");

  const std::string kernel_type = "LinearFVMomentumBoussinesq";
  const std::string kernel_name = prefix() + "ins_momentum_boussinesq_";

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<VariableName>(NS::T_fluid) = _fluid_temperature_name;
  params.set<MooseFunctorName>(NS::density) = _density_gravity_name;
  params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
  params.set<Real>("ref_temperature") = getParam<Real>("ref_temperature");
  params.set<MooseFunctorName>("alpha_name") = getParam<MooseFunctorName>("thermal_expansion");

  for (const auto d : make_range(dimension()))
  {
    params.set<MooseEnum>("momentum_component") = NS::directions[d];
    params.set<LinearVariableName>("variable") = _velocity_names[d];
    getProblem().addLinearFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addInletBC()
{
  unsigned int num_velocity_functor_inlets = 0;
  for (const auto & [bdy, momentum_inlet_type] : _momentum_inlet_types)
    if (momentum_inlet_type == "fixed-velocity" || momentum_inlet_type == "fixed-pressure")
      num_velocity_functor_inlets++;

  if (num_velocity_functor_inlets != _momentum_inlet_functors.size())
    paramError("momentum_inlet_functors",
               "Size (" + std::to_string(_momentum_inlet_functors.size()) +
                   ") is not the same as the number of entries in the momentum_inlet_types "
                   "subvector for fixed-velocities/pressures functors (size " +
                   std::to_string(num_velocity_functor_inlets) + ")");

  unsigned int velocity_pressure_counter = 0;
  for (const auto & [inlet_bdy, momentum_inlet_type] : _momentum_inlet_types)
  {
    if (momentum_inlet_type == "fixed-velocity")
    {
      const std::string bc_type = "LinearFVAdvectionDiffusionFunctorDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {inlet_bdy};
      if (_momentum_inlet_functors.size() < velocity_pressure_counter + 1)
        paramError("momentum_inlet_functors",
                   "More non-flux inlets than inlet functors (" +
                       std::to_string(_momentum_inlet_functors.size()) + ")");

      const auto momentum_functors = libmesh_map_find(_momentum_inlet_functors, inlet_bdy);
      if (momentum_functors.size() < dimension())
        paramError("momentum_inlet_functors",
                   "Subvector for boundary '" + inlet_bdy + "' (size " +
                       std::to_string(momentum_functors.size()) +
                       ") is not the same size as the number of dimensions of the physics (" +
                       std::to_string(dimension()) + ")");

      for (const auto d : make_range(dimension()))
      {
        params.set<LinearVariableName>("variable") = _velocity_names[d];
        params.set<MooseFunctorName>("functor") =
            generatedBoundaryMomentumFunctorName(inlet_bdy, d, "inlet");
        getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + inlet_bdy, params);
      }
      ++velocity_pressure_counter;

      if (getParam<bool>("pressure_two_term_bc_expansion"))
      {
        const std::string pressure_bc_type = "LinearFVExtrapolatedPressureBC";
        InputParameters pressure_params = getFactory().getValidParams(pressure_bc_type);
        pressure_params.set<std::vector<BoundaryName>>("boundary") = {inlet_bdy};
        pressure_params.set<LinearVariableName>("variable") = _pressure_name;
        pressure_params.set<bool>("use_two_term_expansion") = true;
        getProblem().addLinearFVBC(pressure_bc_type,
                                   _pressure_name + "_extrapolation_inlet_" +
                                       Moose::stringify(inlet_bdy),
                                   pressure_params);
      }
    }
    else if (momentum_inlet_type == "fixed-pressure")
    {
      const std::string bc_type = "LinearFVAdvectionDiffusionFunctorDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<LinearVariableName>("variable") = _pressure_name;
      if (_momentum_inlet_functors.size() < velocity_pressure_counter + 1)
        paramError("momentum_inlet_functors",
                   "More non-flux inlets than inlet functors (" +
                       std::to_string(_momentum_inlet_functors.size()) + ")");

      params.set<MooseFunctorName>("functor") =
          libmesh_map_find(_momentum_inlet_functors, inlet_bdy)[0];
      params.set<std::vector<BoundaryName>>("boundary") = {inlet_bdy};

      getProblem().addLinearFVBC(bc_type, _pressure_name + "_" + inlet_bdy, params);
      ++velocity_pressure_counter;
    }
    else
      mooseError("Unsupported inlet boundary condition type: ", momentum_inlet_type);
  }
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addOutletBC()
{
  const bool use_reduced_pressure_outlet_flux_bc =
      _solve_for_dynamic_pressure && _pressure_formulation == "reduced";
  bool has_fixed_pressure_outlet = false;
  for (const auto & outlet_pair : _momentum_outlet_types)
  {
    const auto & momentum_outlet_type = outlet_pair.second;
    if (momentum_outlet_type == "fixed-pressure" ||
        momentum_outlet_type == "fixed-pressure-zero-gradient")
    {
      has_fixed_pressure_outlet = true;
      break;
    }
  }

  if (use_reduced_pressure_outlet_flux_bc && has_fixed_pressure_outlet)
  {
    const auto * executioner = getMooseApp().getExecutioner();
    const bool executioner_pins_pressure =
        executioner && executioner->parameters().isParamValid("pin_pressure") &&
        executioner->getParam<bool>("pin_pressure");

    if (!executioner_pins_pressure)
      paramError("momentum_outlet_types",
                 "Sharp reduced-pressure fixed-pressure outlets now use the constrained "
                 "pressure-flux boundary path and therefore require Executioner/pin_pressure = "
                 "true to provide the pressure reference.");
  }

  unsigned int num_pressure_value_outlets = 0;
  for (const auto & [bdy, momentum_outlet_type] : _momentum_outlet_types)
    if (!use_reduced_pressure_outlet_flux_bc &&
        (momentum_outlet_type == "fixed-pressure" ||
         momentum_outlet_type == "fixed-pressure-zero-gradient"))
      num_pressure_value_outlets++;

  if (num_pressure_value_outlets && num_pressure_value_outlets != _pressure_functors.size())
    paramError("pressure_functors",
               "Size (" + std::to_string(_pressure_functors.size()) +
                   ") is not the same as the number of pressure outlet boundaries in "
                   "'fixed-pressure/fixed-pressure-zero-gradient' (size " +
                   std::to_string(num_pressure_value_outlets) + ")");

  for (const auto & [outlet_bdy, momentum_outlet_type] : _momentum_outlet_types)
  {
    if (momentum_outlet_type == "zero-gradient" || momentum_outlet_type == "fixed-pressure" ||
        momentum_outlet_type == "fixed-pressure-zero-gradient")
    {
      for (const auto d : make_range(dimension()))
      {
        const bool use_pressure_inlet_outlet_velocity =
            use_reduced_pressure_outlet_flux_bc &&
            (momentum_outlet_type == "fixed-pressure" ||
             momentum_outlet_type == "fixed-pressure-zero-gradient");

        const std::string bc_type = use_pressure_inlet_outlet_velocity
                                        ? "LinearFVPressureInletOutletMomentumBC"
                                        : "LinearFVAdvectionDiffusionOutflowBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};
        params.set<bool>("use_two_term_expansion") =
            getParam<bool>("momentum_two_term_bc_expansion");
        params.set<LinearVariableName>("variable") = _velocity_names[d];

        if (use_pressure_inlet_outlet_velocity)
        {
          params.set<SolverVariableName>("u") = _velocity_names[0];
          if (dimension() >= 2)
            params.set<SolverVariableName>("v") = _velocity_names[1];
          if (dimension() >= 3)
            params.set<SolverVariableName>("w") = _velocity_names[2];
          params.set<MooseEnum>("momentum_component") =
              MooseEnum("x=0 y=1 z=2", NS::directions[d]);

          params.set<MooseFunctorName>(NS::density) = _density_name;
          if (shouldCreateGeometryFunctorMaterial())
            params.set<MooseFunctorName>("density_gradient_functor") =
                generatedGeometryFunctorName("density_gradient");
          params.set<Real>("minimum_density") = getParam<Real>("geometry_minimum_density");
          params.set<MooseFunctorName>("backflow_value") = "0";
        }

        getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + outlet_bdy, params);
      }
    }

    if (momentum_outlet_type == "fixed-pressure" ||
        momentum_outlet_type == "fixed-pressure-zero-gradient")
    {
      if (use_reduced_pressure_outlet_flux_bc)
      {
        const std::string pressure_bc_type = "LinearFVPressureFluxBC";
        InputParameters pressure_params = getFactory().getValidParams(pressure_bc_type);
        pressure_params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};
        pressure_params.set<LinearVariableName>("variable") = _pressure_name;
        pressure_params.set<MooseFunctorName>("pressure_predictor_flux") =
            "pressure_predictor_flux";
        pressure_params.set<MooseFunctorName>("constrained_pressure_normal_gradient") =
            "pressure_boundary_normal_gradient";
        pressure_params.set<bool>("use_constrained_pressure_normal_gradient_only") = true;
        pressure_params.set<MooseFunctorName>("HbyA_flux") = "phiHbyA";
        pressure_params.set<MooseFunctorName>("Ainv") = "pressure_Ainv";
        getProblem().addLinearFVBC(
            pressure_bc_type, _pressure_name + "_outlet_flux_" + outlet_bdy, pressure_params);
      }
      else
      {
        const std::string bc_type = "LinearFVAdvectionDiffusionFunctorDirichletBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<LinearVariableName>("variable") = _pressure_name;
        params.set<MooseFunctorName>("functor") = libmesh_map_find(_pressure_functors, outlet_bdy);
        params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};
        getProblem().addLinearFVBC(bc_type, _pressure_name + "_" + outlet_bdy, params);
      }
    }
  }
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addWallsBC()
{
  const std::string u_names[3] = {"u", "v", "w"};
  for (const auto & [boundary_name, momentum_wall_type] : _momentum_wall_types)
  {
    if (momentum_wall_type == "noslip")
    {
      const std::string bc_type = "LinearFVAdvectionDiffusionFunctorDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};

      for (const auto d : make_range(dimension()))
      {
        params.set<LinearVariableName>("variable") = _velocity_names[d];
        if (_momentum_wall_functors.count(boundary_name) == 0)
          params.set<MooseFunctorName>("functor") = "0";
        else
          params.set<MooseFunctorName>("functor") =
              generatedBoundaryMomentumFunctorName(boundary_name, d, "wall");

        getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + boundary_name, params);
      }

      const std::string pressure_bc_type = "LinearFVPressureFluxBC";
      InputParameters pressure_params = getFactory().getValidParams(pressure_bc_type);
      pressure_params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};
      pressure_params.set<LinearVariableName>("variable") = _pressure_name;
      pressure_params.set<MooseFunctorName>("pressure_predictor_flux") =
          "pressure_predictor_flux";
      pressure_params.set<MooseFunctorName>("constrained_pressure_normal_gradient") =
          "pressure_boundary_normal_gradient";
      pressure_params.set<bool>("use_constrained_pressure_normal_gradient_only") = true;
      pressure_params.set<MooseFunctorName>("HbyA_flux") = "phiHbyA";
      pressure_params.set<MooseFunctorName>("Ainv") = "pressure_Ainv";
      getProblem().addLinearFVBC(
          pressure_bc_type, _pressure_name + "_wall_flux_" + boundary_name, pressure_params);
    }
    else if (momentum_wall_type == "symmetry")
    {
      {
        const std::string bc_type = "LinearFVVelocitySymmetryBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};
        for (unsigned int d = 0; d < dimension(); ++d)
          params.set<SolverVariableName>(u_names[d]) = _velocity_names[d];

        for (const auto d : make_range(dimension()))
        {
          params.set<LinearVariableName>("variable") = _velocity_names[d];
          params.set<MooseEnum>("momentum_component") = NS::directions[d];
          getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + boundary_name, params);
        }
      }
      {
        const std::string bc_type = "LinearFVPressureSymmetryBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};
        params.set<LinearVariableName>("variable") = _pressure_name;
        params.set<MooseFunctorName>("pressure_predictor_flux") =
            "pressure_predictor_flux";
        params.set<MooseFunctorName>("constrained_pressure_normal_gradient") =
            "pressure_boundary_normal_gradient";
        params.set<bool>("use_constrained_pressure_normal_gradient_only") = true;
        params.set<MooseFunctorName>("HbyA_flux") = "phiHbyA";
        params.set<MooseFunctorName>("Ainv") = "pressure_Ainv";
        getProblem().addLinearFVBC(bc_type, _pressure_name + "_" + boundary_name, params);
      }
    }
    else
      mooseError("Unsupported wall boundary condition type: " + std::string(momentum_wall_type));
  }

  if (getParam<bool>("pressure_two_term_bc_expansion"))
    paramWarning("pressure_two_term_bc_expansion",
                 "Ignoring pressure_two_term_bc_expansion on sharp-interface wall boundaries "
                 "because the reduced-pressure path now applies an explicit zero-normal-flux "
                 "pressure boundary condition.");
}

MooseFunctorName
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::generatedGeometryFunctorName(
    const std::string & base_name) const
{
  return prefix() + base_name;
}

MooseFunctorName
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::generatedConservativeVelocityFunctorName(
    unsigned int component) const
{
  return prefix() + "conservative_velocity_" + NS::directions[component];
}

MooseFunctorName
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::momentumTransportMassFluxFunctorName() const
{
  return "rho_phi_mass_flux_density";
}

MooseFunctorName
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::generatedBoundaryMomentumFunctorName(
    const BoundaryName & boundary, unsigned int component, const std::string & family) const
{
  return prefix() + family + "_momentum_" + sanitizeFunctorLabel(boundary) + "_" +
         NS::directions[component];
}

bool
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::useMomentumContinuityErrorSink() const
{
  return isParamSetByUser("add_momentum_continuity_error_sink") &&
         getParam<bool>("add_momentum_continuity_error_sink");
}

bool
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::shouldCreateGeometryFunctorMaterial() const
{
  return _create_geometry_functors &&
         !getParam<MooseFunctorName>("volume_fraction_functor").empty();
}

bool
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::shouldCreateCurvatureProducer() const
{
  return _create_curvature_producer &&
         !getParam<MooseFunctorName>("volume_fraction_functor").empty();
}

bool
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::shouldCreateDynamicContactAngleFunctorMaterial()
    const
{
  return _create_dynamic_contact_angle_functor_material &&
         shouldCreateCurvatureProducer() &&
         getParam<MooseFunctorName>("wall_contact_angle_degrees_functor").empty() &&
         containsDynamicContactAngleModel(getParam<std::vector<std::string>>("contact_angle_models"));
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addCurvatureUserObject()
{
  if (!shouldCreateCurvatureProducer())
    return;

  std::vector<UserObject *> objs;
  getProblem()
      .theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribThread>(0)
      .queryInto(objs);

  for (const auto & obj : objs)
    if (const auto * curvature = dynamic_cast<GeneralUserObject *>(obj))
      if (curvature->name() == prefix() + "sharp_interface_curvature")
        return;

  const std::string object_type = "ConservativeSharpInterfaceCurvatureCalculator";
  auto params = getFactory().getValidParams(object_type);
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>("volume_fraction_functor") =
      getParam<MooseFunctorName>("volume_fraction_functor");
  params.set<MooseEnum>("delta_n_mode") = getParam<MooseEnum>("curvature_delta_n_mode");
  params.set<Real>("delta_n_scale") = getParam<Real>("curvature_delta_n_scale");
  params.set<Real>("delta_n_fixed_value") = getParam<Real>("curvature_delta_n_fixed_value");
  params.set<bool>("use_openfoam_simple_curvature") =
      getParam<bool>("use_openfoam_simple_curvature");
  params.set<unsigned int>("n_alpha_smooth_curvature") =
      getParam<unsigned int>("n_alpha_smooth_curvature");
  params.set<std::vector<BoundaryName>>("contact_angle_boundaries") =
      getParam<std::vector<BoundaryName>>("contact_angle_boundaries");
  params.set<std::vector<Real>>("static_contact_angles_degrees") =
      getParam<std::vector<Real>>("static_contact_angles_degrees");
  params.set<Real>("contact_angle_small_det") = getParam<Real>("contact_angle_small_det");

  const auto explicit_wall_angle_functor =
      getParam<MooseFunctorName>("wall_contact_angle_degrees_functor");
  if (!explicit_wall_angle_functor.empty())
    params.set<MooseFunctorName>("wall_contact_angle_degrees_functor") =
        explicit_wall_angle_functor;
  else if (shouldCreateDynamicContactAngleFunctorMaterial())
    params.set<MooseFunctorName>("wall_contact_angle_degrees_functor") =
        generatedGeometryFunctorName("dynamic_wall_contact_angle_degrees");

  params.set<MooseFunctorName>("face_smoothed_alpha_gradient_name") =
      generatedGeometryFunctorName("curvature_face_smoothed_alpha_gradient");
  params.set<MooseFunctorName>("provisional_face_unit_normal_name") =
      generatedGeometryFunctorName("curvature_provisional_interface_unit_normal_face");
  params.set<MooseFunctorName>("face_unit_normal_name") =
      generatedGeometryFunctorName("curvature_interface_unit_normal_face");
  params.set<MooseFunctorName>("curvature_name") = generatedGeometryFunctorName("curvature");

  getProblem().addUserObject(object_type, prefix() + "sharp_interface_curvature", params);
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addRhieChowUserObjects()
{
  mooseAssert(dimension(), "0-dimension not supported");

  std::vector<UserObject *> objs;
  getProblem()
      .theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribThread>(0)
      .queryInto(objs);

  bool have_sharp_rc_uo = false;
  const auto this_block_ids = getSubdomainIDs(std::set<SubdomainName>(_blocks.begin(), _blocks.end()));
  for (const auto & obj : objs)
    if (dynamic_cast<RhieChowMassFlux *>(obj))
    {
      const auto rc_obj = dynamic_cast<RhieChowMassFlux *>(obj);
      const bool overlaps = blocksOverlap(rc_obj->blockIDs(), this_block_ids);
      if (!overlaps)
        continue;

      if (dynamic_cast<ConservativeSharpInterfaceRhieChowMassFlux *>(obj))
        have_sharp_rc_uo = true;
      else
        mooseError("Sharp-interface flow physics '",
                   name(),
                   "' requires a ConservativeSharpInterfaceRhieChowMassFlux on blocks ",
                   Moose::stringify(_blocks),
                   ", but found existing RhieChowMassFlux '",
                   rc_obj->name(),
                   "' on overlapping blocks. Remove the stock Rhie-Chow object or use the "
                   "sharp-interface flow physics as the owner of the segregated flow coupling.");
    }

  if (have_sharp_rc_uo)
    return;

  const std::string u_names[3] = {"u", "v", "w"};
  const auto object_type = "ConservativeSharpInterfaceRhieChowMassFlux";
  const bool use_face_based_reduced_pressure_predictor =
      _solve_for_dynamic_pressure && _pressure_formulation == "reduced" &&
      _add_capillary_hydrostatic_flux;

  auto params = getFactory().getValidParams(object_type);
  assignBlocks(params, _blocks);

  for (unsigned int d = 0; d < dimension(); ++d)
    params.set<VariableName>(u_names[d]) = _velocity_names[d];

  params.set<VariableName>("pressure") = _pressure_name;
  params.set<std::string>("p_diffusion_kernel") = prefix() + "p_diffusion";
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseEnum>("pressure_projection_method") =
      getParam<MooseEnum>("pressure_projection_method");
  params.set<bool>("use_cached_momentum_predictor_operator") =
      getParam<bool>("use_cached_momentum_predictor_operator");
  params.set<bool>("split_momentum_predictor_operator") = false;
  if (parameters().isParamValid("gravity"))
    params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
  if (_solve_for_dynamic_pressure)
    params.set<Point>("reference_pressure_point") = getParam<Point>("reference_pressure_point");
  params.set<bool>("add_transient_projection_flux") = _add_transient_projection_flux;
  params.set<bool>("add_capillary_hydrostatic_flux") = _add_capillary_hydrostatic_flux;
  params.set<bool>("use_face_based_reduced_pressure_predictor_contract") =
      use_face_based_reduced_pressure_predictor;
  params.set<bool>("apply_pressure_velocity_writeback") =
      getParam<bool>("apply_pressure_velocity_writeback");
  params.set<bool>("apply_pressure_face_flux_correction") =
      getParam<bool>("apply_pressure_face_flux_correction");
  params.set<MooseFunctorName>("vof_rho_phi_functor") =
      getParam<MooseFunctorName>("vof_rho_phi_functor");
  params.set<MooseFunctorName>("volume_fraction_functor") =
      getParam<MooseFunctorName>("volume_fraction_functor");
  params.set<Real>("near_interface_lower") = getParam<Real>("near_interface_lower");
  params.set<Real>("near_interface_upper") = getParam<Real>("near_interface_upper");
  params.set<MooseFunctorName>("vof_alpha_phi_limited_functor") =
      getParam<MooseFunctorName>("vof_alpha_phi_limited_functor");
  params.set<MooseFunctorName>("liquid_density_functor") =
      getParam<MooseFunctorName>("liquid_density_functor");
  params.set<MooseFunctorName>("gas_density_functor") =
      getParam<MooseFunctorName>("gas_density_functor");

  const bool use_cell_based_reduced_pressure_force_names = false;
  if (_solve_for_dynamic_pressure && _pressure_formulation == "reduced" &&
      _add_capillary_hydrostatic_flux && use_cell_based_reduced_pressure_force_names)
  {
    const std::vector<std::string> comp_axis({"x", "y", "z"});
    std::vector<std::vector<std::string>> body_force_kernel_names(dimension());
    const auto resolve_source_name = [this](const std::string & param_name,
                                            const std::string & generated_base_name)
    {
      const auto explicit_name = getParam<MooseFunctorName>(param_name);
      if (!explicit_name.empty())
        return explicit_name;

      return shouldCreateGeometryFunctorMaterial()
                 ? generatedGeometryFunctorName(generated_base_name)
                 : MooseFunctorName("");
    };

    for (const auto d : make_range(dimension()))
    {
      if (!resolve_source_name("surface_tension_momentum_source_" + comp_axis[d],
                               "surface_tension_momentum_source_" + comp_axis[d])
               .empty())
        body_force_kernel_names[d].push_back(prefix() + "ins_momentum_capillary_source_" +
                                             comp_axis[d]);

      if (!resolve_source_name("hydrostatic_momentum_source_" + comp_axis[d],
                               "hydrostatic_momentum_source_" + comp_axis[d])
               .empty())
        body_force_kernel_names[d].push_back(prefix() + "ins_momentum_hydrostatic_source_" +
                                             comp_axis[d]);
    }

    params.set<std::vector<std::vector<std::string>>>("body_force_kernel_names") =
        body_force_kernel_names;
  }

  const auto transient_name = getParam<MooseFunctorName>("transient_projection_face_acceleration");
  if (!transient_name.empty())
    params.set<MooseFunctorName>("transient_projection_face_acceleration") = transient_name;

  const auto surface_tension_name =
      getParam<MooseFunctorName>("surface_tension_face_acceleration");
  if (!surface_tension_name.empty())
    params.set<MooseFunctorName>("surface_tension_face_acceleration") = surface_tension_name;
  const auto surface_tension_cell_name =
      getParam<MooseFunctorName>("surface_tension_cell_acceleration");
  if (!surface_tension_cell_name.empty())
    params.set<MooseFunctorName>("surface_tension_cell_acceleration") = surface_tension_cell_name;
  if (shouldCreateGeometryFunctorMaterial())
  {
    const auto sigma_name = getParam<MooseFunctorName>("surface_tension_coefficient");
    const bool sigma_is_literal_zero =
        MooseUtils::parsesToReal(sigma_name) && std::abs(std::stod(sigma_name)) <= libMesh::TOLERANCE;
    if (!sigma_is_literal_zero)
    {
      if (surface_tension_name.empty())
        params.set<MooseFunctorName>("surface_tension_face_acceleration") =
            generatedGeometryFunctorName("surface_tension_face_acceleration");
      if (surface_tension_cell_name.empty())
        params.set<MooseFunctorName>("surface_tension_cell_acceleration") =
            generatedGeometryFunctorName("surface_tension_cell_acceleration");
    }
  }

  if (!use_face_based_reduced_pressure_predictor)
  {
    const auto hydrostatic_name =
        getParam<MooseFunctorName>("hydrostatic_density_gradient_face_acceleration");
    if (!hydrostatic_name.empty())
      params.set<MooseFunctorName>("hydrostatic_density_gradient_face_acceleration") =
          hydrostatic_name;
    const auto hydrostatic_cell_name =
        getParam<MooseFunctorName>("hydrostatic_density_gradient_cell_acceleration");
    if (!hydrostatic_cell_name.empty())
      params.set<MooseFunctorName>("hydrostatic_density_gradient_cell_acceleration") =
          hydrostatic_cell_name;
    if (shouldCreateGeometryFunctorMaterial())
    {
      if (hydrostatic_name.empty())
        params.set<MooseFunctorName>("hydrostatic_density_gradient_face_acceleration") =
            generatedGeometryFunctorName("hydrostatic_density_gradient_face_acceleration");
      if (hydrostatic_cell_name.empty())
        params.set<MooseFunctorName>("hydrostatic_density_gradient_cell_acceleration") =
            generatedGeometryFunctorName("hydrostatic_density_gradient_cell_acceleration");
    }
  }

  getProblem().addUserObject(object_type, rhieChowUOName(), params);
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addDynamicContactAngleFunctorMaterial()
{
  if (!shouldCreateDynamicContactAngleFunctorMaterial())
    return;

  const std::string class_name = "DynamicWallContactAngleFunctorMaterial";
  auto params = getFactory().getValidParams(class_name);
  assignBlocks(params, _blocks);

  params.set<MooseFunctorName>("provisional_interface_unit_normal_functor") =
      generatedGeometryFunctorName("curvature_provisional_interface_unit_normal_face");
  params.set<std::vector<BoundaryName>>("contact_angle_boundaries") =
      getParam<std::vector<BoundaryName>>("contact_angle_boundaries");
  params.set<std::vector<std::string>>("contact_angle_models") =
      getParam<std::vector<std::string>>("contact_angle_models");
  params.set<std::vector<Real>>("equilibrium_contact_angles_deg") =
      getParam<std::vector<Real>>("equilibrium_contact_angles_deg");
  params.set<std::vector<Real>>("advancing_contact_angles_deg") =
      getParam<std::vector<Real>>("advancing_contact_angles_deg");
  params.set<std::vector<Real>>("receding_contact_angles_deg") =
      getParam<std::vector<Real>>("receding_contact_angles_deg");
  params.set<std::vector<Real>>("contact_angle_velocity_scales") =
      getParam<std::vector<Real>>("contact_angle_velocity_scales");

  std::vector<MooseFunctorName> velocity_component_functors;
  for (const auto d : make_range(dimension()))
    velocity_component_functors.push_back(generatedConservativeVelocityFunctorName(d));
  params.set<std::vector<MooseFunctorName>>("velocity_component_functors") =
      velocity_component_functors;

  const auto wall_velocity_functor =
      getParam<MooseFunctorName>("dynamic_contact_angle_wall_velocity_functor");
  if (!wall_velocity_functor.empty())
    params.set<MooseFunctorName>("wall_velocity_functor") = wall_velocity_functor;

  params.set<RealVectorValue>("default_wall_velocity") =
      getParam<RealVectorValue>("dynamic_contact_angle_default_wall_velocity");
  params.set<Real>("parallel_direction_small") =
      getParam<Real>("dynamic_contact_angle_parallel_direction_small");
  params.set<Real>("u_theta_small") = getParam<Real>("dynamic_contact_angle_u_theta_small");
  params.set<MooseFunctorName>("wall_contact_angle_degrees_name") =
      generatedGeometryFunctorName("dynamic_wall_contact_angle_degrees");

  getProblem().addFunctorMaterial(class_name, prefix() + "dynamic_wall_contact_angle", params);
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addFunctorMaterials()
{
  if (parameters().isParamValid("gravity"))
  {
    const auto gravity_vector = getParam<RealVectorValue>("gravity");
    const std::vector<std::string> comp_axis({"x", "y", "z"});

    for (const auto d : make_range(dimension()))
      if (gravity_vector(d) != 0)
      {
        auto params = getFactory().getValidParams("ADParsedFunctorMaterial");
        assignBlocks(params, _blocks);
        params.set<bool>("enable_jit") = false;
        params.set<std::string>("expression") =
            _density_gravity_name + " * " + std::to_string(gravity_vector(d));
        if (!MooseUtils::parsesToReal(_density_gravity_name))
          params.set<std::vector<std::string>>("functor_names") = {_density_gravity_name};
        params.set<std::string>("property_name") = "rho_g_" + comp_axis[d];

        getProblem().addMaterial(
            "ADParsedFunctorMaterial", prefix() + "gravity_helper_" + comp_axis[d], params);
      }
  }

  addConservativeVelocityAdaptorFunctorMaterials();
  addConservativeBoundaryInputFunctorMaterials();
  addDynamicContactAngleFunctorMaterial();

  if (isTransient() && useMomentumContinuityErrorSink())
  {
    auto params = getFactory().getValidParams("ParsedFunctorMaterial");
    assignBlocks(params, _blocks);
    params.set<bool>("enable_jit") = false;
    params.set<std::string>("expression") =
        "conservative_continuity_error / (" + _density_name + " + " +
        std::to_string(getParam<Real>("geometry_minimum_density")) + ")";
    params.set<std::vector<std::string>>("functor_names") =
        {"conservative_continuity_error", _density_name};
    params.set<std::string>("property_name") = "conservative_continuity_error_over_density";
    getProblem().addMaterial(
        "ParsedFunctorMaterial", prefix() + "conservative_continuity_error_over_density", params);
  }

  if (!shouldCreateGeometryFunctorMaterial())
  {
    if (_create_geometry_functors)
      paramWarning("volume_fraction_functor",
                   "No 'volume_fraction_functor' was supplied, so the sharp-interface geometry "
                   "functor material will not be created automatically.");
    return;
  }

  const std::string class_name = "ConservativeSharpInterfaceGeometryFunctorMaterial";
  auto params = getFactory().getValidParams(class_name);
  assignBlocks(params, _blocks);

  params.set<MooseFunctorName>("volume_fraction_functor") =
      getParam<MooseFunctorName>("volume_fraction_functor");
  params.set<MooseFunctorName>("density_functor") = _density_name;
  params.set<MooseFunctorName>("liquid_density_functor") =
      getParam<MooseFunctorName>("liquid_density_functor");
  params.set<MooseFunctorName>("gas_density_functor") =
      getParam<MooseFunctorName>("gas_density_functor");
  params.set<MooseFunctorName>("surface_tension_coefficient") =
      getParam<MooseFunctorName>("surface_tension_coefficient");

  const auto curvature_name = getParam<MooseFunctorName>("curvature_functor");
  if (!curvature_name.empty())
    params.set<MooseFunctorName>("curvature_functor") = curvature_name;
  else if (shouldCreateCurvatureProducer())
    params.set<MooseFunctorName>("curvature_functor") = generatedGeometryFunctorName("curvature");

  if (shouldCreateCurvatureProducer())
  {
    params.set<MooseFunctorName>("face_smoothed_alpha_gradient_functor") =
        generatedGeometryFunctorName("curvature_face_smoothed_alpha_gradient");
    params.set<MooseFunctorName>("interface_unit_normal_functor") =
        generatedGeometryFunctorName("curvature_interface_unit_normal_face");
  }

  if (parameters().isParamValid("gravity"))
    params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");

  if (_solve_for_dynamic_pressure)
    params.set<Point>("reference_pressure_point") = getParam<Point>("reference_pressure_point");

  params.set<bool>("clip_volume_fraction_for_geometry") =
      getParam<bool>("clip_volume_fraction_for_geometry");
  params.set<Real>("alpha_lower_bound") = getParam<Real>("geometry_alpha_lower_bound");
  params.set<Real>("alpha_upper_bound") = getParam<Real>("geometry_alpha_upper_bound");
  params.set<Real>("near_interface_lower") = getParam<Real>("near_interface_lower");
  params.set<Real>("near_interface_upper") = getParam<Real>("near_interface_upper");
  params.set<Real>("minimum_density") = getParam<Real>("geometry_minimum_density");
  params.set<Real>("delta_n") = getParam<Real>("geometry_delta_n");

  params.set<MooseFunctorName>("delta_n_name") = generatedGeometryFunctorName("delta_n");
  params.set<MooseFunctorName>("near_interface_name") =
      generatedGeometryFunctorName("near_interface");
  params.set<MooseFunctorName>("alpha_gradient_name") =
      generatedGeometryFunctorName("alpha_gradient");
  params.set<MooseFunctorName>("face_smoothed_alpha_gradient_name") =
      generatedGeometryFunctorName("face_smoothed_alpha_gradient");
  params.set<MooseFunctorName>("density_gradient_name") =
      generatedGeometryFunctorName("density_gradient");
  params.set<MooseFunctorName>("interface_unit_normal_name") =
      generatedGeometryFunctorName("interface_unit_normal_face");
  params.set<MooseFunctorName>("sigma_k_name") = generatedGeometryFunctorName("sigma_k");
  params.set<MooseFunctorName>("reduced_pressure_head_name") =
      generatedGeometryFunctorName("reduced_pressure_head");
  params.set<MooseFunctorName>("surface_tension_face_acceleration_name") =
      generatedGeometryFunctorName("surface_tension_face_acceleration");
  params.set<MooseFunctorName>("surface_tension_cell_acceleration_name") =
      generatedGeometryFunctorName("surface_tension_cell_acceleration");
  params.set<MooseFunctorName>("surface_tension_momentum_source_x_name") =
      generatedGeometryFunctorName("surface_tension_momentum_source_x");
  params.set<MooseFunctorName>("surface_tension_momentum_source_y_name") =
      generatedGeometryFunctorName("surface_tension_momentum_source_y");
  params.set<MooseFunctorName>("surface_tension_momentum_source_z_name") =
      generatedGeometryFunctorName("surface_tension_momentum_source_z");
  params.set<MooseFunctorName>("hydrostatic_density_gradient_face_acceleration_name") =
      generatedGeometryFunctorName("hydrostatic_density_gradient_face_acceleration");
  params.set<MooseFunctorName>("hydrostatic_density_gradient_cell_acceleration_name") =
      generatedGeometryFunctorName("hydrostatic_density_gradient_cell_acceleration");
  params.set<MooseFunctorName>("hydrostatic_momentum_source_x_name") =
      generatedGeometryFunctorName("hydrostatic_momentum_source_x");
  params.set<MooseFunctorName>("hydrostatic_momentum_source_y_name") =
      generatedGeometryFunctorName("hydrostatic_momentum_source_y");
  params.set<MooseFunctorName>("hydrostatic_momentum_source_z_name") =
      generatedGeometryFunctorName("hydrostatic_momentum_source_z");

  getProblem().addFunctorMaterial(class_name, prefix() + "sharp_interface_geometry", params);
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addConservativeVelocityAdaptorFunctorMaterials()
{
  const auto minimum_density = getParam<Real>("geometry_minimum_density");
  for (const auto d : make_range(dimension()))
  {
    auto params = getFactory().getValidParams("ADParsedFunctorMaterial");
    assignBlocks(params, _blocks);
    params.set<bool>("enable_jit") = false;
    params.set<std::string>("expression") =
        _velocity_names[d] + " / (" + _density_name + " + " + std::to_string(minimum_density) +
        ")";
    params.set<std::vector<std::string>>("functor_names") = {_velocity_names[d], _density_name};
    params.set<std::string>("property_name") = generatedConservativeVelocityFunctorName(d);
    getProblem().addMaterial("ADParsedFunctorMaterial",
                             prefix() + "conservative_velocity_adapter_" + NS::directions[d],
                             params);
  }
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addConservativeBoundaryInputFunctorMaterials()
{
  auto add_momentum_functor = [this](const MooseFunctorName & source_functor,
                                     const MooseFunctorName & target_functor,
                                     const std::string & object_suffix)
  {
    auto params = getFactory().getValidParams("ADParsedFunctorMaterial");
    assignBlocks(params, _blocks);
    params.set<bool>("enable_jit") = false;
    params.set<std::string>("expression") = _density_name + " * (" + source_functor + ")";
    std::vector<std::string> functor_names;
    if (!MooseUtils::parsesToReal(_density_name))
      functor_names.push_back(_density_name);
    if (!MooseUtils::parsesToReal(source_functor))
      functor_names.push_back(source_functor);
    if (!functor_names.empty())
      params.set<std::vector<std::string>>("functor_names") = functor_names;
    params.set<std::string>("property_name") = target_functor;
    getProblem().addMaterial(
        "ADParsedFunctorMaterial", prefix() + object_suffix + "_" + target_functor, params);
  };

  for (const auto & [boundary, inlet_type] : _momentum_inlet_types)
    if (inlet_type == "fixed-velocity")
    {
      const auto & velocity_functors = libmesh_map_find(_momentum_inlet_functors, boundary);
      for (const auto d : make_range(dimension()))
        add_momentum_functor(velocity_functors[d],
                            generatedBoundaryMomentumFunctorName(boundary, d, "inlet"),
                            "inlet_momentum_bc");
    }

  for (const auto & [boundary, wall_type] : _momentum_wall_types)
    if (wall_type == "noslip" && _momentum_wall_functors.count(boundary))
    {
      const auto & velocity_functors = _momentum_wall_functors[boundary];
      for (const auto d : make_range(dimension()))
        add_momentum_functor(velocity_functors[d],
                            generatedBoundaryMomentumFunctorName(boundary, d, "wall"),
                            "wall_momentum_bc");
    }
}

unsigned short
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::getNumberAlgebraicGhostingLayersNeeded() const
{
  return 2;
}
