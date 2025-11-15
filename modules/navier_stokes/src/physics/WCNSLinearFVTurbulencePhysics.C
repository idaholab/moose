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
registerMooseAction("NavierStokesApp", WCNSLinearFVTurbulencePhysics, "add_functor_material");

InputParameters
WCNSLinearFVTurbulencePhysics::validParams()
{
  InputParameters params = WCNSFVTurbulencePhysicsBase::validParams();
  params.addClassDescription(
      "Define a turbulence model for an incompressible or weakly-compressible Navier Stokes "
      "flow with a linear finite volume discretization");

  params.addParam<std::vector<SolverSystemName>>(
      "system_names",
      {"TKE_system", "TKED_system"},
      "Names of the linear solver systems for each equation. Default is set for K-Epsilon and "
      "should be adapted for other models");

  // Not an option
  params.addParam<bool>("mu_t_as_aux_variable",
                        true,
                        "Whether to use an auxiliary variable instead of a functor material "
                        "property for the turbulent viscosity");
  params.suppressParameter<bool>("mu_t_as_aux_variable");
  params.suppressParameter<bool>("turbulent_viscosity_two_term_bc_expansion");

  // Could be implemented when boundary conditions are implemented in turbulence physics
  params.suppressParameter<bool>("tke_two_term_bc_expansion");
  params.suppressParameter<bool>("tked_two_term_bc_expansion");

  // Not implemented
  params.suppressParameter<MooseEnum>("wall_treatment_T");
  params.suppressParameter<MooseEnum>("tke_face_interpolation");
  params.suppressParameter<MooseEnum>("tked_face_interpolation");

  // LinearFV-specific parameters
  params.addParam<bool>(
      "use_nonorthogonal_correction",
      true,
      "Whether to use a non-orthogonal correction. This can potentially slow down convergence "
      ", but reduces numerical dispersion on non-orthogonal meshes. Can be safely turned off on "
      "orthogonal meshes.");
  params.addParamNamesToGroup("use_nonorthogonal_correction", "Numerical scheme");
  return params;
}

WCNSLinearFVTurbulencePhysics::WCNSLinearFVTurbulencePhysics(const InputParameters & parameters)
  : WCNSFVTurbulencePhysicsBase(parameters)
{
  if (_turbulence_model != "k-epsilon")
    errorDependentParameter("turbulence_handling", "k-epsilon", {"use_nonorthogonal_correction"});
  if (_turbulence_model == "mixing-length")
    paramError("turbulence_handling",
               "Mixing length is not implemented for the linear finite volume discretization");
}

void
WCNSLinearFVTurbulencePhysics::initializePhysicsAdditional()
{
  if (_turbulence_model == "k-epsilon")
    getProblem().needSolutionState(1, Moose::SolutionIterationType::Nonlinear);
}

