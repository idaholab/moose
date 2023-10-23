//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVTurbulencePhysics.h"
#include "NSFVAction.h"
#include "WCNSFVHeatAdvectionPhysics.h"
#include "WCNSFVScalarAdvectionPhysics.h"

registerMooseAction("NavierStokesApp", WCNSFVTurbulencePhysics, "add_variable");
registerMooseAction("NavierStokesApp", WCNSFVTurbulencePhysics, "add_fv_kernel");
registerMooseAction("NavierStokesApp", WCNSFVTurbulencePhysics, "add_aux_variable");
registerMooseAction("NavierStokesApp", WCNSFVTurbulencePhysics, "add_aux_kernel");
registerMooseAction("NavierStokesApp", WCNSFVTurbulencePhysics, "add_material");

InputParameters
WCNSFVTurbulencePhysics::validParams()
{
  InputParameters params = WCNSFVPhysicsBase::validParams();
  params.addClassDescription("Define a turbulence model for a weakly-compressible Navier Stokes "
                             "flow with a finite volume discretization");

  MooseEnum turbulence_type("mixing-length none", "none");
  params.addParam<MooseEnum>(
      "turbulence_model",
      turbulence_type,
      "The way turbulent diffusivities are determined in the turbulent regime.");
  params += NSFVAction::commonTurbulenceParams();

  // Add the coupled physics
  params.addParam<PhysicsName>("coupled_flow_physics",
                               "WCNSFVFlowPhysics generating the Navier Stokes flow equations");
  params.addParam<PhysicsName>(
      "heat_advection_physics",
      "WCNSFVHeatAdvectionPhysics generating the heat advection equations");
  params.addParam<PhysicsName>(
      "scalar_advection_physics",
      "WCNSFVScalarAdvectionPhysics generating the scalar advection equations");

  return params;
}

WCNSFVTurbulencePhysics::WCNSFVTurbulencePhysics(const InputParameters & parameters)
  : WCNSFVPhysicsBase(parameters), _turbulence_model(getParam<MooseEnum>("turbulence_model"))
{
  if (isParamValid("heat_advection_physics"))
  {
    _fluid_energy_physics = getCoupledPhysics<WCNSFVHeatAdvectionPhysics>(
        getParam<PhysicsName>("heat_advection_physics"));
    checkCommonParametersConsistent(_fluid_energy_physics->parameters());
  }
  else
    _fluid_energy_physics = nullptr;
  if (isParamValid("scalar_advection_physics"))
  {
    _scalar_advection_physics = getCoupledPhysics<WCNSFVScalarAdvectionPhysics>(
        getParam<PhysicsName>("scalar_advection_physics"));
    checkCommonParametersConsistent(_scalar_advection_physics->parameters());
  }
  else
    _scalar_advection_physics = nullptr;

  // Parameter checks
  if (_turbulence_model != "mixing-length")
    errorDependentParameter("turbulence_handling",
                            "mixing-length",
                            {"mixing_length_delta",
                             "mixing_length_aux_execute_on",
                             "mixing_length_walls",
                             "von_karman_const",
                             "von_karman_const_0"});
}

void
WCNSFVTurbulencePhysics::addNonlinearVariables()
{
  if (_turbulence_model == "mixing-length")
    return;
}

void
WCNSFVTurbulencePhysics::addAuxiliaryVariables()
{
  if (_turbulence_model == "mixing-length")
  {
    auto params = getFactory().getValidParams("MooseVariableFVReal");
    assignBlocks(params, _blocks);
    params.set<bool>("two_term_boundary_expansion") =
        getParam<bool>("mixing_length_two_term_bc_expansion");

    getProblem().addVariable("MooseVariableFVReal", NS::mixing_length, params);
    // addNSAuxVariable("MooseVariableFVReal", NS::mixing_length, params);
  }
}

