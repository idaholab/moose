//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSLinearFVConservativeSharpInterfaceVOFPhysics.h"

#include "NS.h"
#include "WCNSFVFlowPhysicsBase.h"

registerNavierStokesPhysicsBaseTasks("NavierStokesApp", WCNSLinearFVConservativeSharpInterfaceVOFPhysics);
registerMooseAction("NavierStokesApp", WCNSLinearFVConservativeSharpInterfaceVOFPhysics, "add_variables_physics");
registerMooseAction("NavierStokesApp", WCNSLinearFVConservativeSharpInterfaceVOFPhysics, "add_fv_ic");
registerMooseAction("NavierStokesApp", WCNSLinearFVConservativeSharpInterfaceVOFPhysics, "add_linear_fv_kernel");
registerMooseAction("NavierStokesApp", WCNSLinearFVConservativeSharpInterfaceVOFPhysics, "add_linear_fv_bc");
registerMooseAction("NavierStokesApp", WCNSLinearFVConservativeSharpInterfaceVOFPhysics, "add_material");
registerMooseAction("NavierStokesApp", WCNSLinearFVConservativeSharpInterfaceVOFPhysics, "add_user_object");

InputParameters
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::validParams()
{
  InputParameters params = NavierStokesPhysicsBase::validParams();
  params += WCNSFVCoupledAdvectionPhysicsHelper::validParams();
  params.addClassDescription(
      "Create a linear-FV sharp-interface volume-fraction transport equation with an explicit "
      "compression term and optional mixture-property functors.");

  params.set<std::vector<SolverSystemName>>("system_names") = {"alpha_system"};

  params.addParam<VariableName>(
      "volume_fraction_variable",
      "alpha",
      "Name of the transported volume-fraction variable.");
  params.addParam<FunctionName>("initial_volume_fraction",
                                "0",
                                "Initial condition for the transported volume fraction.");
  params.addParam<MooseFunctorName>(
      "complementary_volume_fraction_name",
      "1_minus_alpha",
      "Name of the complementary gas/second-phase volume-fraction functor.");

  params += Moose::FV::advectedInterpolationParameter();
  params.addParam<std::vector<MooseFunctorName>>(
      "volume_fraction_inlet_functors",
      {},
      "Fixed-value inlet functors for the transported volume fraction, ordered like the coupled "
      "flow inlet boundaries.");
  MooseEnum volume_fraction_outlet_type("outflow inlet-outlet", "outflow");
  params.addParam<MooseEnum>(
      "volume_fraction_outlet_type",
      volume_fraction_outlet_type,
      "Outlet treatment for the transported volume fraction. 'inlet-outlet' matches the "
      "interFoam atmosphere treatment more closely by imposing a backflow value on inflow and "
      "zero-gradient / extrapolation on outflow.");
  params.addParam<MooseFunctorName>(
      "volume_fraction_outlet_backflow_functor",
      "0",
      "Backflow value imposed by the inlet-outlet volume-fraction BC, typically 0 for air on "
      "an atmosphere patch.");
  params.addParam<bool>(
      "volume_fraction_two_term_bc_expansion",
      true,
      "Whether the outlet outflow BC should use a two-term expansion.");

  params.addParam<MooseFunctorName>(
      "compression_factor",
      "0",
      "Compression coefficient c_alpha for the explicit interface-compression flux.");
  params.addParam<MooseFunctorName>(
      "interface_normal_functor",
      "interface_unit_normal_face",
      "Face-oriented interface unit normal used by the explicit compression flux.");
  MooseEnum alpha_correction_scheme("venkatakrishnan vanLeer", "vanLeer");
  params.addParam<MooseEnum>(
      "alpha_correction_scheme",
      alpha_correction_scheme,
      "High-order correction used in the bounded MULES-style alpha update. The donor/base flux "
      "remains upwind; this selects the higher-order correction flux added on top of it.");
  params.addParam<MooseFunctorName>("alpha_phi_bd_functor_name",
                                    "alpha_phi_bd",
                                    "Published donor/base alpha face flux.");
  params.addParam<MooseFunctorName>("alpha_phi_ho_functor_name",
                                    "alpha_phi_ho",
                                    "Published high-order advective alpha face flux.");
  params.addParam<MooseFunctorName>("alpha_phi_comp_functor_name",
                                    "alpha_phi_comp",
                                    "Published explicit compressive alpha face flux.");
  params.addParam<MooseFunctorName>("alpha_phi_corr_raw_functor_name",
                                    "alpha_phi_corr_raw",
                                    "Published raw correction alpha face flux prior to limiting.");
  params.addParam<MooseFunctorName>("alpha_phi_corr_functor_name",
                                    "alpha_phi_corr",
                                    "Published limited correction alpha face flux.");
  params.addParam<MooseFunctorName>("alpha_phi_limited_functor_name",
                                    "alpha_phi_limited",
                                    "Published limited alpha face flux.");
  params.addParam<MooseFunctorName>("rho_phi_functor_name",
                                    "rho_phi",
                                    "Published density-weighted face flux accumulated over alpha "
                                    "subcycles.");
  params.addParam<bool>(
      "use_mules_correction",
      true,
      "Whether to use an interFoam-style bounded explicit correction stage after the donor alpha "
      "solve. When enabled, the matrix transport should remain on upwind/donor transport.");
  params.addRangeCheckedParam<unsigned int>(
      "n_alpha_corrections", 2, "n_alpha_corrections>0", "Number of MULES-style correction sweeps.");
  params.addRangeCheckedParam<unsigned int>(
      "n_limiter_iterations",
      5,
      "n_limiter_iterations>0",
      "Number of limiter tightening passes inside each correction sweep.");
  params.addRangeCheckedParam<Real>(
      "mules_correction_relaxation",
      1.0,
      "mules_correction_relaxation>0 & mules_correction_relaxation<=1",
      "Under-relaxation factor applied to the bounded explicit correction.");
  params.addRangeCheckedParam<Real>(
      "later_alpha_correction_relaxation",
      0.5,
      "later_alpha_correction_relaxation>0 & later_alpha_correction_relaxation<=1",
      "Additional damping applied to alpha correctors after the first one, matching "
      "interFoam's later-corrector blending more closely.");
  params.addParam<bool>(
      "alpha_apply_prev_corr",
      true,
      "Whether to reuse the previous limited correction flux as the initial bounded-correction "
      "guess on the next alpha solve, matching interFoam's alphaApplyPrevCorr lifecycle.");
  params.addParam<bool>(
      "use_cell_summed_mules_limiter",
      false,
      "Forwarded to ConservativeSharpInterfaceVOFMULESCorrector to use a classic per-cell summed MULES "
      "limiter instead of sequential face-budget depletion.");
  params.addParam<bool>("debug_dump_subcycle",
                        false,
                        "Forwarded to ConservativeSharpInterfaceVOFMULESCorrector for targeted interface-face "
                        "debug dumps.");
  params.addParam<bool>("debug_only_first_subcycle",
                        true,
                        "Only dump the first alpha subcycle when debug dumping is enabled.");
  params.addRangeCheckedParam<unsigned int>(
      "debug_dump_max_faces", 12, "debug_dump_max_faces>0", "Maximum number of interface faces dumped.");
  params.addParam<std::vector<unsigned int>>(
      "debug_face_ids",
      {},
      "Optional list of face ids to include in ConservativeSharpInterfaceVOFMULESCorrector debug dumps. If "
      "empty, the usual interface-face filter is used.");
  params.addRangeCheckedParam<Real>(
      "debug_interface_alpha_tolerance",
      1e-10,
      "debug_interface_alpha_tolerance>=0",
      "Minimum alpha jump across a face before it is considered an interface face in debug dumps.");

  params.addParam<bool>(
      "create_complementary_fraction",
      true,
      "Whether to automatically define a complementary phase fraction functor 1 - alpha.");
  params.addParam<bool>(
      "create_mixture_materials",
      true,
      "Whether to automatically define rho(alpha) and mu(alpha) using the stock linear-FV "
      "mixture functor material.");
  params.addParam<MooseFunctorName>("mixture_density_name",
                                    "rho_mixture",
                                    "Name of the generated mixture density functor.");
  params.addParam<MooseFunctorName>("mixture_dynamic_viscosity_name",
                                    "mu_mixture",
                                    "Name of the generated mixture dynamic viscosity functor.");
  params.addRequiredParam<MooseFunctorName>("liquid_density_name", "Liquid-phase density functor.");
  params.addRequiredParam<MooseFunctorName>("gas_density_name", "Gas-phase density functor.");
  params.addRequiredParam<MooseFunctorName>("liquid_dynamic_viscosity_name",
                                            "Liquid-phase dynamic viscosity functor.");
  params.addRequiredParam<MooseFunctorName>("gas_dynamic_viscosity_name",
                                            "Gas-phase dynamic viscosity functor.");

  params.addParamNamesToGroup(
      "system_names advected_interp_method compression_factor interface_normal_functor "
      "alpha_correction_scheme",
      "Numerical scheme");

  params.suppressParameter<MooseEnum>("preconditioning");
  return params;
}

