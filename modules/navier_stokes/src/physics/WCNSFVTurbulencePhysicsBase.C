//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVTurbulencePhysicsBase.h"
#include "WCNSFVFlowPhysicsBase.h"
#include "WCNSFVFluidHeatTransferPhysicsBase.h"
#include "WCNSFVScalarTransportPhysicsBase.h"
#include "WCNSFVCoupledAdvectionPhysicsHelper.h"
#include "WCNSLinearFVTurbulencePhysics.h"
#include "kEpsilonViscosityAux.h"
#include "INSFVTKESourceSink.h"
#include "INSFVTurbulentViscosityWallFunction.h"
#include "NSFVBase.h"

InputParameters
WCNSFVTurbulencePhysicsBase::validParams()
{
  InputParameters params = NavierStokesPhysicsBase::validParams();
  params += WCNSFVCoupledAdvectionPhysicsHelper::validParams();
  params.addClassDescription(
      "Define a turbulence model for a incompressible or weakly-compressible Navier Stokes "
      "flow with a finite volume discretization");

  MooseEnum turbulence_type("mixing-length k-epsilon none", "none");
  params.addParam<MooseEnum>(
      "turbulence_handling",
      turbulence_type,
      "The way turbulent diffusivities are determined in the turbulent regime.");
  params += NSFVBase::commonTurbulenceParams();

  params.deprecateParam("mixing_length_walls", "turbulence_walls", "");

  // Not implemented, re-enable with k-epsilon
  params.suppressParameter<MooseEnum>("preconditioning");

  // K-Epsilon parameters
  params.addParam<MooseFunctorName>(
      "tke_name", NS::TKE, "Name of the turbulent kinetic energy variable");
  params.addParam<MooseFunctorName>(
      "tked_name", NS::TKED, "Name of the turbulent kinetic energy dissipation variable");
  params.addParam<FunctionName>(
      "initial_tke", "0", "Initial value for the turbulence kinetic energy");
  params.addParam<FunctionName>(
      "initial_tked", "0", "Initial value for the turbulence kinetic energy dissipation");
  params.addParam<FunctionName>("initial_mu_t", "Initial value for the turbulence viscosity");

  params.addParam<Real>("C1_eps",
                        "C1 coefficient for the turbulent kinetic energy dissipation equation");
  params.addParam<Real>("C2_eps",
                        "C2 coefficient for the turbulent kinetic energy dissipation equation");
  params.addParam<MooseFunctorName>(
      "sigma_k", "Scaling coefficient for the turbulent kinetic energy diffusion term");
  params.addParam<MooseFunctorName>(
      "sigma_eps",
      "Scaling coefficient for the turbulent kinetic energy dissipation diffusion term");
  params.addParam<MooseFunctorName>(
      NS::turbulent_Prandtl, NS::turbulent_Prandtl, "Turbulent Prandtl number");
  params.transferParam<Real>(INSFVTKESourceSink::validParams(), "C_pl");
  params.transferParam<Real>(kEpsilonViscosityAux::validParams(), "mu_t_ratio_max");

  // Boundary parameters
  params.addParam<bool>("bulk_wall_treatment", true, "Whether to treat the wall cell as bulk");
  MooseEnum wall_treatment("eq_newton eq_incremental eq_linearized neq", "neq");
  params.addParam<MooseEnum>("wall_treatment_eps",
                             wall_treatment,
                             "The method used for computing the epsilon wall functions and the "
                             "turbulence viscosity wall functions");
  params.addParam<MooseEnum>("wall_treatment_T",
                             wall_treatment,
                             "The method used for computing the temperature wall functions");
  params.transferParam<Real>(INSFVTurbulentViscosityWallFunction::validParams(), "C_mu");

  // K-Epsilon numerical scheme parameters
  MooseEnum face_interpol_types("average skewness-corrected", "average");
  MooseEnum adv_interpol_types("average upwind", "upwind");
  params.addParam<MooseEnum>("tke_face_interpolation",
                             face_interpol_types,
                             "The numerical scheme to interpolate the TKE to the "
                             "face (separate from the advected quantity interpolation).");
  params.addParam<MooseEnum>("tke_advection_interpolation",
                             adv_interpol_types,
                             "The numerical scheme to interpolate the TKE to the "
                             "face when in the advection kernel.");
  params.addParam<bool>(
      "tke_two_term_bc_expansion",
      false,
      "If a two-term Taylor expansion is needed for the determination of the boundary values"
      "of the turbulent kinetic energy.");

  params.addParam<MooseEnum>("tked_face_interpolation",
                             face_interpol_types,
                             "The numerical scheme to interpolate the TKED to the "
                             "face (separate from the advected quantity interpolation).");
  params.addParam<MooseEnum>("tked_advection_interpolation",
                             adv_interpol_types,
                             "The numerical scheme to interpolate the TKED to the "
                             "face when in the advection kernel.");
  params.addParam<bool>(
      "tked_two_term_bc_expansion",
      false,
      "If a two-term Taylor expansion is needed for the determination of the boundary values"
      "of the turbulent kinetic energy dissipation.");
  params.addParam<bool>(
      "turbulent_viscosity_two_term_bc_expansion",
      true,
      "If a two-term Taylor expansion is needed for the determination of the boundary values"
      "of the turbulent viscosity.");
  params.addParam<bool>("mu_t_as_aux_variable",
                        false,
                        "Whether to use an auxiliary variable instead of a functor material "
                        "property for the turbulent viscosity");
  params.addParam<bool>("output_mu_t", true, "Whether to add mu_t to the field outputs");
  params.addParam<bool>("k_t_as_aux_variable",
                        false,
                        "Whether to use an auxiliary variable for the turbulent conductivity");

  // Add the coupled physics
  // TODO Remove the defaults once NavierStokesFV action is removed
  // It is a little risky right now because the user could forget to pass the parameter and
  // be missing the influence of turbulence on either of these physics. There is a check in the
  // constructor to present this from happening
  params.addParam<PhysicsName>(
      "fluid_heat_transfer_physics",
      "NavierStokesFV",
      "WCNS(Linear)FVFluidHeatTransferPhysics generating the heat advection equations");
  params.addParam<PhysicsName>(
      "scalar_transport_physics",
      "NavierStokesFV",
      "WCNS(Linear)FVScalarTransportPhysics generating the scalar advection equations");

  // Parameter groups
  params.addParamNamesToGroup("fluid_heat_transfer_physics turbulent_prandtl "
                              "scalar_transport_physics Sc_t",
                              "Coupled Physics");
  params.addParamNamesToGroup("initial_tke initial_tked C1_eps C2_eps sigma_k sigma_eps",
                              "K-Epsilon model");
  params.addParamNamesToGroup("C_mu bulk_wall_treatment wall_treatment_eps wall_treatment_T",
                              "K-Epsilon wall function");
  params.addParamNamesToGroup("tke_face_interpolation tke_two_term_bc_expansion "
                              "tked_face_interpolation tked_two_term_bc_expansion "
                              "turbulent_viscosity_two_term_bc_expansion "
                              "mu_t_as_aux_variable k_t_as_aux_variable",
                              "K-Epsilon model numerical");

  return params;
}

