//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSLinearFVFluidHeatTransferPhysics.h"
#include "WCNSFVFlowPhysics.h"
#include "PINSFVEnergyAnisotropicDiffusion.h"
#include "NSFVBase.h"

registerNavierStokesPhysicsBaseTasks("NavierStokesApp", WCNSLinearFVFluidHeatTransferPhysics);
registerWCNSFVFluidHeatTransferPhysicsBaseTasks("NavierStokesApp",
                                                WCNSLinearFVFluidHeatTransferPhysics);

InputParameters
WCNSLinearFVFluidHeatTransferPhysics::validParams()
{
  InputParameters params = WCNSFVFluidHeatTransferPhysicsBase::validParams();
  params.addParam<bool>(
      "use_nonorthogonal_correction",
      true,
      "Whether to use a non-orthogonal correction. This can potentially slow down convergence "
      ", but reduces numerical dispersion on non-orthogonal meshes. Can "
      "be safely turned off on orthogonal meshes.");
  params.set<std::vector<SolverSystemName>>("system_names") = {"energy_system"};

  // Not implemented
  params.suppressParameter<bool>("effective_conductivity");
  return params;
}

WCNSLinearFVFluidHeatTransferPhysics::WCNSLinearFVFluidHeatTransferPhysics(
    const InputParameters & parameters)
  : WCNSFVFluidHeatTransferPhysicsBase(parameters)
{
  if (_porous_medium_treatment)
    paramError("porous_medium_treatment", "Porous media not supported at this time");
}

void
WCNSLinearFVFluidHeatTransferPhysics::addSolverVariables()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_energy_equation)
    return;

  // Dont add if the user already defined the variable
  if (variableExists(_fluid_temperature_name,
                     /*error_if_aux=*/true))
    checkBlockRestrictionIdentical(_fluid_temperature_name,
                                   getProblem().getVariable(0, _fluid_temperature_name).blocks());
  else if (_define_variables)
  {
    const auto var_type = "MooseLinearVariableFVReal";
    auto params = getFactory().getValidParams(var_type);
    assignBlocks(params, _blocks);
    params.set<std::vector<Real>>("scaling") = {getParam<Real>("energy_scaling")};
    params.set<SolverSystemName>("solver_sys") = getSolverSystem(_fluid_temperature_name);
    getProblem().addVariable(var_type, _fluid_temperature_name, params);
  }
  else
    paramError("fluid_temperature_variable",
               "Variable (" + _fluid_temperature_name +
                   ") supplied to the WCNSLinearFVFluidHeatTransferPhysics does not exist!");
}

void
WCNSLinearFVFluidHeatTransferPhysics::addINSEnergyTimeKernels()
{
  paramError("transient",
             "Transient simulations not supported at this time with the linear FV discretization");
}

void
WCNSLinearFVFluidHeatTransferPhysics::addWCNSEnergyTimeKernels()
{
  paramError("transient",
             "Transient simulations not supported at this time with the linear FV discretization");
}

void
WCNSLinearFVFluidHeatTransferPhysics::addINSEnergyAdvectionKernels()
{
  std::string kernel_type = "LinearFVEnergyAdvection";
  std::string kernel_name = prefix() + "ins_energy_advection";

  InputParameters params = getFactory().getValidParams(kernel_type);
  params.set<LinearVariableName>("variable") = _fluid_temperature_name;
  assignBlocks(params, _blocks);
  if (!MooseUtils::isFloat(_specific_heat_name))
    paramError(NS::cp, "Must be a Real number. Functors not supported at this time");
  params.set<Real>("cp") = std::atof(_specific_heat_name.c_str());
  params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
  params.set<MooseEnum>("advected_interp_method") =
      getParam<MooseEnum>("energy_advection_interpolation");

  getProblem().addLinearFVKernel(kernel_type, kernel_name, params);
}

