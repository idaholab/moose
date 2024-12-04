//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVTurbulencePhysics.h"
#include "WCNSFVFlowPhysics.h"
#include "WCNSFVFluidHeatTransferPhysics.h"
#include "WCNSFVScalarTransportPhysics.h"
#include "WCNSFVCoupledAdvectionPhysicsHelper.h"
#include "INSFVTurbulentViscosityWallFunction.h"
#include "INSFVTKESourceSink.h"
#include "NSFVBase.h"

registerNavierStokesPhysicsBaseTasks("NavierStokesApp", WCNSFVTurbulencePhysics);
registerMooseAction("NavierStokesApp", WCNSFVTurbulencePhysics, "get_turbulence_physics");
registerMooseAction("NavierStokesApp", WCNSFVTurbulencePhysics, "add_variable");
registerMooseAction("NavierStokesApp", WCNSFVTurbulencePhysics, "add_fv_kernel");
registerMooseAction("NavierStokesApp", WCNSFVTurbulencePhysics, "add_fv_bc");
registerMooseAction("NavierStokesApp", WCNSFVTurbulencePhysics, "add_ic");
registerMooseAction("NavierStokesApp", WCNSFVTurbulencePhysics, "add_aux_variable");
registerMooseAction("NavierStokesApp", WCNSFVTurbulencePhysics, "add_aux_kernel");
registerMooseAction("NavierStokesApp", WCNSFVTurbulencePhysics, "add_material");

InputParameters
WCNSFVTurbulencePhysics::validParams()
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
  params.transferParam<bool>(NSFVBase::validParams(), "mixing_length_two_term_bc_expansion");

  // TODO Added to facilitate transition, remove default once NavierStokesFV action is removed
  params.addParam<AuxVariableName>(
      "mixing_length_name", "mixing_length", "Name of the mixing length auxiliary variable");
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

  // Boundary parameters
  params.addParam<bool>("bulk_wall_treatment", true, "Whether to treat the wall cell as bulk");
  MooseEnum wall_treatment("eq_newton eq_incremental eq_linearized neq", "neq");
  params.addParam<MooseEnum>("wall_treatment_eps",
                             wall_treatment,
                             "The method used for computing the epsilon wall functions");
  params.addParam<MooseEnum>("wall_treatment_T",
                             wall_treatment,
                             "The method used for computing the temperature wall functions");
  params.transferParam<Real>(INSFVTurbulentViscosityWallFunction::validParams(), "C_mu");

  // K-Epsilon numerical scheme parameters
  MooseEnum face_interpol_types("average skewness-corrected", "average");
  MooseEnum adv_interpol_types("average upwind", "upwind");
  params.addRangeCheckedParam<Real>(
      "tke_scaling",
      1.0,
      "tke_scaling > 0.0",
      "The scaling factor for the turbulent kinetic energy equation.");
  params.addParam<MooseEnum>("tke_face_interpolation",
                             face_interpol_types,
                             "The numerical scheme to interpolate the TKE to the "
                             "face (separate from the advected quantity interpolation).");
  params.addParam<MooseEnum>("tke_advection_interpolation",
                             adv_interpol_types,
                             "The numerical scheme to interpolate the TKE to the "
                             "face when in the advection kernel.");
  // Better Jacobian if not linearizing sink and sources
  params.addParam<bool>("linearize_sink_sources", false, "Whether to linearize the source term");
  // Better convergence on some cases when neglecting advection derivatives
  params.addParam<bool>(
      "neglect_advection_derivatives",
      false,
      "Whether to remove the off-diagonal velocity term in the TKE and TKED advection term");
  params.addParam<bool>(
      "tke_two_term_bc_expansion",
      true,
      "If a two-term Taylor expansion is needed for the determination of the boundary values"
      "of the turbulent kinetic energy.");
  params.addRangeCheckedParam<Real>(
      "tked_scaling",
      1.0,
      "tked_scaling > 0.0",
      "The scaling factor for the turbulent kinetic energy dissipation equation.");
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
      true,
      "If a two-term Taylor expansion is needed for the determination of the boundary values"
      "of the turbulent kinetic energy dissipation.");
  params.addParam<bool>(
      "turbulent_viscosity_two_term_bc_expansion",
      true,
      "If a two-term Taylor expansion is needed for the determination of the boundary values"
      "of the turbulent viscosity.");
  MooseEnum coeff_interp_method("average harmonic", "harmonic");
  params.addParam<MooseEnum>("turbulent_viscosity_interp_method",
                             coeff_interp_method,
                             "Face interpolation method for the turbulent viscosity");
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
      "WCNSFVFluidHeatTransferPhysics generating the heat advection equations");
  params.addParam<PhysicsName>(
      "scalar_transport_physics",
      "NavierStokesFV",
      "WCNSFVScalarTransportPhysics generating the scalar advection equations");

  // Parameter groups
  params.addParamNamesToGroup("mixing_length_name mixing_length_two_term_bc_expansion",
                              "Mixing length model");
  params.addParamNamesToGroup("fluid_heat_transfer_physics turbulent_prandtl "
                              "scalar_transport_physics Sc_t",
                              "Coupled Physics");
  params.addParamNamesToGroup("initial_tke initial_tked C1_eps C2_eps sigma_k sigma_eps",
                              "K-Epsilon model");
  params.addParamNamesToGroup("C_mu bulk_wall_treatment wall_treatment_eps wall_treatment_T",
                              "K-Epsilon wall function");
  params.addParamNamesToGroup(
      "tke_scaling tke_face_interpolation tke_two_term_bc_expansion tked_scaling "
      "tked_face_interpolation tked_two_term_bc_expansion "
      "turbulent_viscosity_two_term_bc_expansion turbulent_viscosity_interp_method "
      "mu_t_as_aux_variable k_t_as_aux_variable linearize_sink_sources",
      "K-Epsilon model numerical");

  return params;
}