WCNSFVTurbulencePhysicsBase::WCNSFVTurbulencePhysicsBase(const InputParameters & parameters)
  : NavierStokesPhysicsBase(parameters),
    WCNSFVCoupledAdvectionPhysicsHelper(this),
    _turbulence_model(getParam<MooseEnum>("turbulence_handling")),
    _turbulence_walls(getParam<std::vector<BoundaryName>>("turbulence_walls")),
    _wall_treatment_eps(getParam<MooseEnum>("wall_treatment_eps")),
    _wall_treatment_temp(getParam<MooseEnum>("wall_treatment_T")),
    _tke_name(getParam<MooseFunctorName>("tke_name")),
    _tked_name(getParam<MooseFunctorName>("tked_name"))
{
  if (_verbose && _turbulence_model != "none")
    _console << "Creating a " << std::string(_turbulence_model) << " turbulence model."
             << std::endl;

  // Keep track of the variable names, for loading variables from files notably
  if (_turbulence_model == "k-epsilon")
  {
    saveSolverVariableName(_tke_name);
    saveSolverVariableName(_tked_name);
    if (getParam<bool>("mu_t_as_aux_variable"))
      saveAuxVariableName(_turbulent_viscosity_name);
    if (getParam<bool>("k_t_as_aux_variable"))
      saveAuxVariableName(NS::k_t);
  }

  // Parameter checks
  if (_turbulence_model == "none")
    errorInconsistentDependentParameter("turbulence_handling", "none", {"turbulence_walls"});
  if (_turbulence_model != "k-epsilon")
  {
    errorDependentParameter("turbulence_handling",
                            "k-epsilon",
                            {"C_mu",
                             "C1_eps",
                             "C2_eps",
                             "bulk_wall_treatment",
                             "tke_scaling",
                             "tke_face_interpolation",
                             "tke_two_term_bc_expansion",
                             "tked_scaling",
                             "tked_face_interpolation",
                             "tked_two_term_bc_expansion",
                             "turbulent_viscosity_two_term_bc_expansion"});
    checkSecondParamSetOnlyIfFirstOneTrue("mu_t_as_aux_variable", "initial_mu_t");
  }
}