void
WCNSLinearFVFluidHeatTransferPhysics::addINSEnergyHeatConductionKernels()
{
  const auto num_blocks = _thermal_conductivity_blocks.size();
  const auto num_used_blocks = num_blocks ? num_blocks : 1;

  for (const auto block_i : make_range(num_used_blocks))
  {
    std::string block_name = "";
    if (num_blocks)
      block_name = Moose::stringify(_thermal_conductivity_blocks[block_i]);
    else
      block_name = "all";

    const std::string kernel_type = "LinearFVDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<LinearVariableName>("variable") = _fluid_temperature_name;
    std::vector<SubdomainName> block_names =
        num_blocks ? _thermal_conductivity_blocks[block_i] : _blocks;
    assignBlocks(params, block_names);
    // NOTE: vector conductivities not supported at this time
    params.set<MooseFunctorName>("diffusion_coeff") = _thermal_conductivity_name[block_i];
    params.set<bool>("use_nonorthogonal_correction") =
        getParam<bool>("use_nonorthogonal_correction");

    getProblem().addLinearFVKernel(
        kernel_type, prefix() + "ins_energy_diffusion_" + block_name, params);
  }
}

void
WCNSLinearFVFluidHeatTransferPhysics::addINSEnergyAmbientConvection()
{
  unsigned int num_convection_blocks = _ambient_convection_blocks.size();
  unsigned int num_used_blocks = num_convection_blocks ? num_convection_blocks : 1;

  const std::string kernel_type = "LinearFVVolumetricHeatTransfer";
  InputParameters params = getFactory().getValidParams(kernel_type);
  params.set<LinearVariableName>("variable") = _fluid_temperature_name;
  params.set<VariableName>(NS::T_fluid) = _fluid_temperature_name;
  params.set<bool>("is_solid") = false;

  for (unsigned int block_i = 0; block_i < num_used_blocks; ++block_i)
  {
    std::string block_name = "";
    if (num_convection_blocks)
    {
      params.set<std::vector<SubdomainName>>("block") = _ambient_convection_blocks[block_i];
      block_name = Moose::stringify(_ambient_convection_blocks[block_i]);
    }
    else
    {
      assignBlocks(params, _blocks);
      block_name = std::to_string(block_i);
    }

    params.set<MooseFunctorName>("h_solid_fluid") = _ambient_convection_alpha[block_i];
    params.set<VariableName>(NS::T_solid) = _ambient_temperature[block_i];

    getProblem().addLinearFVKernel(
        kernel_type, prefix() + "ambient_convection_" + block_name, params);
  }
}

void
WCNSLinearFVFluidHeatTransferPhysics::addINSEnergyExternalHeatSource()
{
  const std::string kernel_type = "LinearFVSource";
  InputParameters params = getFactory().getValidParams(kernel_type);
  params.set<LinearVariableName>("variable") = _fluid_temperature_name;
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>("source_density") =
      getParam<MooseFunctorName>("external_heat_source");
  params.set<Real>("scaling_factor") = getParam<Real>("external_heat_source_coeff");

  getProblem().addLinearFVKernel(kernel_type, prefix() + "external_heat_source", params);
}