WCNSFVTurbulencePhysics::WCNSFVTurbulencePhysics(const InputParameters & parameters)
  : NavierStokesPhysicsBase(parameters),
    WCNSFVCoupledAdvectionPhysicsHelper(this),
    _turbulence_model(getParam<MooseEnum>("turbulence_handling")),
    _mixing_length_name(getParam<AuxVariableName>("mixing_length_name")),
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
  if (_turbulence_model == "mixing-length")
    saveAuxVariableName(_mixing_length_name);
  else if (_turbulence_model == "k-epsilon")
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
  if (_turbulence_model != "mixing-length")
    errorDependentParameter("turbulence_handling",
                            "mixing-length",
                            {"mixing_length_delta",
                             "mixing_length_aux_execute_on",
                             "von_karman_const",
                             "von_karman_const_0",
                             "mixing_length_two_term_bc_expansion"});
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
WCNSFVTurbulencePhysics::initializePhysicsAdditional()
{
  if (_turbulence_model == "k-epsilon")
    getProblem().needSolutionState(2, Moose::SolutionIterationType::Nonlinear);
}

void
WCNSFVTurbulencePhysics::actOnAdditionalTasks()
{
  // Other Physics may not exist or be initialized at construction time, so
  // we retrieve them now, on this task which occurs after 'init_physics'
  if (_current_task == "get_turbulence_physics")
    retrieveCoupledPhysics();
}