void
WCNSFVTurbulencePhysicsBase::actOnAdditionalTasks()
{
  // Other Physics may not exist or be initialized at construction time, so
  // we retrieve them now, on this task which occurs after 'init_physics'
  if (_current_task == "get_turbulence_physics")
    retrieveCoupledPhysics();
}

void
WCNSFVTurbulencePhysicsBase::retrieveCoupledPhysics()
{
  // _flow_equations_physics is initialized by 'WCNSFVCoupledAdvectionPhysicsHelper'
  if (_flow_equations_physics && _flow_equations_physics->hasFlowEquations())
    _has_flow_equations = true;
  else
    _has_flow_equations = false;

  // Sanity check for interaction for fluid heat transfer physics
  if (isParamValid("fluid_heat_transfer_physics") && _turbulence_model != "none")
  {
    _fluid_energy_physics = getCoupledPhysics<WCNSFVFluidHeatTransferPhysicsBase>(
        getParam<PhysicsName>("fluid_heat_transfer_physics"), true);
    // Check for a missing parameter / do not support isolated physics for now
    if (!_fluid_energy_physics &&
        !getCoupledPhysics<const WCNSFVFluidHeatTransferPhysicsBase>(true).empty())
      paramError("fluid_heat_transfer_physics",
                 "We currently do not support creating both turbulence physics and fluid heat "
                 "transfer physics that are not coupled together. Use "
                 "'fluid_heat_transfer_physics' to explicitly specify the coupling");
    if (_fluid_energy_physics && _fluid_energy_physics->hasEnergyEquation())
      _has_energy_equation = true;
    else
      _has_energy_equation = false;
  }
  else
  {
    _has_energy_equation = false;
    _fluid_energy_physics = nullptr;
  }

  // Sanity check for interaction with scalar transport physics
  if (isParamValid("scalar_transport_physics") && _turbulence_model != "none")
  {
    _scalar_transport_physics = getCoupledPhysics<WCNSFVScalarTransportPhysicsBase>(
        getParam<PhysicsName>("scalar_transport_physics"), true);
    if (!_scalar_transport_physics &&
        !getCoupledPhysics<const WCNSFVScalarTransportPhysicsBase>(true).empty())
      paramError(
          "scalar_transport_physics",
          "We currently do not support creating both turbulence physics and scalar transport "
          "physics that are not coupled together");
    if (_scalar_transport_physics && _scalar_transport_physics->hasScalarEquations())
      _has_scalar_equations = true;
    else
      _has_scalar_equations = false;
  }
  else
  {
    _has_scalar_equations = false;
    _scalar_transport_physics = nullptr;
  }

  // To help remediate the danger of the parameter setup
  if (_verbose)
  {
    if (_has_energy_equation)
      mooseInfoRepeated("Coupling turbulence physics with fluid heat transfer physics " +
                        _fluid_energy_physics->name());
    else
      mooseInfoRepeated("No fluid heat transfer equation considered by this turbulence "
                        "physics.");
    if (_has_scalar_equations)
      mooseInfoRepeated("Coupling turbulence physics with scalar transport physics " +
                        _scalar_transport_physics->name());
    else
      mooseInfoRepeated("No scalar transport equations considered by this turbulence physics.");
  }
}