void
WCNSLinearFVTurbulencePhysics::checkIntegrity() const
{
  WCNSFVTurbulencePhysicsBase::checkIntegrity();

  if (_flow_equations_physics &&
      !_flow_equations_physics->getParam<bool>("include_deviatoric_stress"))
    _flow_equations_physics->paramWarning(
        "include_deviatoric_stress", "This should be set to true when using a turbulence model");
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
      reportPotentiallyMissedParameters({"system_names"}, "MooseLinearVariableFVReal");
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
      reportPotentiallyMissedParameters({"system_names"}, "MooseLinearVariableFVReal");
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

  // For linear FV discretization:
  // We have to add the kernel in the flow/heat/scalar physics with mu_eff instead of two kernels
  // one with mu, one with mu_turb, and chose one of the two to NOT have the advective term
  // Also, flux boundary conditions would be executed twice with two kernels

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
WCNSLinearFVTurbulencePhysics::addKEpsilonTimeDerivatives()
{
  const std::string kernel_type = "LinearFVTimeDerivative";
  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);

  params.set<LinearVariableName>("variable") = _tke_name;
  params.set<MooseFunctorName>("factor") = _flow_equations_physics->densityName();
  if (shouldCreateTimeDerivative(_tke_name, _blocks, /*error if already defined*/ false))
    getProblem().addLinearFVKernel(kernel_type, prefix() + "tke_time", params);
  params.set<LinearVariableName>("variable") = _tked_name;
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
  params.set<LinearVariableName>("variable") = _tke_name;
  getProblem().addLinearFVKernel(kernel_type, prefix() + "tke_advection", params);
  params.set<LinearVariableName>("variable") = _tked_name;
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

    // Note: we have to use a single diffusion kernel in case we have a flux BC so it is not applied
    // twice
    params.set<LinearVariableName>("variable") = _tke_name;
    params.set<MooseFunctorName>("diffusion_coeff") = "mu_eff_tke";
    getProblem().addLinearFVKernel(kernel_type, prefix() + "tke_diffusion_mu", params);

    params.set<std::vector<BoundaryName>>("walls") = _turbulence_walls;
    params.set<LinearVariableName>("variable") = _tked_name;
    params.set<MooseFunctorName>("diffusion_coeff") = "mu_eff_tked";
    getProblem().addLinearFVKernel(kernel_type, prefix() + "tked_diffusion_mu", params);
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
    params.set<LinearVariableName>("variable") = _tke_name;
    params.set<MooseFunctorName>(NS::TKED) = _tked_name;
    params.set<MooseFunctorName>(NS::density) = _flow_equations_physics->densityName();
    params.set<MooseFunctorName>(NS::mu) = _flow_equations_physics->dynamicViscosityName();
    params.set<MooseFunctorName>(NS::mu_t) = _turbulent_viscosity_name;
    params.set<Real>("C_mu") = getParam<Real>("C_mu");
    params.set<Real>("C_pl") = getParam<Real>("C_pl");
    params.set<std::vector<BoundaryName>>("walls") = _turbulence_walls;
    params.set<MooseEnum>("wall_treatment") = _wall_treatment_eps;
    for (const auto d : make_range(dimension()))
      params.set<MooseFunctorName>(u_names[d]) = _velocity_names[d];
    getProblem().addLinearFVKernel(kernel_type, prefix() + "tke_source_sink", params);
  }

  {
    const std::string kernel_type = "LinearFVTKEDSourceSink";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") = _tked_name;
    params.set<MooseFunctorName>(NS::TKE) = _tke_name;
    params.set<MooseFunctorName>(NS::density) = _flow_equations_physics->densityName();
    params.set<MooseFunctorName>(NS::mu) = _flow_equations_physics->dynamicViscosityName();
    params.set<MooseFunctorName>(NS::mu_t) = _turbulent_viscosity_name;
    params.set<std::vector<BoundaryName>>("walls") = _turbulence_walls;
    params.set<MooseEnum>("wall_treatment") = _wall_treatment_eps;
    params.set<Real>("C1_eps") = getParam<Real>("C1_eps");
    params.set<Real>("C2_eps") = getParam<Real>("C2_eps");
    params.set<Real>("C_mu") = getParam<Real>("C_mu");
    params.set<Real>("C_pl") = getParam<Real>("C_pl");
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
    const std::string bc_type = "LinearFVTurbulentViscosityWallFunctionBC";
    InputParameters params = getFactory().getValidParams(bc_type);
    params.set<std::vector<BoundaryName>>("boundary") = _turbulence_walls;
    params.set<LinearVariableName>("variable") = _turbulent_viscosity_name;
    params.set<MooseFunctorName>(NS::density) = _flow_equations_physics->densityName();
    params.set<MooseFunctorName>(NS::mu) = _flow_equations_physics->dynamicViscosityName();
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

void
WCNSLinearFVTurbulencePhysics::addFunctorMaterials()
{
  // Functor materials for the diffusion coefficients
  if (_turbulence_model == "k-epsilon")
  {
    // Since sigma_k = 1 in the standard k-epsilon, this is often unnecessary
    const std::string mat_type = "FunctorEffectiveDynamicViscosity";
    InputParameters params = getFactory().getValidParams(mat_type);
    assignBlocks(params, _blocks);
    params.set<MooseFunctorName>("property_name") = "mu_eff_tke";
    params.set<MooseFunctorName>(NS::mu) = _flow_equations_physics->dynamicViscosityName();
    params.set<MooseFunctorName>(NS::mu_t) = _turbulent_viscosity_name;
    params.set<MooseFunctorName>(NS::mu_t + "_inverse_factor") =
        getParam<MooseFunctorName>("sigma_k");
    getProblem().addMaterial(mat_type, prefix() + "mu_eff_tke", params);
  }
  if (_turbulence_model == "k-epsilon")
  {
    const std::string mat_type = "FunctorEffectiveDynamicViscosity";
    InputParameters params = getFactory().getValidParams(mat_type);
    assignBlocks(params, _blocks);
    params.set<MooseFunctorName>("property_name") = "mu_eff_tked";
    params.set<MooseFunctorName>(NS::mu) = _flow_equations_physics->dynamicViscosityName();
    params.set<MooseFunctorName>(NS::mu_t) = _turbulent_viscosity_name;
    params.set<MooseFunctorName>(NS::mu_t + "_inverse_factor") =
        getParam<MooseFunctorName>("sigma_eps");
    getProblem().addMaterial(mat_type, prefix() + "mu_eff_tked", params);
  }
}

unsigned short
WCNSLinearFVTurbulencePhysics::getNumberAlgebraicGhostingLayersNeeded() const
{
  return _flow_equations_physics->getNumberAlgebraicGhostingLayersNeeded();
}