void
WCNSFVTurbulencePhysics::retrieveCoupledPhysics()
{
  // _flow_equations_physics is initialized by 'WCNSFVCoupledAdvectionPhysicsHelper'
  if (_flow_equations_physics && _flow_equations_physics->hasFlowEquations())
    _has_flow_equations = true;
  else
    _has_flow_equations = false;

  // Sanity check for interaction for fluid heat transfer physics
  if (isParamValid("fluid_heat_transfer_physics") && _turbulence_model != "none")
  {
    _fluid_energy_physics = getCoupledPhysics<WCNSFVFluidHeatTransferPhysics>(
        getParam<PhysicsName>("fluid_heat_transfer_physics"), true);
    // Check for a missing parameter / do not support isolated physics for now
    if (!_fluid_energy_physics &&
        !getCoupledPhysics<const WCNSFVFluidHeatTransferPhysics>(true).empty())
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
    _scalar_transport_physics = getCoupledPhysics<WCNSFVScalarTransportPhysics>(
        getParam<PhysicsName>("scalar_transport_physics"), true);
    if (!_scalar_transport_physics &&
        !getCoupledPhysics<const WCNSFVScalarTransportPhysics>(true).empty())
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
WCNSFVTurbulencePhysics::addSolverVariables()
{
  if (_turbulence_model == "mixing-length" || _turbulence_model == "none")
    return;
  else if (_turbulence_model == "k-epsilon")
  {
    // Dont add if the user already defined the variable
    // Add turbulent kinetic energy variable
    if (variableExists(_tke_name,
                       /*error_if_aux=*/true))
      checkBlockRestrictionIdentical(_tke_name, getProblem().getVariable(0, _tke_name).blocks());
    else if (_define_variables)
    {
      auto params = getFactory().getValidParams("INSFVEnergyVariable");
      assignBlocks(params, _blocks);
      params.set<std::vector<Real>>("scaling") = {getParam<Real>("tke_scaling")};
      params.set<MooseEnum>("face_interp_method") = getParam<MooseEnum>("tke_face_interpolation");
      params.set<bool>("two_term_boundary_expansion") = getParam<bool>("tke_two_term_bc_expansion");
      params.set<SolverSystemName>("solver_sys") = getSolverSystem(_tke_name);

      getProblem().addVariable("INSFVEnergyVariable", _tke_name, params);
    }
    else
      paramError("turbulence_kinetic_energy_variable",
                 "Variable (" + _tke_name +
                     ") supplied to the WCNSFVTurbulencePhysics does not exist!");

    // Add turbulent kinetic energy dissipation variable
    if (variableExists(_tked_name,
                       /*error_if_aux=*/true))
      checkBlockRestrictionIdentical(_tked_name, getProblem().getVariable(0, _tked_name).blocks());
    else if (_define_variables)
    {
      auto params = getFactory().getValidParams("INSFVEnergyVariable");
      assignBlocks(params, _blocks);
      params.set<std::vector<Real>>("scaling") = {getParam<Real>("tked_scaling")};
      params.set<MooseEnum>("face_interp_method") = getParam<MooseEnum>("tked_face_interpolation");
      params.set<bool>("two_term_boundary_expansion") =
          getParam<bool>("tked_two_term_bc_expansion");
      params.set<SolverSystemName>("solver_sys") = getSolverSystem(_tked_name);
      getProblem().addVariable("INSFVEnergyVariable", _tked_name, params);
    }
    else
      paramError("turbulence_kinetic_energy_dissipation_variable",
                 "Variable (" + _tked_name +
                     ") supplied to the WCNSFVTurbulencePhysics does not exist!");
  }
}

void
WCNSFVTurbulencePhysics::addAuxiliaryVariables()
{
  if (_turbulence_model == "mixing-length" && _define_variables)
  {
    auto params = getFactory().getValidParams("MooseVariableFVReal");
    assignBlocks(params, _blocks);
    if (isParamValid("mixing_length_two_term_bc_expansion"))
      params.set<bool>("two_term_boundary_expansion") =
          getParam<bool>("mixing_length_two_term_bc_expansion");
    getProblem().addAuxVariable("MooseVariableFVReal", _mixing_length_name, params);
  }
  if (_turbulence_model == "k-epsilon" && getParam<bool>("mu_t_as_aux_variable"))
  {
    auto params = getFactory().getValidParams("MooseVariableFVReal");
    assignBlocks(params, _blocks);
    if (isParamValid("turbulent_viscosity_two_term_bc_expansion"))
      params.set<bool>("two_term_boundary_expansion") =
          getParam<bool>("turbulent_viscosity_two_term_bc_expansion");
    getProblem().addAuxVariable("MooseVariableFVReal", _turbulent_viscosity_name, params);
  }
  if (_turbulence_model == "k-epsilon" && getParam<bool>("k_t_as_aux_variable"))
  {
    auto params = getFactory().getValidParams("MooseVariableFVReal");
    assignBlocks(params, _blocks);
    getProblem().addAuxVariable("MooseVariableFVReal", NS::k_t, params);
  }
}

void
WCNSFVTurbulencePhysics::addFVKernels()
{
  if (_turbulence_model == "none")
    return;

  // Turbulence terms in other equations
  if (_has_flow_equations)
    addFlowTurbulenceKernels();
  if (_has_energy_equation)
    addFluidEnergyTurbulenceKernels();
  if (_has_scalar_equations)
    addScalarAdvectionTurbulenceKernels();

  // Turbulence models with their own set of equations
  if (_turbulence_model == "k-epsilon")
  {
    if (isTransient())
      addKEpsilonTimeDerivatives();
    addKEpsilonAdvection();
    addKEpsilonDiffusion();
    addKEpsilonSink();
  }
}

void
WCNSFVTurbulencePhysics::addFlowTurbulenceKernels()
{
  if (_turbulence_model == "mixing-length")
  {
    const std::string u_names[3] = {"u", "v", "w"};
    const std::string kernel_type = "INSFVMixingLengthReynoldsStress";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<MooseFunctorName>(NS::mixing_length) = _mixing_length_name;

    std::string kernel_name = prefix() + "ins_momentum_mixing_length_reynolds_stress_";
    if (_porous_medium_treatment)
      kernel_name = prefix() + "pins_momentum_mixing_length_reynolds_stress_";

    params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
    for (const auto dim_i : make_range(dimension()))
      params.set<MooseFunctorName>(u_names[dim_i]) = _velocity_names[dim_i];

    for (const auto d : make_range(dimension()))
    {
      params.set<NonlinearVariableName>("variable") = _velocity_names[d];
      params.set<MooseEnum>("momentum_component") = NS::directions[d];

      getProblem().addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
    }
  }
  else if (_turbulence_model == "k-epsilon")
  {
    // We rely on using the turbulent viscosity in the flow equation
    // This check is rudimentary, we should think of a better way
    // We could also check for the use of 'mu_t' with the right parameters already
    if (_flow_equations_physics->dynamicViscosityName() != "mu" &&
        !MooseUtils::isFloat(_flow_equations_physics->dynamicViscosityName()))
      mooseError(
          "Regular fluid viscosity 'mu' should be used for the momentum diffusion term. You are "
          "currently using: " +
          _flow_equations_physics->dynamicViscosityName());

    const std::string u_names[3] = {"u", "v", "w"};
    const std::string kernel_type = "INSFVMomentumDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<MooseFunctorName>("mu") = _turbulent_viscosity_name;
    params.set<MooseEnum>("mu_interp_method") =
        getParam<MooseEnum>("turbulent_viscosity_interp_method");
    params.set<bool>("complete_expansion") = true;

    std::string kernel_name = prefix() + "ins_momentum_k_epsilon_reynolds_stress_";
    if (_porous_medium_treatment)
      kernel_name = prefix() + "pins_momentum_k_epsilon_reynolds_stress_";

    params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
    for (const auto dim_i : make_range(dimension()))
      params.set<MooseFunctorName>(u_names[dim_i]) = _velocity_names[dim_i];

    for (const auto d : make_range(dimension()))
    {
      params.set<NonlinearVariableName>("variable") = _velocity_names[d];
      params.set<MooseEnum>("momentum_component") = NS::directions[d];

      getProblem().addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
    }
  }
}

void
WCNSFVTurbulencePhysics::addFluidEnergyTurbulenceKernels()
{
  if (_turbulence_model == "mixing-length")
  {
    const std::string u_names[3] = {"u", "v", "w"};
    const std::string kernel_type = "WCNSFVMixingLengthEnergyDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<MooseFunctorName>(NS::cp) = _fluid_energy_physics->getSpecificHeatName();
    params.set<MooseFunctorName>(NS::mixing_length) = _mixing_length_name;
    params.set<Real>("schmidt_number") = getParam<Real>("turbulent_prandtl");
    params.set<NonlinearVariableName>("variable") =
        _flow_equations_physics->getFluidTemperatureName();

    for (const auto dim_i : make_range(dimension()))
      params.set<MooseFunctorName>(u_names[dim_i]) = _velocity_names[dim_i];

    if (_porous_medium_treatment)
      getProblem().addFVKernel(
          kernel_type, prefix() + "pins_energy_mixing_length_diffusion", params);
    else
      getProblem().addFVKernel(
          kernel_type, prefix() + "ins_energy_mixing_length_diffusion", params);
  }
  else if (_turbulence_model == "k-epsilon")
  {
    const std::string kernel_type = "FVDiffusion";
    const auto T_fluid_name = _flow_equations_physics->getFluidTemperatureName();
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<NonlinearVariableName>("variable") = T_fluid_name;
    params.set<MooseFunctorName>("coeff") = NS::k_t;
    getProblem().addFVKernel(kernel_type, prefix() + T_fluid_name + "_turbulent_diffusion", params);
  }
}

void
WCNSFVTurbulencePhysics::addScalarAdvectionTurbulenceKernels()
{
  const auto & passive_scalar_names = _scalar_transport_physics->getAdvectedScalarNames();
  const auto & passive_scalar_schmidt_number = getParam<std::vector<Real>>("Sc_t");
  if (passive_scalar_schmidt_number.size() != passive_scalar_names.size() &&
      passive_scalar_schmidt_number.size() != 1)
    paramError(
        "Sc_t",
        "The number of turbulent Schmidt numbers defined is not equal to the number of passive "
        "scalar fields!");

  if (_turbulence_model == "mixing-length")
  {
    const std::string u_names[3] = {"u", "v", "w"};
    const std::string kernel_type = "INSFVMixingLengthScalarDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<MooseFunctorName>(NS::mixing_length) = _mixing_length_name;
    for (const auto dim_i : make_range(dimension()))
      params.set<MooseFunctorName>(u_names[dim_i]) = _velocity_names[dim_i];

    for (const auto & name_i : index_range(passive_scalar_names))
    {
      params.set<NonlinearVariableName>("variable") = passive_scalar_names[name_i];
      if (passive_scalar_schmidt_number.size() > 1)
        params.set<Real>("schmidt_number") = passive_scalar_schmidt_number[name_i];
      else if (passive_scalar_schmidt_number.size() == 1)
        params.set<Real>("schmidt_number") = passive_scalar_schmidt_number[0];
      else
        params.set<Real>("schmidt_number") = 1.0;

      getProblem().addFVKernel(
          kernel_type, prefix() + passive_scalar_names[name_i] + "_mixing_length", params);
    }
  }
  else if (_turbulence_model == "k-epsilon")
  {
    const std::string kernel_type = "FVDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);

    for (const auto & name_i : index_range(passive_scalar_names))
    {
      params.set<NonlinearVariableName>("variable") = passive_scalar_names[name_i];
      params.set<MooseFunctorName>("coeff") = NS::mu_t_passive_scalar;
      getProblem().addFVKernel(
          kernel_type, prefix() + passive_scalar_names[name_i] + "_turbulent_diffusion", params);
    }
  }
}

void
WCNSFVTurbulencePhysics::addKEpsilonTimeDerivatives()
{
  const std::string kernel_type = "FVFunctorTimeKernel";
  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);

  params.set<NonlinearVariableName>("variable") = _tke_name;
  getProblem().addFVKernel(kernel_type, prefix() + "tke_time", params);
  params.set<NonlinearVariableName>("variable") = _tked_name;
  getProblem().addFVKernel(kernel_type, prefix() + "tked_time", params);
}

void
WCNSFVTurbulencePhysics::addKEpsilonAdvection()
{
  const std::string kernel_type = "INSFVTurbulentAdvection";
  InputParameters params = getFactory().getValidParams(kernel_type);

  assignBlocks(params, _blocks);

  params.set<MooseEnum>("velocity_interp_method") = _velocity_interpolation;
  params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
  params.set<MooseFunctorName>(NS::density) = _flow_equations_physics->densityName();
  params.set<bool>("neglect_advection_derivatives") =
      getParam<bool>("neglect_advection_derivatives");

  params.set<MooseEnum>("advected_interp_method") =
      getParam<MooseEnum>("tke_advection_interpolation");
  params.set<NonlinearVariableName>("variable") = _tke_name;
  getProblem().addFVKernel(kernel_type, prefix() + "tke_advection", params);
  params.set<NonlinearVariableName>("variable") = _tked_name;
  params.set<std::vector<BoundaryName>>("walls") = _turbulence_walls;
  params.set<MooseEnum>("advected_interp_method") =
      getParam<MooseEnum>("tked_advection_interpolation");
  getProblem().addFVKernel(kernel_type, prefix() + "tked_advection", params);
}

void
WCNSFVTurbulencePhysics::addKEpsilonDiffusion()
{
  {
    const std::string kernel_type = "INSFVTurbulentDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);

    params.set<NonlinearVariableName>("variable") = _tke_name;
    params.set<MooseFunctorName>("coeff") = _flow_equations_physics->dynamicViscosityName();
    getProblem().addFVKernel(kernel_type, prefix() + "tke_diffusion_mu", params);

    params.set<std::vector<BoundaryName>>("walls") = _turbulence_walls;
    params.set<NonlinearVariableName>("variable") = _tked_name;
    getProblem().addFVKernel(kernel_type, prefix() + "tked_diffusion_mu", params);
  }

  {
    const std::string kernel_type = "INSFVTurbulentDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);

    params.set<NonlinearVariableName>("variable") = _tke_name;
    params.set<MooseFunctorName>("coeff") = _turbulent_viscosity_name;
    params.set<MooseFunctorName>("scaling_coef") = getParam<MooseFunctorName>("sigma_k");
    params.set<MooseEnum>("coeff_interp_method") =
        getParam<MooseEnum>("turbulent_viscosity_interp_method");
    getProblem().addFVKernel(kernel_type, prefix() + "tke_diffusion_mu_turb", params);

    params.set<std::vector<BoundaryName>>("walls") = _turbulence_walls;
    params.set<NonlinearVariableName>("variable") = _tked_name;
    params.set<MooseFunctorName>("scaling_coef") = getParam<MooseFunctorName>("sigma_eps");
    getProblem().addFVKernel(kernel_type, prefix() + "tked_diffusion_mu_turb", params);
  }
}

void
WCNSFVTurbulencePhysics::addKEpsilonSink()
{
  const std::string u_names[3] = {"u", "v", "w"};
  {
    const std::string kernel_type = "INSFVTKESourceSink";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<NonlinearVariableName>("variable") = _tke_name;
    params.set<MooseFunctorName>(NS::TKED) = _tked_name;
    params.set<MooseFunctorName>(NS::density) = _flow_equations_physics->densityName();
    params.set<MooseFunctorName>(NS::mu) = _flow_equations_physics->dynamicViscosityName();
    params.set<MooseFunctorName>(NS::mu_t) = _turbulent_viscosity_name;
    params.set<Real>("C_pl") = getParam<Real>("C_pl");
    params.set<bool>("linearized_model") = getParam<bool>("linearize_sink_sources");
    params.set<std::vector<BoundaryName>>("walls") = _turbulence_walls;
    params.set<MooseEnum>("wall_treatment") = _wall_treatment_eps;
    // Currently only Newton method for WCNSFVTurbulencePhysics
    params.set<bool>("newton_solve") = true;
    for (const auto d : make_range(dimension()))
      params.set<MooseFunctorName>(u_names[d]) = _velocity_names[d];
    getProblem().addFVKernel(kernel_type, prefix() + "tke_source_sink", params);
  }

  {
    const std::string kernel_type = "INSFVTKEDSourceSink";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<NonlinearVariableName>("variable") = _tked_name;
    params.set<MooseFunctorName>(NS::TKE) = _tke_name;
    params.set<MooseFunctorName>(NS::density) = _flow_equations_physics->densityName();
    params.set<MooseFunctorName>(NS::mu) = _flow_equations_physics->dynamicViscosityName();
    params.set<MooseFunctorName>(NS::mu_t) = _turbulent_viscosity_name;
    params.set<Real>("C_pl") = getParam<Real>("C_pl");
    params.set<bool>("linearized_model") = getParam<bool>("linearize_sink_sources");
    params.set<std::vector<BoundaryName>>("walls") = _turbulence_walls;
    params.set<MooseEnum>("wall_treatment") = _wall_treatment_eps;
    params.set<Real>("C1_eps") = getParam<Real>("C1_eps");
    params.set<Real>("C2_eps") = getParam<Real>("C2_eps");
    // Currently only Newton method for WCNSFVTurbulencePhysics
    params.set<bool>("newton_solve") = true;
    for (const auto d : make_range(dimension()))
      params.set<MooseFunctorName>(u_names[d]) = _velocity_names[d];
    getProblem().addFVKernel(kernel_type, prefix() + "tked_source_sink", params);
  }
}

void
WCNSFVTurbulencePhysics::addAuxiliaryKernels()
{
  // Note that if we are restarting this will overwrite the restarted mixing-length
  if (_turbulence_model == "mixing-length")
  {
    const std::string ml_kernel_type = "WallDistanceMixingLengthAux";
    InputParameters ml_params = getFactory().getValidParams(ml_kernel_type);
    assignBlocks(ml_params, _blocks);
    ml_params.set<AuxVariableName>("variable") = _mixing_length_name;
    ml_params.set<std::vector<BoundaryName>>("walls") = _turbulence_walls;
    if (parameters().isParamValid("mixing_length_aux_execute_on"))
      ml_params.set<ExecFlagEnum>("execute_on") =
          getParam<ExecFlagEnum>("mixing_length_aux_execute_on");
    else
      ml_params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
    ml_params.set<MooseFunctorName>("von_karman_const") =
        getParam<MooseFunctorName>("von_karman_const");
    ml_params.set<MooseFunctorName>("von_karman_const_0") =
        getParam<MooseFunctorName>("von_karman_const_0");
    ml_params.set<MooseFunctorName>("delta") = getParam<MooseFunctorName>("mixing_length_delta");

    getProblem().addAuxKernel(ml_kernel_type, prefix() + "mixing_length_aux ", ml_params);
  }

  if (_turbulence_model == "k-epsilon" && getParam<bool>("mu_t_as_aux_variable"))
  {
    const std::string u_names[3] = {"u", "v", "w"};
    const std::string mut_kernel_type = "kEpsilonViscosityAux";
    InputParameters params = getFactory().getValidParams(mut_kernel_type);
    assignBlocks(params, _blocks);
    params.set<AuxVariableName>("variable") = _turbulent_viscosity_name;
    for (const auto d : make_range(dimension()))
      params.set<MooseFunctorName>(u_names[d]) = _velocity_names[d];
    params.set<MooseFunctorName>(NS::TKE) = _tke_name;
    params.set<MooseFunctorName>(NS::TKED) = _tked_name;
    params.set<MooseFunctorName>(NS::density) = _flow_equations_physics->densityName();
    params.set<MooseFunctorName>(NS::mu) = _flow_equations_physics->dynamicViscosityName();
    params.set<Real>("C_mu") = getParam<Real>("C_mu");
    params.set<MooseEnum>("wall_treatment") = _wall_treatment_eps;
    params.set<bool>("bulk_wall_treatment") = getParam<bool>("bulk_wall_treatment");
    params.set<std::vector<BoundaryName>>("walls") = _turbulence_walls;
    params.set<ExecFlagEnum>("execute_on") = {EXEC_NONLINEAR};
    params.set<bool>("newton_solve") = true;
    getProblem().addAuxKernel(mut_kernel_type, prefix() + "mixing_length_aux ", params);
  }
  if (_turbulence_model == "k-epsilon" && getParam<bool>("k_t_as_aux_variable") &&
      _has_energy_equation)
  {
    const std::string kt_kernel_type = "TurbulentConductivityAux";
    InputParameters params = getFactory().getValidParams(kt_kernel_type);
    assignBlocks(params, _blocks);
    params.set<AuxVariableName>("variable") = NS::k_t;
    params.set<MooseFunctorName>(NS::cp) = _fluid_energy_physics->getSpecificHeatName();
    params.set<MooseFunctorName>(NS::mu_t) = _turbulent_viscosity_name;
    params.set<MooseFunctorName>(NS::turbulent_Prandtl) = getParam<MooseFunctorName>("Pr_t");
    params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_NONLINEAR};
    getProblem().addAuxKernel(kt_kernel_type, prefix() + "turbulent_conductivity_aux ", params);
  }
}