void
WCNSFVTurbulencePhysicsBase::addInitialConditions()
{
  if (_turbulence_model == "mixing-length" || _turbulence_model == "none")
    return;
  const std::string ic_type = "FunctionIC";
  InputParameters params = getFactory().getValidParams(ic_type);

  // Parameter checking: error if initial conditions are provided but not going to be used
  if ((getParam<bool>("initialize_variables_from_mesh_file") || !_define_variables) &&
      ((getParam<bool>("mu_t_as_aux_variable") && isParamValid("initial_mu_t")) ||
       isParamSetByUser("initial_tke") || isParamSetByUser("initial_tked")))
    mooseError("inital_mu_t/tke/tked should not be provided if we are restarting from a mesh file "
               "or not defining variables in the Physics");

  // do not set initial conditions if we are not defining variables
  if (!_define_variables)
    return;
  // on regular restarts (from checkpoint), we obey the user specification of initial conditions

  if (getParam<bool>("mu_t_as_aux_variable"))
  {
    const auto rho_name = _flow_equations_physics->densityName();
    // If the user provided an initial value, we use that
    if (isParamValid("initial_mu_t"))
      params.set<FunctionName>("function") = getParam<FunctionName>("initial_mu_t");
    // If we can compute the initialization value from the user parameters, we do that
    else if (MooseUtils::isFloat(rho_name) &&
             MooseUtils::isFloat(getParam<FunctionName>("initial_tke")) &&
             MooseUtils::isFloat(getParam<FunctionName>("initial_tked")))
      params.set<FunctionName>("function") =
          std::to_string(std::atof(rho_name.c_str()) * getParam<Real>("C_mu") *
                         std::pow(std::atof(getParam<FunctionName>("initial_tke").c_str()), 2) /
                         std::atof(getParam<FunctionName>("initial_tked").c_str()));
    else
      paramError("initial_mu_t",
                 "Initial turbulent viscosity should be provided. A sensible value is "
                 "rho * C_mu TKE_initial^2 / TKED_initial");

    params.set<VariableName>("variable") = _turbulent_viscosity_name;
    // Always obey the user specification of an initial condition
    if (shouldCreateIC(_turbulent_viscosity_name,
                       _blocks,
                       /*whether IC is a default*/ !isParamSetByUser("initial_mu_t"),
                       /*error if already an IC*/ isParamSetByUser("initial_mu_t")))
      getProblem().addInitialCondition(ic_type, prefix() + "initial_mu_turb", params);
  }
  else if (isParamSetByUser("initial_mu_t"))
    paramError("initial_mu_t",
               "This parameter can only be specified if 'mu_t_as_aux_variable=true'");

  params.set<VariableName>("variable") = _tke_name;
  params.set<FunctionName>("function") = getParam<FunctionName>("initial_tke");
  if (shouldCreateIC(_tke_name,
                     _blocks,
                     /*whether IC is a default*/ !isParamSetByUser("initial_tke"),
                     /*error if already an IC*/ isParamSetByUser("initial_tke")))
    getProblem().addInitialCondition(ic_type, prefix() + "initial_tke", params);
  params.set<VariableName>("variable") = _tked_name;
  params.set<FunctionName>("function") = getParam<FunctionName>("initial_tked");
  if (shouldCreateIC(_tked_name,
                     _blocks,
                     /*whether IC is a default*/ !isParamSetByUser("initial_tked"),
                     /*error if already an IC*/ isParamSetByUser("initial_tked")))
    getProblem().addInitialCondition(ic_type, prefix() + "initial_tked", params);
}

void
WCNSFVTurbulencePhysicsBase::addAuxiliaryVariables()
{
  // Not future-proof
  const bool is_linear = dynamic_cast<WCNSLinearFVTurbulencePhysics *>(this);
  const auto var_type = is_linear ? "MooseLinearVariableFVReal" : "MooseVariableFVReal";

  if (_turbulence_model == "k-epsilon" && getParam<bool>("mu_t_as_aux_variable"))
  {
    auto params = getFactory().getValidParams(var_type);
    assignBlocks(params, _blocks);
    if (!is_linear && isParamValid("turbulent_viscosity_two_term_bc_expansion"))
      params.set<bool>("two_term_boundary_expansion") =
          getParam<bool>("turbulent_viscosity_two_term_bc_expansion");
    if (!shouldCreateVariable(_turbulent_viscosity_name, _blocks, /*error if aux*/ false))
      reportPotentiallyMissedParameters({"turbulent_viscosity_two_term_bc_expansion"}, var_type);
    else
      getProblem().addAuxVariable(var_type, _turbulent_viscosity_name, params);
  }
  if (_turbulence_model == "k-epsilon" && getParam<bool>("k_t_as_aux_variable"))
  {
    auto params = getFactory().getValidParams(var_type);
    assignBlocks(params, _blocks);
    if (shouldCreateVariable(NS::k_t, _blocks, /*error if aux*/ false))
      getProblem().addAuxVariable(var_type, NS::k_t, params);
  }
}

