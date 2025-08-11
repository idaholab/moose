//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSLinearFVTurbulencePhysics.h"
#include "WCNSFVFlowPhysics.h"
#include "WCNSFVFluidHeatTransferPhysics.h"
#include "WCNSFVScalarTransportPhysics.h"
#include "WCNSFVCoupledAdvectionPhysicsHelper.h"
#include "INSFVTurbulentViscosityWallFunction.h"
#include "INSFVTKESourceSink.h"
#include "NSFVBase.h"

registerWCNSFVTurbulenceBaseTasks("NavierStokesApp", WCNSLinearFVTurbulencePhysics);

InputParameters
WCNSLinearFVTurbulencePhysics::validParams()
{
  InputParameters params = WCNSFVTurbulencePhysicsBase::validParams();
  params.addClassDescription(
      "Define a turbulence model for a incompressible or weakly-compressible Navier Stokes "
      "flow with a linear finite volume discretization");

  // Not an option
  params.addParam<bool>("mu_t_as_aux_variable",
                        true,
                        "Whether to use an auxiliary variable instead of a functor material "
                        "property for the turbulent viscosity");
  params.suppressParameter<bool>("mu_t_as_aux_variable");
  return params;
}

WCNSLinearFVTurbulencePhysics::WCNSLinearFVTurbulencePhysics(const InputParameters & parameters)
  : WCNSFVTurbulencePhysicsBase(parameters)
{
}

void
WCNSLinearFVTurbulencePhysics::initializePhysicsAdditional()
{
  if (_turbulence_model == "k-epsilon")
    getProblem().needSolutionState(1, Moose::SolutionIterationType::Nonlinear);
}

void
WCNSLinearFVTurbulencePhysics::addSolverVariables()
{
  if (_turbulence_model == "mixing-length" || _turbulence_model == "none")
    return;
  else if (_turbulence_model == "k-epsilon")
  {
    // Dont add if the user already defined the variable
    // Add turbulent kinetic energy variable
    if (!shouldCreateVariable(_tke_name, _blocks, /*error if aux*/ true))
      reportPotentiallyMissedParameters({"system_names"}, "INSFVEnergyVariable");
    else if (_define_variables)
    {
      std::string variable_type = "MooseLinearVariableFVReal";

      auto params = getFactory().getValidParams(variable_type);
      assignBlocks(params, _blocks);
      params.set<SolverSystemName>("solver_sys") = getSolverSystem(_tke_name);

      getProblem().addVariable(variable_type, _tke_name, params);
    }
    else
      paramError("turbulence_kinetic_energy_variable",
                 "Variable (" + _tke_name +
                     ") supplied to the WCNSLinearFVTurbulencePhysics does not exist!");

    // Add turbulent kinetic energy dissipation variable
    if (!shouldCreateVariable(_tked_name, _blocks, /*error if aux*/ true))
      reportPotentiallyMissedParameters({"system_names"}, "INSFVEnergyVariable");
    else if (_define_variables)
    {
      std::string variable_type = "MooseLinearVariableFVReal";

      auto params = getFactory().getValidParams(variable_type);
      assignBlocks(params, _blocks);
      params.set<SolverSystemName>("solver_sys") = getSolverSystem(_tked_name);

      getProblem().addVariable(variable_type, _tked_name, params);
    }
    else
      paramError("turbulence_kinetic_energy_dissipation_variable",
                 "Variable (" + _tked_name +
                     ") supplied to the WCNSLinearFVTurbulencePhysics does not exist!");
  }
}

void
WCNSLinearFVTurbulencePhysics::addFVKernels()
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
WCNSLinearFVTurbulencePhysics::addFlowTurbulenceKernels()
{
  if (_turbulence_model == "k-epsilon")
  {
    const std::string u_names[3] = {"u", "v", "w"};
    std::string kernel_type = "LinearWCNSFVMomentumFlux";
    std::string kernel_name = prefix() + "ins_momentum_turbulent_flux_";

    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<MooseFunctorName>(NS::mu) = _flow_equations_physics->dynamicViscosityName();
    params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
    params.set<MooseEnum>("advected_interp_method") =
        _flow_equations_physics->getMomentumAdvectionFaceInterpolationMethod();
    params.set<bool>("use_nonorthogonal_correction") =
        _flow_equations_physics->getParam<bool>("use_nonorthogonal_correction");
    params.set<bool>("use_deviatoric_terms") =
        _flow_equations_physics->getParam<bool>("include_deviatoric_stress");
    params.set<bool>("add_advective_term") = false;

    for (unsigned int i = 0; i < dimension(); ++i)
      params.set<SolverVariableName>(u_names[i]) = _velocity_names[i];

    for (const auto d : make_range(dimension()))
    {
      params.set<LinearVariableName>("variable") = _velocity_names[d];
      params.set<MooseEnum>("momentum_component") = NS::directions[d];

      getProblem().addLinearFVKernel(kernel_type, kernel_name + NS::directions[d], params);
    }
    if (!_flow_equations_physics->getParam<bool>("use_deviatoric_term"))
      _flow_equations_physics->paramWarning(
          "use_deviatoric_term", "This should be set to true when using a turbulence model");
  }
}