WCNSLinearFVConservativeSharpInterfaceVOFPhysics::WCNSLinearFVConservativeSharpInterfaceVOFPhysics(
    const InputParameters & parameters)
  : NavierStokesPhysicsBase(parameters),
    WCNSFVCoupledAdvectionPhysicsHelper(this),
    _alpha_name(getParam<VariableName>("volume_fraction_variable")),
    _gas_fraction_name(getParam<MooseFunctorName>("complementary_volume_fraction_name")),
    _alpha_inlet_functors(getParam<std::vector<MooseFunctorName>>("volume_fraction_inlet_functors")),
    _alpha_outlet_type(getParam<MooseEnum>("volume_fraction_outlet_type")),
    _alpha_outlet_backflow_functor(
        getParam<MooseFunctorName>("volume_fraction_outlet_backflow_functor")),
    _alpha_two_term_bc_expansion(getParam<bool>("volume_fraction_two_term_bc_expansion")),
    _compression_factor_name(getParam<MooseFunctorName>("compression_factor")),
    _interface_normal_functor_name(getParam<MooseFunctorName>("interface_normal_functor")),
    _alpha_correction_scheme(getParam<MooseEnum>("alpha_correction_scheme")),
    _use_mules_correction(getParam<bool>("use_mules_correction")),
    _create_complementary_fraction(getParam<bool>("create_complementary_fraction")),
    _create_mixture_materials(getParam<bool>("create_mixture_materials")),
    _mixture_density_name(getParam<MooseFunctorName>("mixture_density_name")),
    _mixture_dynamic_viscosity_name(getParam<MooseFunctorName>("mixture_dynamic_viscosity_name")),
    _liquid_density_name(getParam<MooseFunctorName>("liquid_density_name")),
    _gas_density_name(getParam<MooseFunctorName>("gas_density_name")),
    _liquid_dynamic_viscosity_name(getParam<MooseFunctorName>("liquid_dynamic_viscosity_name")),
    _gas_dynamic_viscosity_name(getParam<MooseFunctorName>("gas_dynamic_viscosity_name"))
{
  if (_alpha_name == _gas_fraction_name)
    paramError("complementary_volume_fraction_name",
               "The complementary volume-fraction name must differ from the transported "
               "volume-fraction variable name.");

  if (_use_mules_correction && getParam<MooseEnum>("advected_interp_method") != "upwind")
    paramError("advected_interp_method",
               "When use_mules_correction=true the matrix transport must remain on donor/upwind "
               "transport, matching the bounded-base-plus-limited-correction structure used by "
               "interFoam.");
}