void
WCNSFVTurbulencePhysicsBase::addAuxiliaryKernels()
{
  const std::string u_names[3] = {"u", "v", "w"};
  // Not future-proof
  const bool is_linear = dynamic_cast<WCNSLinearFVTurbulencePhysics *>(this);

  if (_turbulence_model == "k-epsilon" && getParam<bool>("mu_t_as_aux_variable"))
  {
    auto params = getFactory().getValidParams("kEpsilonViscosityAux");
    assignBlocks(params, _blocks);

    params.set<AuxVariableName>("variable") = _turbulent_viscosity_name;
    params.set<MooseFunctorName>(NS::density) = _flow_equations_physics->densityName();
    params.set<MooseFunctorName>(NS::mu) = _flow_equations_physics->dynamicViscosityName();
    params.set<MooseFunctorName>(NS::TKE) = _tke_name;
    params.set<MooseFunctorName>(NS::TKED) = _tked_name;
    params.set<std::vector<BoundaryName>>("walls") = _turbulence_walls;
    params.set<MooseEnum>("wall_treatment") = _wall_treatment_eps;
    for (const auto d : make_range(dimension()))
      params.set<MooseFunctorName>(u_names[d]) = _velocity_names[d];

    params.set<bool>("newton_solve") = !is_linear;
    params.applySpecificParameters(parameters(), {"C_mu", "bulk_wall_treatment", "mu_t_ratio_max"});
    params.set<ExecFlagEnum>("execute_on") = {EXEC_NONLINEAR};

    getProblem().addAuxKernel("kEpsilonViscosityAux", name() + "_viscosity_aux", params);
  }
  if (_turbulence_model == "k-epsilon" && _has_energy_equation &&
      getParam<bool>("k_t_as_aux_variable"))
  {
    auto params = getFactory().getValidParams("TurbulentConductivityAux");
    assignBlocks(params, _blocks);
    params.set<AuxVariableName>("variable") = NS::k_t;
    params.set<MooseFunctorName>(NS::cp) = _fluid_energy_physics->getSpecificHeatName();
    params.set<MooseFunctorName>(NS::mu_t) = _turbulent_viscosity_name;
    params.applySpecificParameters(parameters(), {"Pr_t"});
    getProblem().addAuxKernel(
        "TurbulentConductivityAux", name() + "_thermal_conductivity_aux", params);
  }
}