void
WCNSLinearFVTurbulencePhysics::addFluidEnergyTurbulenceKernels()
{
  if (_turbulence_model == "k-epsilon")
  {
    const std::string kernel_type = "LinearFVDiffusion";
    const auto T_fluid_name = _flow_equations_physics->getFluidTemperatureName();
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    if (!_fluid_energy_physics->getParam<bool>("solve_for_enthalpy"))
    {
      params.set<NonlinearVariableName>("variable") = T_fluid_name;
      params.set<MooseFunctorName>("diffusion_coeff") = NS::k_t;
    }
    else
    {
      params.set<NonlinearVariableName>("variable") =
          _fluid_energy_physics->getSpecificEnthalpyName();
      params.set<MooseFunctorName>("diffusion_coeff") = NS::k_t + "_by_cp";
    }
    params.set<bool>("use_nonorthogonal_correction") =
        _fluid_energy_physics->getParam<bool>("use_nonorthogonal_correction");
    getProblem().addLinearFVKernel(
        kernel_type, prefix() + T_fluid_name + "_turbulent_diffusion", params);
  }
}

void
WCNSLinearFVTurbulencePhysics::addScalarAdvectionTurbulenceKernels()
{
  const auto & passive_scalar_names = _scalar_transport_physics->getAdvectedScalarNames();
  const auto & passive_scalar_schmidt_number = getParam<std::vector<Real>>("Sc_t");
  if (passive_scalar_schmidt_number.size() != passive_scalar_names.size() &&
      passive_scalar_schmidt_number.size() != 1)
    paramError(
        "Sc_t",
        "The number of turbulent Schmidt numbers defined is not equal to the number of passive "
        "scalar fields!");

  if (_turbulence_model == "k-epsilon")
  {
    const std::string kernel_type = "LinearFVDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<bool>("use_nonorthogonal_correction") =
        _scalar_transport_physics->getParam<bool>("use_nonorthogonal_correction");

    for (const auto & name_i : index_range(passive_scalar_names))
    {
      params.set<NonlinearVariableName>("variable") = passive_scalar_names[name_i];
      params.set<MooseFunctorName>("diffusion_coeff") = NS::mu_t_passive_scalar;
      getProblem().addLinearFVKernel(
          kernel_type, prefix() + passive_scalar_names[name_i] + "_turbulent_diffusion", params);
    }
  }
}

void
WCNSLinearFVTurbulencePhysics::addKEpsilonTimeDerivatives()
{
  const std::string kernel_type = "LinearFVTimeDerivative";
  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);

  params.set<NonlinearVariableName>("variable") = _tke_name;
  if (shouldCreateTimeDerivative(_tke_name, _blocks, /*error if already defined*/ false))
    getProblem().addLinearFVKernel(kernel_type, prefix() + "tke_time", params);
  params.set<NonlinearVariableName>("variable") = _tked_name;
  if (shouldCreateTimeDerivative(_tked_name, _blocks, /*error if already defined*/ false))
    getProblem().addLinearFVKernel(kernel_type, prefix() + "tked_time", params);
}

void
WCNSLinearFVTurbulencePhysics::addKEpsilonAdvection()
{
  const std::string kernel_type = "LinearFVTurbulentAdvection";
  InputParameters params = getFactory().getValidParams(kernel_type);

  assignBlocks(params, _blocks);

  params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
  params.set<MooseEnum>("advected_interp_method") =
      getParam<MooseEnum>("tke_advection_interpolation");
  params.set<NonlinearVariableName>("variable") = _tke_name;
  getProblem().addLinearFVKernel(kernel_type, prefix() + "tke_advection", params);
  params.set<NonlinearVariableName>("variable") = _tked_name;
  params.set<std::vector<BoundaryName>>("walls") = _turbulence_walls;
  params.set<MooseEnum>("advected_interp_method") =
      getParam<MooseEnum>("tked_advection_interpolation");
  getProblem().addLinearFVKernel(kernel_type, prefix() + "tked_advection", params);
}