void
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::addSolverVariables()
{
  if (!shouldCreateVariable(_alpha_name, _blocks, /*error if aux*/ true))
    reportPotentiallyMissedParameters({"system_names"}, "MooseLinearVariableFVReal");
  else if (_define_variables)
  {
    const std::string variable_type = "MooseLinearVariableFVReal";
    auto params = getFactory().getValidParams(variable_type);
    assignBlocks(params, _blocks);
    params.set<SolverSystemName>("solver_sys") = getSolverSystem(_alpha_name);
    getProblem().addVariable(variable_type, _alpha_name, params);
  }
  else
    paramError("volume_fraction_variable",
               "Variable (" + _alpha_name +
                   ") supplied to the WCNSLinearFVConservativeSharpInterfaceVOFPhysics does not exist!");

  saveSolverVariableName(_alpha_name);
}

void
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::addFVKernels()
{
  if (isTransient())
    addAlphaTimeKernels();

  addAlphaAdvectionKernels();
  if (!_use_mules_correction)
    addAlphaCompressionKernels();
}

void
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::addInitialConditions()
{
  if (!_define_variables && parameters().isParamSetByUser("initial_volume_fraction"))
    paramError("initial_volume_fraction",
               "Volume-fraction variable is defined externally of "
               "WCNSLinearFVConservativeSharpInterfaceVOFPhysics, so should its initial condition.");

  if (getParam<bool>("initialize_variables_from_mesh_file"))
    return;

  if (!shouldCreateIC(_alpha_name,
                      _blocks,
                      /*whether IC is a default*/ !isParamSetByUser("initial_volume_fraction"),
                      /*error if already an IC*/ isParamSetByUser("initial_volume_fraction")))
    return;

  auto params = getFactory().getValidParams("FVFunctionIC");
  assignBlocks(params, _blocks);
  params.set<VariableName>("variable") = _alpha_name;
  params.set<FunctionName>("function") = getParam<FunctionName>("initial_volume_fraction");
  getProblem().addFVInitialCondition("FVFunctionIC", prefix() + _alpha_name + "_ic", params);
}