void
WCNSFVTurbulencePhysicsBase::addMaterials()
{
  // Not future-proof
  const bool is_linear = dynamic_cast<WCNSLinearFVTurbulencePhysics *>(this);
  if (_turbulence_model == "k-epsilon")
  {
    if (!getProblem().hasFunctor(NS::mu_eff, /*thread_id=*/0))
    {
      const auto object_type = is_linear ? "ParsedFunctorMaterial" : "ADParsedFunctorMaterial";
      InputParameters params = getFactory().getValidParams(object_type);
      assignBlocks(params, _blocks);
      const auto mu_name = _flow_equations_physics->dynamicViscosityName();

      // Avoid defining floats as functors in the parsed expression
      if (!MooseUtils::isFloat(_turbulent_viscosity_name) && !MooseUtils::isFloat(mu_name))
        params.set<std::vector<std::string>>("functor_names") = {_turbulent_viscosity_name,
                                                                 mu_name};
      else if (MooseUtils::isFloat(_turbulent_viscosity_name) && !MooseUtils::isFloat(mu_name))
        params.set<std::vector<std::string>>("functor_names") = {mu_name};
      else if (!MooseUtils::isFloat(_turbulent_viscosity_name) && MooseUtils::isFloat(mu_name))
        params.set<std::vector<std::string>>("functor_names") = {_turbulent_viscosity_name};

      params.set<std::string>("expression") =
          _flow_equations_physics->dynamicViscosityName() + " + " + _turbulent_viscosity_name;
      params.set<std::string>("property_name") = NS::mu_eff;
      getProblem().addMaterial(object_type, prefix() + "effective_viscosity", params);
    }
    if (!getParam<bool>("mu_t_as_aux_variable"))
    {
      // TODO: suppress this for linearFV
      InputParameters params = getFactory().getValidParams("INSFVkEpsilonViscosityFunctorMaterial");
      params.set<MooseFunctorName>(NS::TKE) = _tke_name;
      params.set<MooseFunctorName>(NS::TKED) = _tked_name;
      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<ExecFlagEnum>("execute_on") = {EXEC_NONLINEAR};
      if (getParam<bool>("output_mu_t"))
        params.set<std::vector<OutputName>>("outputs") = {"all"};
      getProblem().addMaterial(
          "INSFVkEpsilonViscosityFunctorMaterial", prefix() + "compute_mu_t", params);
    }

    if (_has_energy_equation && !getProblem().hasFunctor(NS::k_t, /*thread_id=*/0))
    {
      mooseAssert(!getParam<bool>("k_t_as_aux_variable"), "k_t should not exist");
      const auto object_type = is_linear ? "ParsedFunctorMaterial" : "ADParsedFunctorMaterial";
      InputParameters params = getFactory().getValidParams(object_type);
      assignBlocks(params, _blocks);
      const auto mu_t_name = NS::mu_t;
      const auto cp_name = _fluid_energy_physics->getSpecificHeatName();
      const auto Pr_t_name = getParam<MooseFunctorName>("Pr_t");

      // Avoid defining floats as functors in the parsed expression
      if (!MooseUtils::isFloat(cp_name) && !MooseUtils::isFloat(Pr_t_name))
        params.set<std::vector<std::string>>("functor_names") = {cp_name, Pr_t_name, mu_t_name};
      else if (MooseUtils::isFloat(cp_name) && !MooseUtils::isFloat(Pr_t_name))
        params.set<std::vector<std::string>>("functor_names") = {Pr_t_name, mu_t_name};
      else if (!MooseUtils::isFloat(cp_name) && MooseUtils::isFloat(Pr_t_name))
        params.set<std::vector<std::string>>("functor_names") = {cp_name, mu_t_name};
      else
        params.set<std::vector<std::string>>("functor_names") = {mu_t_name};

      params.set<std::string>("expression") = mu_t_name + "*" + cp_name + "/" + Pr_t_name;
      params.set<std::string>("property_name") = NS::k_t;
      params.set<ExecFlagEnum>("execute_on") = {EXEC_NONLINEAR};
      params.set<std::vector<OutputName>>("outputs") = {"all"};
      getProblem().addMaterial(object_type, prefix() + "turbulent_heat_eff_conductivity", params);
    }

    if (_has_scalar_equations && !getProblem().hasFunctor(NS::mu_t_passive_scalar, /*thread_id=*/0))
    {
      const auto object_type = is_linear ? "ParsedFunctorMaterial" : "ADParsedFunctorMaterial";
      InputParameters params = getFactory().getValidParams(object_type);
      assignBlocks(params, _blocks);
      const auto & rho_name = _flow_equations_physics->densityName();

      // Avoid defining floats as functors in the parsed expression
      if (!MooseUtils::isFloat(_turbulent_viscosity_name) && !MooseUtils::isFloat(rho_name))
        params.set<std::vector<std::string>>("functor_names") = {_turbulent_viscosity_name,
                                                                 rho_name};
      else if (MooseUtils::isFloat(_turbulent_viscosity_name) && !MooseUtils::isFloat(rho_name))
        params.set<std::vector<std::string>>("functor_names") = {rho_name};
      else if (!MooseUtils::isFloat(_turbulent_viscosity_name) && MooseUtils::isFloat(rho_name))
        params.set<std::vector<std::string>>("functor_names") = {_turbulent_viscosity_name};

      const auto turbulent_schmidt_number = getParam<std::vector<Real>>("Sc_t");
      if (turbulent_schmidt_number.size() != 1)
        paramError("passive_scalar_schmidt_number",
                   "A single passive scalar turbulent Schmidt number can and must be specified "
                   "with k-epsilon");
      params.set<std::string>("expression") = _turbulent_viscosity_name + "/" + rho_name + " / " +
                                              std::to_string(turbulent_schmidt_number[0]);
      params.set<std::string>("property_name") = NS::mu_t_passive_scalar;
      getProblem().addMaterial(object_type, prefix() + "scalar_turbulent_diffusivity", params);
    }
  }
}
