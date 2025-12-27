//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "MooseMesh.h"

registerWCNSFVTurbulenceBaseTasks("NavierStokesApp", WCNSFVTurbulencePhysics);

InputParameters
WCNSFVTurbulencePhysics::validParams()
{
  InputParameters params = WCNSFVTurbulencePhysicsBase::validParams();
  params.addClassDescription(
      "Define a turbulence model for a incompressible or weakly-compressible Navier Stokes "
      "flow with a finite volume discretization");

  // TODO Added to facilitate transition, remove default once NavierStokesFV action is removed
  params.addParam<AuxVariableName>(
      "mixing_length_name", "mixing_length", "Name of the mixing length auxiliary variable");
  params.transferParam<bool>(NSFVBase::validParams(), "mixing_length_two_term_bc_expansion");

  // K-Epsilon numerical scheme parameters
  params.addRangeCheckedParam<Real>(
      "tke_scaling",
      1.0,
      "tke_scaling > 0.0",
      "The scaling factor for the turbulent kinetic energy equation.");
  params.addRangeCheckedParam<Real>(
      "tked_scaling",
      1.0,
      "tked_scaling > 0.0",
      "The scaling factor for the turbulent kinetic energy dissipation equation.");

  // Better Jacobian if not linearizing sink and sources
  params.addParam<bool>("linearize_sink_sources", false, "Whether to linearize the source term");
  // Better convergence on some cases when neglecting advection derivatives
  params.addParam<bool>(
      "neglect_advection_derivatives",
      false,
      "Whether to remove the off-diagonal velocity term in the TKE and TKED advection term");
  MooseEnum coeff_interp_method("average harmonic", "harmonic");
  params.addParam<MooseEnum>("turbulent_viscosity_interp_method",
                             coeff_interp_method,
                             "Face interpolation method for the turbulent viscosity");

  params.addParamNamesToGroup("tke_scaling tked_scaling "
                              "turbulent_viscosity_interp_method linearize_sink_sources",
                              "K-Epsilon model numerical");

  return params;
}

WCNSFVTurbulencePhysics::WCNSFVTurbulencePhysics(const InputParameters & parameters)
  : WCNSFVTurbulencePhysicsBase(parameters),
    _mixing_length_name(getParam<AuxVariableName>("mixing_length_name"))
{
  // Keep track of the variable names, for loading variables from files notably
  if (_turbulence_model == "mixing-length")
    saveAuxVariableName(_mixing_length_name);

  // Parameter checks
  if (_turbulence_model != "mixing-length")
    errorDependentParameter("turbulence_handling",
                            "mixing-length",
                            {"mixing_length_delta",
                             "mixing_length_aux_execute_on",
                             "von_karman_const",
                             "von_karman_const_0",
                             "mixing_length_two_term_bc_expansion"});
}

void
WCNSFVTurbulencePhysics::initializePhysicsAdditional()
{
  if (_turbulence_model == "k-epsilon")
    getProblem().needSolutionState(2, Moose::SolutionIterationType::Nonlinear);
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
    if (!shouldCreateVariable(_tke_name, _blocks, /*error if aux*/ true))
      reportPotentiallyMissedParameters(
          {"system_names", "tke_scaling", "tke_face_interpolation", "tke_two_term_bc_expansion"},
          "INSFVEnergyVariable");
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
    if (!shouldCreateVariable(_tked_name, _blocks, /*error if aux*/ true))
      reportPotentiallyMissedParameters(
          {"system_names", "tked_scaling", "tked_face_interpolation", "tked_two_term_bc_expansion"},
          "INSFVEnergyVariable");
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
    if (!shouldCreateVariable(_tke_name, _blocks, /*error if aux*/ false))
      reportPotentiallyMissedParameters({"mixing_length_two_term_bc_expansion"},
                                        "MooseVariableFVReal");
    else
      getProblem().addAuxVariable("MooseVariableFVReal", _mixing_length_name, params);
  }
  WCNSFVTurbulencePhysicsBase::addAuxiliaryVariables();
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
    params.set<MooseEnum>("variable_interp_method") =
        _flow_equations_physics->getMomentumFaceInterpolationMethod();
    params.set<bool>("complete_expansion") = true;
    if (_flow_equations_physics->includeIsotropicStress())
      params.set<bool>("include_isotropic_viscous_stress") = true;

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

    // We only add it here because the mixing length kernels deal with this
    // withn the kernel. For mixing length see issue: #32112
    if (_flow_equations_physics && _flow_equations_physics->hasFlowEquations())
      addAxisymmetricTurbulentViscousSource();
  }
}

void
WCNSFVTurbulencePhysics::addAxisymmetricTurbulentViscousSource()
{
  if (_turbulence_model == "none")
    return;
  if (!_flow_equations_physics)
    return;
  if (!_flow_equations_physics->addAxisymmetricViscousSourceEnabled())
    return;

  const auto rz_blocks = _flow_equations_physics->getAxisymmetricRZBlocks();
  if (rz_blocks.empty())
    return;

  const auto radial_index =
      _flow_equations_physics->getProblem().mesh().getAxisymmetricRadialCoord();

  InputParameters params = getFactory().getValidParams("INSFVMomentumViscousSourceRZ");
  assignBlocks(params, rz_blocks);
  params.set<MooseFunctorName>(NS::mu) = _turbulent_viscosity_name;
  params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
  params.set<MooseEnum>("momentum_component") = NS::directions[radial_index];
  params.set<bool>("complete_expansion") =
      _flow_equations_physics->includeSymmetrizedViscousStress();
  params.set<NonlinearVariableName>("variable") =
      _flow_equations_physics->getVelocityNames()[radial_index];

  getProblem().addFVKernel("INSFVMomentumViscousSourceRZ",
                           prefix() + "ins_momentum_turbulent_viscous_source_rz_" +
                               NS::directions[radial_index],
                           params);
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
  if (shouldCreateTimeDerivative(_tke_name, _blocks, /*error if already defined*/ false))
    getProblem().addFVKernel(kernel_type, prefix() + "tke_time", params);
  params.set<NonlinearVariableName>("variable") = _tked_name;
  if (shouldCreateTimeDerivative(_tked_name, _blocks, /*error if already defined*/ false))
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
    params.set<Real>("C_mu") = getParam<Real>("C_mu");
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
    params.set<Real>("C_mu") = getParam<Real>("C_mu");
    params.set<Real>("C_pl") = getParam<Real>("C_pl");
    params.set<bool>("linearized_model") = getParam<bool>("linearize_sink_sources");
    params.set<std::vector<BoundaryName>>("walls") = _turbulence_walls;
    params.set<MooseEnum>("wall_treatment") = _wall_treatment_eps;
    params.set<MooseFunctorName>("C1_eps") = getParam<MooseFunctorName>("C1_eps");
    params.set<MooseFunctorName>("C2_eps") = getParam<MooseFunctorName>("C2_eps");
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
  WCNSFVTurbulencePhysicsBase::addAuxiliaryKernels();

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
    params.set<Real>("C_mu") = getParam<Real>("C_mu");
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
  WCNSFVTurbulencePhysicsBase::addMaterials();
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