void
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::addFVBCs()
{
  addAlphaInletBC();
  addAlphaOutletBC();
}

void
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::addMaterials()
{
  if (_create_complementary_fraction && !getProblem().hasFunctor(_gas_fraction_name, /*tid=*/0))
  {
    auto params = getFactory().getValidParams("ParsedFunctorMaterial");
    assignBlocks(params, _blocks);
    params.set<std::string>("expression") = "1 - " + _alpha_name;
    params.set<std::vector<std::string>>("functor_names") = {_alpha_name};
    params.set<std::string>("property_name") = _gas_fraction_name;
    params.set<std::vector<std::string>>("output_properties") = {_gas_fraction_name};
    params.set<std::vector<OutputName>>("outputs") = {"all"};
    getProblem().addMaterial("ParsedFunctorMaterial", prefix() + "complementary_fraction", params);
  }

  if (_create_mixture_materials)
  {
    auto params = getFactory().getValidParams("WCNSLinearFVMixtureFunctorMaterial");
    assignBlocks(params, _blocks);
    params.set<std::vector<MooseFunctorName>>("prop_names") = {_mixture_density_name,
                                                               _mixture_dynamic_viscosity_name};
    params.set<std::vector<MooseFunctorName>>("phase_1_names") = {_liquid_density_name,
                                                                  _liquid_dynamic_viscosity_name};
    params.set<std::vector<MooseFunctorName>>("phase_2_names") = {_gas_density_name,
                                                                  _gas_dynamic_viscosity_name};
    params.set<MooseFunctorName>("phase_1_fraction") = _alpha_name;
    params.set<bool>("limit_phase_fraction") = true;
    getProblem().addMaterial(
        "WCNSLinearFVMixtureFunctorMaterial", prefix() + "mixture_properties", params);
  }
}

void
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::addUserObjects()
{
  if (!_use_mules_correction)
    return;

  const std::string object_type = "ConservativeSharpInterfaceVOFMULESCorrector";
  const std::string object_name = prefix() + "vof_mules";
  auto params = getFactory().getValidParams(object_type);
  assignBlocks(params, _blocks);
  params.set<SolverSystemName>("system_name") = getSolverSystem(_alpha_name);
  params.set<VariableName>("variable") = _alpha_name;
  params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
  params.set<MooseFunctorName>("compression_factor") = _compression_factor_name;
  params.set<MooseFunctorName>("interface_normal") = _interface_normal_functor_name;
  params.set<MooseEnum>("high_order_correction_scheme") = _alpha_correction_scheme;
  params.set<unsigned int>("n_alpha_corrections") = getParam<unsigned int>("n_alpha_corrections");
  params.set<unsigned int>("n_limiter_iterations") =
      getParam<unsigned int>("n_limiter_iterations");
  params.set<Real>("correction_relaxation") = getParam<Real>("mules_correction_relaxation");
  params.set<Real>("later_correction_relaxation") =
      getParam<Real>("later_alpha_correction_relaxation");
  params.set<bool>("alpha_apply_prev_corr") = getParam<bool>("alpha_apply_prev_corr");
  params.set<bool>("use_cell_summed_mules_limiter") =
      getParam<bool>("use_cell_summed_mules_limiter");
  params.set<MooseFunctorName>("liquid_density") = _liquid_density_name;
  params.set<MooseFunctorName>("gas_density") = _gas_density_name;
  params.set<MooseFunctorName>("alpha_phi_bd_functor_name") =
      getParam<MooseFunctorName>("alpha_phi_bd_functor_name");
  params.set<MooseFunctorName>("alpha_phi_ho_functor_name") =
      getParam<MooseFunctorName>("alpha_phi_ho_functor_name");
  params.set<MooseFunctorName>("alpha_phi_comp_functor_name") =
      getParam<MooseFunctorName>("alpha_phi_comp_functor_name");
  params.set<MooseFunctorName>("alpha_phi_corr_raw_functor_name") =
      getParam<MooseFunctorName>("alpha_phi_corr_raw_functor_name");
  params.set<MooseFunctorName>("alpha_phi_corr_functor_name") =
      getParam<MooseFunctorName>("alpha_phi_corr_functor_name");
  params.set<MooseFunctorName>("alpha_phi_limited_functor_name") =
      getParam<MooseFunctorName>("alpha_phi_limited_functor_name");
  params.set<MooseFunctorName>("rho_phi_functor_name") =
      getParam<MooseFunctorName>("rho_phi_functor_name");
  params.set<bool>("debug_dump_subcycle") = getParam<bool>("debug_dump_subcycle");
  params.set<bool>("debug_only_first_subcycle") = getParam<bool>("debug_only_first_subcycle");
  params.set<unsigned int>("debug_dump_max_faces") = getParam<unsigned int>("debug_dump_max_faces");
  params.set<std::vector<unsigned int>>("debug_face_ids") =
      getParam<std::vector<unsigned int>>("debug_face_ids");
  params.set<Real>("debug_interface_alpha_tolerance") =
      getParam<Real>("debug_interface_alpha_tolerance");
  getProblem().addUserObject(object_type, object_name, params);
}