void
WCNSFVTurbulencePhysics::addFVBCs()
{
  const std::string u_names[3] = {"u", "v", "w"};

  if (_turbulence_model == "k-epsilon" && getParam<bool>("mu_t_as_aux_variable"))
  {
    mooseAssert(_flow_equations_physics, "Should have a flow equation physics");
    const std::string bc_type = "INSFVTurbulentViscosityWallFunction";
    InputParameters params = getFactory().getValidParams(bc_type);
    params.set<std::vector<BoundaryName>>("boundary") = _turbulence_walls;
    params.set<NonlinearVariableName>("variable") = _turbulent_viscosity_name;
    params.set<MooseFunctorName>(NS::density) = _flow_equations_physics->densityName();
    params.set<MooseFunctorName>(NS::mu) = _flow_equations_physics->dynamicViscosityName();
    params.set<MooseFunctorName>(NS::mu_t) = _turbulent_viscosity_name;
    params.set<MooseFunctorName>(NS::TKE) = _tke_name;
    params.set<MooseEnum>("wall_treatment") = _wall_treatment_eps;
    for (const auto d : make_range(dimension()))
      params.set<MooseFunctorName>(u_names[d]) = _velocity_names[d];

    getProblem().addFVBC(bc_type, prefix() + "turbulence_walls", params);
    // Energy wall function boundary conditions are added in the WCNSFVFluidEnergyPhysics
    // because it facilitates counting the number of walls, specifying energy wall functors
    // the same way as for boundary conditions
  }
}

