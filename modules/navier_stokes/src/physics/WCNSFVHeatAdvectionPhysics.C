//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVHeatAdvectionPhysics.h"
#include "NSFVAction.h"

InputParameters
WCNSFVHeatAdvectionPhysics::validParams()
{
  InputParameters params = WCNSFVPhysicsBase::validParams();
  params.addClassDescription("Define the Navier Stokes weakly-compressible energy equation");

  params.transferParam<FunctionName>(NSFVAction::validParams(), "initial_temperature");

  // Material properties
  params.transferParam<std::vector<std::vector<SubdomainName>>>(NSFVAction::validParams(),
                                                                "thermal_conductivity_blocks");
  params.transferParam<std::vector<std::vector<MooseFunctorName>>>(NSFVAction::validParams(),
                                                                   "thermal_conductivity");
  params.transferParam<MooseFunctorName>(NSFVAction::validParams(), "specific_heat");
  params.transferParam<bool>(NSFVAction::validParams(), "use_external_enthalpy_material");

  // Boundary conditions
  params.transferParam<MultiMooseEnum>(NSFVAction::validParams(), "energy_inlet_types");
  params.transferParam<MultiMooseEnum>(NSFVAction::validParams(), "energy_inlet_function");
  params.transferParam<MultiMooseEnum>(NSFVAction::validParams(), "energy_wall_types");
  params.transferParam<std::vector<FunctionName>>(NSFVAction::validParams(),
                                                  "energy_wall_function");

  // Ambient convection
  params.transferParam<std::vector<std::vector<SubdomainName>>>(NSFVAction::validParams(),
                                                                "ambient_convection_blocks");
  params.transferParam<std::vector<std::vector<MooseFunctorName>>>(NSFVAction::validParams(),
                                                                   "ambient_convection_alpha");
  params.transferParam<std::vector<std::vector<MooseFunctorName>>>(NSFVAction::validParams(),
                                                                   "ambient_temperature");

  // Heat source directly in the fluid
  params.transferParam<MooseFunctorName>(NSFVAction::validParams(), "external_heat_source");
  params.transferParam<Real>(NSFVAction::validParams(), "external_heat_source_coeff");

  // Spatial finite volume discretization scheme
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "energy_advection_interpolation");
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "energy_face_interpolation");

  // Nonlinear equation solver scaling
  params.transferParam<Real>(NSFVAction::validParams(), "energy_scaling");

  return params;
}

WCNSFVHeatAdvectionPhysics::WCNSFVHeatAdvectionPhysics(const InputParameters & parameters)
  : WCNSFVPhysicsBase(parameters),
    _specific_heat_name(getParam<MooseFunctorName>("specific_heat")),
    _thermal_conductivity_blocks(
        parameters.isParamValid("thermal_conductivity_blocks")
            ? getParam<std::vector<std::vector<SubdomainName>>>("thermal_conductivity_blocks")
            : std::vector<std::vector<SubdomainName>>()),
    _thermal_conductivity_name(getParam<std::vector<MooseFunctorName>>("thermal_conductivity"))
{
}

void
WCNSFVHeatAdvectionPhysics::addNonlinearVariables()
{
  if (nonLinearVariableExists(_fluid_temperature_name, /*error_if_aux=*/true))
    return;

  auto params = getFactory().getValidParams("INSFVEnergyVariable");
  assignBlocks(params, blocks());
  params.set<std::vector<Real>>("scaling") = {getParam<Real>("energy_scaling")};
  params.set<MooseEnum>("face_interp_method") = getParam<MooseEnum>("energy_face_interpolation");
  params.set<bool>("two_term_boundary_expansion") = getParam<bool>("energy_two_term_interpolation");

  getProblem().addVariable("INSFVEnergyVariable", _fluid_temperature_name, params);
}

void
WCNSFVHeatAdvectionPhysics::addFVKernels()
{
  if (_compressibility == "incompressible")
    addINSEnergyTimeKernels();
  else
    addWCNSEnergyTimeKernels();

  addINSEnergyAdvectionKernels();
  addINSEnergyHeatConductionKernels();
  addINSEnergyAmbientConvection();
  addINSEnergyExternalHeatSource();
}