void
WCNSFVTurbulencePhysics::addFVKernels()
{
  if (_flow_equations_physics)
    addFlowTurbulenceKernels();
  if (_fluid_energy_physics)
    addFluidEnergyTurbulenceKernels();
  if (_scalar_advection_physics)
    addScalarAdvectionTurbulenceKernels();
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
    params.set<MooseFunctorName>(NS::mixing_length) = NS::mixing_length;

    std::string kernel_name = prefix() + "ins_momentum_mixing_length_reynolds_stress_";
    if (_porous_medium_treatment)
      kernel_name = prefix() + "pins_momentum_mixing_length_reynolds_stress_";

    params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
    for (unsigned int dim_i = 0; dim_i < dimension(); ++dim_i)
      params.set<MooseFunctorName>(u_names[dim_i]) = _velocity_names[dim_i];

    for (unsigned int d = 0; d < dimension(); ++d)
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
    params.set<MooseFunctorName>(NS::specific_enthalpy) =
        _fluid_energy_physics->getSpecificEnthalpyName();
    params.set<MooseFunctorName>(NS::mixing_length) = NS::mixing_length;
    params.set<Real>("schmidt_number") = getParam<Real>("turbulent_prandtl");
    params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;

    for (unsigned int dim_i = 0; dim_i < dimension(); ++dim_i)
      params.set<MooseFunctorName>(u_names[dim_i]) = _velocity_names[dim_i];

    if (_porous_medium_treatment)
      getProblem().addFVKernel(
          kernel_type, prefix() + "pins_energy_mixing_length_diffusion", params);
    else
      getProblem().addFVKernel(
          kernel_type, prefix() + "ins_energy_mixing_length_diffusion", params);
  }
}

void
WCNSFVTurbulencePhysics::addScalarAdvectionTurbulenceKernels()
{
  if (_turbulence_model == "mixing-length")
  {
    const std::string u_names[3] = {"u", "v", "w"};
    const std::string kernel_type = "INSFVMixingLengthScalarDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<MooseFunctorName>(NS::mixing_length) = NS::mixing_length;

    for (unsigned int dim_i = 0; dim_i < dimension(); ++dim_i)
      params.set<MooseFunctorName>(u_names[dim_i]) = _velocity_names[dim_i];

    const auto & passive_scalar_names = _scalar_advection_physics->getAdvectedScalarNames();
    const auto & passive_scalar_schmidt_number =
        getParam<std::vector<Real>>("passive_scalar_schmidt_number");

    for (const auto & name_i : index_range(passive_scalar_names))
    {
      params.set<NonlinearVariableName>("variable") = passive_scalar_names[name_i];
      if (passive_scalar_schmidt_number.size())
        params.set<Real>("schmidt_number") = passive_scalar_schmidt_number[name_i];
      else
        params.set<Real>("schmidt_number") = 1.0;

      getProblem().addFVKernel(
          kernel_type, prefix() + passive_scalar_names[name_i] + "_mixing_length", params);
    }
  }
}

void
WCNSFVTurbulencePhysics::addAuxiliaryKernels()
{
  if (_turbulence_model == "mixing-length")
  {
    const std::string ml_kernel_type = "WallDistanceMixingLengthAux";
    InputParameters ml_params = getFactory().getValidParams(ml_kernel_type);
    assignBlocks(ml_params, _blocks);
    ml_params.set<AuxVariableName>("variable") = NS::mixing_length;
    ml_params.set<std::vector<BoundaryName>>("walls") =
        getParam<std::vector<BoundaryName>>("mixing_length_walls");
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
WCNSFVTurbulencePhysics::addMaterials()
{
  if (_turbulence_model == "mixing-length")
  {
    const std::string u_names[3] = {"u", "v", "w"};
    InputParameters params = getFactory().getValidParams("MixingLengthTurbulentViscosityMaterial");
    assignBlocks(params, _blocks);

    for (unsigned int d = 0; d < dimension(); ++d)
      params.set<MooseFunctorName>(u_names[d]) = _velocity_names[d];

    params.set<MooseFunctorName>(NS::mixing_length) = NS::mixing_length;
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;

    getProblem().addMaterial(
        "MixingLengthTurbulentViscosityMaterial", prefix() + "mixing_length_material", params);
  }
}