void
WCNSFVTurbulencePhysics::addInitialConditions()
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

  // do not set initial conditions if we are loading from file
  if (getParam<bool>("initialize_variables_from_mesh_file"))
    return;
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
    if (!_app.isRestarting() || parameters().isParamSetByUser("initial_mu_t"))
      getProblem().addInitialCondition(ic_type, prefix() + "initial_mu_turb", params);
  }
  else if (isParamSetByUser("initial_mu_t"))
    paramError("initial_mu_t",
               "This parameter can only be specified if 'mu_t_as_aux_variable=true'");

  params.set<VariableName>("variable") = _tke_name;
  params.set<FunctionName>("function") = getParam<FunctionName>("initial_tke");
  if (!_app.isRestarting() || parameters().isParamSetByUser("initial_tke"))
    getProblem().addInitialCondition(ic_type, prefix() + "initial_tke", params);
  params.set<VariableName>("variable") = _tked_name;
  params.set<FunctionName>("function") = getParam<FunctionName>("initial_tked");
  if (!_app.isRestarting() || parameters().isParamSetByUser("initial_tked"))
    getProblem().addInitialCondition(ic_type, prefix() + "initial_tked", params);
}

void
WCNSFVTurbulencePhysics::addMaterials()
{
  if (_turbulence_model == "mixing-length")
  {
    const std::string u_names[3] = {"u", "v", "w"};
    InputParameters params =
        getFactory().getValidParams("MixingLengthTurbulentViscosityFunctorMaterial");
    assignBlocks(params, _blocks);

    for (const auto d : make_range(dimension()))
      params.set<MooseFunctorName>(u_names[d]) = _velocity_names[d];

    params.set<MooseFunctorName>(NS::mixing_length) = _mixing_length_name;
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;

    getProblem().addMaterial("MixingLengthTurbulentViscosityFunctorMaterial",
                             prefix() + "mixing_length_material",
                             params);
  }
  else if (_turbulence_model == "k-epsilon")
  {
    if (!getProblem().hasFunctor(NS::mu_eff, /*thread_id=*/0))
    {
      InputParameters params = getFactory().getValidParams("ADParsedFunctorMaterial");
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
      getProblem().addMaterial("ADParsedFunctorMaterial", prefix() + "effective_viscosity", params);
    }
    if (!getParam<bool>("mu_t_as_aux_variable"))
    {
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
      mooseAssert(!getParam<bool>("k_t_as_aux_variable"), "k_t should exist");
      InputParameters params = getFactory().getValidParams("ADParsedFunctorMaterial");
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
      getProblem().addMaterial(
          "ADParsedFunctorMaterial", prefix() + "turbulent_heat_eff_conductivity", params);
    }

    if (_has_scalar_equations && !getProblem().hasFunctor(NS::mu_t_passive_scalar, /*thread_id=*/0))
    {
      InputParameters params = getFactory().getValidParams("ADParsedFunctorMaterial");
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
        paramError(
            "passive_scalar_schmidt_number",
            "Only one passive scalar turbulent Schmidt number can be specified with k-epsilon");
      params.set<std::string>("expression") = _turbulent_viscosity_name + "/" + rho_name + " / " +
                                              std::to_string(turbulent_schmidt_number[0]);
      params.set<std::string>("property_name") = NS::mu_t_passive_scalar;
      getProblem().addMaterial(
          "ADParsedFunctorMaterial", prefix() + "scalar_turbulent_diffusivity", params);
    }
  }
}

unsigned short
WCNSFVTurbulencePhysics::getNumberAlgebraicGhostingLayersNeeded() const
{
  unsigned short ghost_layers = _flow_equations_physics->getNumberAlgebraicGhostingLayersNeeded();
  // due to the computation of the eddy-diffusivity from the strain tensor
  if (_turbulence_model == "mixing-length")
    ghost_layers = std::max(ghost_layers, (unsigned short)3);
  return ghost_layers;
}