void
WCNSFVHeatAdvectionPhysics::addINSEnergyTimeKernels()
{
  std::string kernel_type = "INSFVEnergyTimeDerivative";
  std::string kernel_name = prefix() + "ins_energy_time";

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVEnergyTimeDerivative";
    kernel_name = prefix() + "pins_energy_time";
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;

  if (_porous_medium_treatment)
  {
    params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
    if (getProblem().hasFunctor(NS::time_deriv(_density_name), /*thread_id=*/0))
      params.set<MooseFunctorName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);
    params.set<bool>("is_solid") = false;
  }

  getProblem().addFVKernel(kernel_type, kernel_name, params);
}

void
WCNSFVHeatAdvectionPhysics::addWCNSEnergyTimeKernels()
{
  std::string en_kernel_type = "WCNSFVEnergyTimeDerivative";
  std::string kernel_name = prefix() + "wcns_energy_time";

  if (_porous_medium_treatment)
  {
    en_kernel_type = "PINSFVEnergyTimeDerivative";
    kernel_name = prefix() + "pwcns_energy_time";
  }

  InputParameters params = getFactory().getValidParams(en_kernel_type);
  assignBlocks(params, _blocks);
  params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseFunctorName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);
  params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;

  if (_porous_medium_treatment)
  {
    params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
    params.set<bool>("is_solid") = false;
  }

  getProblem().addFVKernel(en_kernel_type, kernel_name, params);
}

void
WCNSFVHeatAdvectionPhysics::addINSEnergyAdvectionKernels()
{
  std::string kernel_type = "INSFVEnergyAdvection";
  std::string kernel_name = prefix() + "ins_energy_advection";
  std::string rhie_chow_name = prefix() + "ins_rhie_chow_interpolator";
  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVEnergyAdvection";
    kernel_name = prefix() + "pins_energy_advection";
    rhie_chow_name = prefix() + "pins_rhie_chow_interpolator";
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  assignBlocks(params, _blocks);
  params.set<MooseEnum>("velocity_interp_method") = _velocity_interpolation;
  params.set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
  params.set<MooseEnum>("advected_interp_method") = getParam<MooseEnum>("energy_advection_method");

  getProblem().addFVKernel(kernel_type, kernel_name, params);
}

void
WCNSFVHeatAdvectionPhysics::addINSEnergyHeatConductionKernels()
{
  bool vector_conductivity = processThermalConductivity();
  unsigned int num_blocks = _thermal_conductivity_blocks.size();
  unsigned int num_used_blocks = num_blocks ? num_blocks : 1;

  for (unsigned int block_i = 0; block_i < num_used_blocks; ++block_i)
  {
    std::string block_name = "";
    if (num_blocks)
      block_name = Moose::stringify(_thermal_conductivity_blocks[block_i]);
    else
      block_name = std::to_string(block_i);

    if (_porous_medium_treatment)
    {
      const std::string kernel_type =
          vector_conductivity ? "PINSFVEnergyAnisotropicDiffusion" : "PINSFVEnergyDiffusion";

      InputParameters params = getFactory().getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      std::vector<SubdomainName> block_names =
          num_blocks ? _thermal_conductivity_blocks[block_i] : _blocks;
      assignBlocks(params, block_names);
      std::string conductivity_name = vector_conductivity ? NS::kappa : NS::k;
      params.set<MooseFunctorName>(conductivity_name) = _thermal_conductivity_name[block_i];
      params.set<MooseFunctorName>(NS::porosity) = _porosity_name;

      getProblem().addFVKernel(
          kernel_type, prefix() + "pins_energy_diffusion_" + block_name, params);
    }
    else
    {
      const std::string kernel_type = "FVDiffusion";
      InputParameters params = getFactory().getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      std::vector<SubdomainName> block_names =
          num_blocks ? _thermal_conductivity_blocks[block_i] : _blocks;
      assignBlocks(params, block_names);
      params.set<MooseFunctorName>("coeff") = _thermal_conductivity_name[block_i];

      getProblem().addFVKernel(
          kernel_type, prefix() + "ins_energy_diffusion_" + block_name, params);
    }
  }
}