void
WCNSLinearFVFluidHeatTransferPhysics::addINSEnergyInletBC()
{
  const auto & inlet_boundaries = _flow_equations_physics->getInletBoundaries();
  // These are parameter errors for now. If Components add boundaries to Physics, the error
  // may not be due to parameters anymore.
  if (inlet_boundaries.size() != _energy_inlet_types.size())
    paramError("energy_inlet_types",
               "Energy inlet types (size " + std::to_string(_energy_inlet_types.size()) +
                   ") should be the same size as inlet_boundaries (size " +
                   std::to_string(inlet_boundaries.size()) + ")");
  if (inlet_boundaries.size() != _energy_inlet_functors.size())
    paramError("energy_inlet_functors",
               "Energy inlet functors (size " + std::to_string(_energy_inlet_functors.size()) +
                   ") should be the same size as inlet_boundaries (size " +
                   std::to_string(inlet_boundaries.size()) + ")");

  for (const auto bc_ind : index_range(_energy_inlet_types))
  {
    if (_energy_inlet_types[bc_ind] == "fixed-temperature")
    {
      const std::string bc_type = "LinearFVAdvectionDiffusionFunctorDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<LinearVariableName>("variable") = _fluid_temperature_name;
      params.set<MooseFunctorName>("functor") = _energy_inlet_functors[bc_ind];
      params.set<std::vector<BoundaryName>>("boundary") = {inlet_boundaries[bc_ind]};

      getProblem().addLinearFVBC(
          bc_type, _fluid_temperature_name + "_" + inlet_boundaries[bc_ind], params);
    }
    else if (_energy_inlet_types[bc_ind] == "heatflux")
      paramError("energy_inlet_types", "Heat flux inlet boundary conditions not yet supported");
    else if (_energy_inlet_types[bc_ind] == "flux-mass" ||
             _energy_inlet_types[bc_ind] == "flux-velocity")
      paramError("energy_inlet_types", "Flux inlet boundary conditions not yet supported");
  }
}

void
WCNSLinearFVFluidHeatTransferPhysics::addINSEnergyWallBC()
{
  const auto & wall_boundaries = _flow_equations_physics->getWallBoundaries();
  if (wall_boundaries.size() != _energy_wall_types.size())
    paramError("energy_wall_types",
               "Energy wall types (size " + std::to_string(_energy_wall_types.size()) +
                   ") should be the same size as wall_boundaries (size " +
                   std::to_string(wall_boundaries.size()) + ")");
  if (wall_boundaries.size() != _energy_wall_functors.size())
    paramError("energy_wall_functors",
               "Energy wall functors (size " + std::to_string(_energy_wall_functors.size()) +
                   ") should be the same size as wall_boundaries (size " +
                   std::to_string(wall_boundaries.size()) + ")");

  for (unsigned int bc_ind = 0; bc_ind < _energy_wall_types.size(); ++bc_ind)
  {
    if (_energy_wall_types[bc_ind] == "fixed-temperature")
    {
      const std::string bc_type = "LinearFVAdvectionDiffusionFunctorDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<LinearVariableName>("variable") = _fluid_temperature_name;
      params.set<MooseFunctorName>("functor") = _energy_wall_functors[bc_ind];
      params.set<std::vector<BoundaryName>>("boundary") = {wall_boundaries[bc_ind]};

      getProblem().addLinearFVBC(
          bc_type, _fluid_temperature_name + "_" + wall_boundaries[bc_ind], params);
    }
    else if (_energy_wall_types[bc_ind] == "heatflux")
      paramError("energy_inlet_types", "Heat flux wall boundary conditions not yet supported");
    // We add this boundary condition here to facilitate the input of wall boundaries / functors for
    // energy. If there are too many turbulence options and this gets out of hand we will have to
    // move this to the turbulence Physics
    else if (_energy_wall_types[bc_ind] == "wallfunction")
      mooseError("Wall-function boundary condition not supported at this time.");
  }
}

void
WCNSLinearFVFluidHeatTransferPhysics::addINSEnergyOutletBC()
{
  const auto & outlet_boundaries = _flow_equations_physics->getOutletBoundaries();
  if (outlet_boundaries.empty())
    return;

  for (const auto & outlet_bdy : outlet_boundaries)
  {
    const std::string bc_type = "LinearFVAdvectionDiffusionOutflowBC";
    InputParameters params = getFactory().getValidParams(bc_type);
    params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};
    params.set<bool>("use_two_term_expansion") = getParam<bool>("energy_two_term_bc_expansion");

    params.set<LinearVariableName>("variable") = _fluid_temperature_name;
    getProblem().addLinearFVBC(bc_type, _fluid_temperature_name + "_" + outlet_bdy, params);
  }
}