void
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::addAlphaTimeKernels()
{
  if (!shouldCreateTimeDerivative(_alpha_name, _blocks, /*error_if_already_defined=*/false))
    return;

  auto params = getFactory().getValidParams("LinearFVTimeDerivative");
  assignBlocks(params, _blocks);
  params.set<LinearVariableName>("variable") = _alpha_name;
  getProblem().addLinearFVKernel("LinearFVTimeDerivative", prefix() + "alpha_time", params);
}

void
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::addAlphaAdvectionKernels()
{
  auto params = getFactory().getValidParams("LinearFVScalarAdvection");
  assignBlocks(params, _blocks);
  params.set<LinearVariableName>("variable") = _alpha_name;
  params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
  params.set<MooseEnum>("advected_interp_method") = getParam<MooseEnum>("advected_interp_method");
  getProblem().addLinearFVKernel("LinearFVScalarAdvection", prefix() + "alpha_advection", params);
}

void
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::addAlphaCompressionKernels()
{
  auto params = getFactory().getValidParams("LinearFVVOFCompression");
  assignBlocks(params, _blocks);
  params.set<LinearVariableName>("variable") = _alpha_name;
  params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
  params.set<MooseFunctorName>("compression_factor") = _compression_factor_name;
  params.set<MooseFunctorName>("interface_normal") = _interface_normal_functor_name;
  getProblem().addLinearFVKernel("LinearFVVOFCompression", prefix() + "alpha_compression", params);
}

void
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::addAlphaInletBC()
{
  const auto & inlet_boundaries = _flow_equations_physics->getInletBoundaries();
  if (inlet_boundaries.empty() || _alpha_inlet_functors.empty())
    return;

  if (_alpha_inlet_functors.size() != inlet_boundaries.size())
    paramError("volume_fraction_inlet_functors",
               "The number of inlet functors (" + std::to_string(_alpha_inlet_functors.size()) +
                   ") must match the number of inlet boundaries (" +
                   std::to_string(inlet_boundaries.size()) + ").");

  for (const auto i : index_range(inlet_boundaries))
  {
    auto params = getFactory().getValidParams("LinearFVAdvectionDiffusionFunctorDirichletBC");
    params.set<LinearVariableName>("variable") = _alpha_name;
    params.set<MooseFunctorName>("functor") = _alpha_inlet_functors[i];
    params.set<std::vector<BoundaryName>>("boundary") = {inlet_boundaries[i]};
    getProblem().addLinearFVBC("LinearFVAdvectionDiffusionFunctorDirichletBC",
                               prefix() + "alpha_inlet_" + inlet_boundaries[i],
                               params);
  }
}

void
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::addAlphaOutletBC()
{
  const auto & outlet_boundaries = _flow_equations_physics->getOutletBoundaries();
  if (outlet_boundaries.empty())
    return;

  for (const auto & outlet_bdy : outlet_boundaries)
  {
    const std::string bc_type = _alpha_outlet_type == "inlet-outlet"
                                    ? "LinearFVInletOutletScalarBC"
                                    : "LinearFVAdvectionDiffusionOutflowBC";
    auto params = getFactory().getValidParams(bc_type);
    params.set<LinearVariableName>("variable") = _alpha_name;
    params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};
    params.set<bool>("use_two_term_expansion") = _alpha_two_term_bc_expansion;
    if (_alpha_outlet_type == "inlet-outlet")
    {
      const auto & velocity_names = _flow_equations_physics->getVelocityNames();
      params.set<SolverVariableName>("u") = velocity_names[0];
      if (dimension() >= 2)
        params.set<SolverVariableName>("v") = velocity_names[1];
      if (dimension() >= 3)
        params.set<SolverVariableName>("w") = velocity_names[2];
      params.set<MooseFunctorName>("backflow_value") = _alpha_outlet_backflow_functor;
    }
    getProblem().addLinearFVBC(bc_type, prefix() + "alpha_outlet_" + outlet_bdy, params);
  }
}