void
WCNSFVHeatAdvectionPhysics::addINSEnergyAmbientConvection()
{
  const auto ambient_convection_blocks =
      getParam<std::vector<std::vector<SubdomainName>>>("ambient_convection_blocks");
  const auto ambient_convection_alpha =
      getParam<std::vector<MooseFunctorName>>("ambient_convection_alpha");
  const auto ambient_temperature =
      getParam<std::vector<MooseFunctorName>>("ambient_convection_temperatures");

  unsigned int num_convection_blocks = ambient_convection_blocks.size();
  unsigned int num_used_blocks = num_convection_blocks ? num_convection_blocks : 1;

  const std::string kernel_type = "PINSFVEnergyAmbientConvection";
  InputParameters params = getFactory().getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  params.set<MooseFunctorName>(NS::T_fluid) = _fluid_temperature_name;
  params.set<bool>("is_solid") = false;

  for (unsigned int block_i = 0; block_i < num_used_blocks; ++block_i)
  {
    std::string block_name = "";
    if (num_convection_blocks)
    {
      params.set<std::vector<SubdomainName>>("block") = ambient_convection_blocks[block_i];
      block_name = Moose::stringify(ambient_convection_blocks[block_i]);
    }
    else
    {
      assignBlocks(params, _blocks);
      block_name = std::to_string(block_i);
    }

    params.set<MooseFunctorName>("h_solid_fluid") = ambient_convection_alpha[block_i];
    params.set<MooseFunctorName>(NS::T_solid) = ambient_temperature[block_i];

    getProblem().addFVKernel(kernel_type, prefix() + "ambient_convection_" + block_name, params);
  }
}

void
WCNSFVHeatAdvectionPhysics::addINSEnergyExternalHeatSource()
{
  const std::string kernel_type = "FVCoupledForce";
  InputParameters params = getFactory().getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>("v") = getParam<MooseFunctorName>("external_heat_source");
  params.set<Real>("coef") = getParam<Real>("external_heat_source_coeff");

  getProblem().addFVKernel(kernel_type, prefix() + "external_heat_source", params);
}

void
WCNSFVHeatAdvectionPhysics::addWCNSEnergyMixingLengthKernels()
{
  const std::string u_names[3] = {"u", "v", "w"};
  const std::string kernel_type = "WCNSFVMixingLengthEnergyDiffusion";
  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;
  params.set<MooseFunctorName>(NS::mixing_length) = NS::mixing_length;
  params.set<Real>("schmidt_number") = getParam<Real>("turbulent_prandtl");
  params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;

  for (unsigned int dim_i = 0; dim_i < _dim; ++dim_i)
    params.set<MooseFunctorName>(u_names[dim_i]) = _velocity_names[dim_i];

  if (_porous_medium_treatment)
    getProblem().addFVKernel(kernel_type, prefix() + "pins_energy_mixing_length_diffusion", params);
  else
    getProblem().addFVKernel(kernel_type, prefix() + "ins_energy_mixing_length_diffusion", params);
}

void
WCNSFVHeatAdvectionPhysics::addEnthalpyMaterial()
{
  InputParameters params = getFactory().getValidParams("INSFVEnthalpyMaterial");
  assignBlocks(params, _blocks);

  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;
  params.set<MooseFunctorName>("temperature") = _fluid_temperature_name;

  getProblem().addMaterial("INSFVEnthalpyMaterial", prefix() + "ins_enthalpy_material", params);
}

bool
WCNSFVHeatAdvectionPhysics::processThermalConductivity()
{
  checkBlockwiseConsistency<MooseFunctorName>("thermal_conductivity_blocks",
                                              {"thermal_conductivity"});
  bool have_scalar = false;
  bool have_vector = false;

  for (unsigned int i = 0; i < _thermal_conductivity_name.size(); ++i)
  {
    // First, check if the name is just a number (only in case of isotropic conduction)
    if (MooseUtils::parsesToReal(_thermal_conductivity_name[i]))
      have_scalar = true;
    // Now we determine what kind of functor we are dealing with
    else
    {
      if (getProblem().template hasFunctorWithType<ADReal>(_thermal_conductivity_name[i],
                                                           /*thread_id=*/0))
        have_scalar = true;
      else
      {
        if (getProblem().template hasFunctorWithType<ADRealVectorValue>(
                _thermal_conductivity_name[i],
                /*thread_id=*/0))
          have_vector = true;
        else
        {
          paramError("thermal_conductivity",
                     "We only allow functor of type ADReal or ADRealVectorValue for thermal "
                     "conductivity!");
        }
      }
    }
  }

  if (have_vector && !_porous_medium_treatment)
    paramError("thermal_conductivity", "Cannot use anistropic diffusion with non-porous flows!");

  if (have_vector == have_scalar)
    paramError("thermal_conductivity",
               "The entries on thermal conductivity shall either be scalars of vectors, mixing "
               "them is not supported!");
  return have_vector;
}