void
WCNSLinearFVTurbulencePhysics::addKEpsilonDiffusion()
{
  {
    const std::string kernel_type = "LinearFVTurbulentDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<bool>("use_nonorthogonal_correction") =
        getParam<bool>("use_nonorthogonal_correction");

    params.set<NonlinearVariableName>("variable") = _tke_name;
    params.set<MooseFunctorName>("diffusion_coeff") =
        _flow_equations_physics->dynamicViscosityName();
    getProblem().addLinearFVKernel(kernel_type, prefix() + "tke_diffusion_mu", params);

    params.set<std::vector<BoundaryName>>("walls") = _turbulence_walls;
    params.set<NonlinearVariableName>("variable") = _tked_name;
    getProblem().addLinearFVKernel(kernel_type, prefix() + "tked_diffusion_mu", params);
  }

  {
    const std::string kernel_type = "LinearFVTurbulentDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<bool>("use_nonorthogonal_correction") =
        getParam<bool>("use_nonorthogonal_correction");

    params.set<NonlinearVariableName>("variable") = _tke_name;
    params.set<MooseFunctorName>("diffusion_coeff") = _turbulent_viscosity_name;
    params.set<MooseFunctorName>("scaling_coeff") = getParam<MooseFunctorName>("sigma_k");
    params.set<MooseEnum>("coeff_interp_method") =
        getParam<MooseEnum>("turbulent_viscosity_interp_method");
    getProblem().addLinearFVKernel(kernel_type, prefix() + "tke_diffusion_mu_turb", params);

    params.set<std::vector<BoundaryName>>("walls") = _turbulence_walls;
    params.set<NonlinearVariableName>("variable") = _tked_name;
    params.set<MooseFunctorName>("scaling_coeff") = getParam<MooseFunctorName>("sigma_eps");
    getProblem().addLinearFVKernel(kernel_type, prefix() + "tked_diffusion_mu_turb", params);
  }
}

void
WCNSLinearFVTurbulencePhysics::addKEpsilonSink()
{
  const std::string u_names[3] = {"u", "v", "w"};
  {
    const std::string kernel_type = "LinearFVTKESourceSink";
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
    // Currently only Newton method for WCNSLinearFVTurbulencePhysics
    params.set<bool>("newton_solve") = true;
    for (const auto d : make_range(dimension()))
      params.set<MooseFunctorName>(u_names[d]) = _velocity_names[d];
    getProblem().addLinearFVKernel(kernel_type, prefix() + "tke_source_sink", params);
  }

  {
    const std::string kernel_type = "LinearFVTKEDSourceSink";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<NonlinearVariableName>("variable") = _tked_name;
    params.set<MooseFunctorName>(NS::TKE) = _tke_name;
    params.set<MooseFunctorName>(NS::density) = _flow_equations_physics->densityName();
    params.set<MooseFunctorName>(NS::mu) = _flow_equations_physics->dynamicViscosityName();
    params.set<MooseFunctorName>(NS::mu_t) = _turbulent_viscosity_name;
    params.set<bool>("linearized_model") = getParam<bool>("linearize_sink_sources");
    params.set<std::vector<BoundaryName>>("walls") = _turbulence_walls;
    params.set<MooseEnum>("wall_treatment") = _wall_treatment_eps;
    params.set<Real>("C1_eps") = getParam<Real>("C1_eps");
    params.set<Real>("C2_eps") = getParam<Real>("C2_eps");
    params.set<Real>("C_mu") = getParam<Real>("C_mu");
    params.set<Real>("C_pl") = getParam<Real>("C_pl");
    // Currently only Newton method for WCNSLinearFVTurbulencePhysics
    params.set<bool>("newton_solve") = true;
    for (const auto d : make_range(dimension()))
      params.set<MooseFunctorName>(u_names[d]) = _velocity_names[d];
    getProblem().addLinearFVKernel(kernel_type, prefix() + "tked_source_sink", params);
  }
}

void
WCNSLinearFVTurbulencePhysics::addFVBCs()
{
  const std::string u_names[3] = {"u", "v", "w"};

  if (_turbulence_model == "k-epsilon" && getParam<bool>("mu_t_as_aux_variable"))
  {
    mooseAssert(_flow_equations_physics, "Should have a flow equation physics");
    const std::string bc_type = "LinearFVTurbulentViscosityWallFunction";
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

    getProblem().addLinearFVBC(bc_type, prefix() + "turbulence_walls", params);
    // Energy wall function boundary conditions are added in the WCNSFVFluidEnergyPhysics
    // because it facilitates counting the number of walls, specifying energy wall functors
    // the same way as for boundary conditions
  }
}

unsigned short
WCNSLinearFVTurbulencePhysics::getNumberAlgebraicGhostingLayersNeeded() const
{
  return _flow_equations_physics->getNumberAlgebraicGhostingLayersNeeded();
}
