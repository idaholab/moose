//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
registerMooseAction("NavierStokesApp", WCNSLinearFVFluidHeatTransferPhysics, "add_aux_variable");
registerMooseAction("NavierStokesApp", WCNSLinearFVFluidHeatTransferPhysics, "add_aux_kernel");

InputParameters
WCNSLinearFVFluidHeatTransferPhysics::validParams()
{
  InputParameters params = WCNSFVFluidHeatTransferPhysicsBase::validParams();
  params.addParam<bool>(
      "use_nonorthogonal_correction",
      true,
      "Whether to use a non-orthogonal correction. This can potentially slow down convergence "
      ", but reduces numerical dispersion on non-orthogonal meshes. Can be safely turned off on "
      "orthogonal meshes.");
  params.set<std::vector<SolverSystemName>>("system_names") = {"energy_system"};

  params.addParam<UserObjectName>(NS::fluid, "Fluid properties userobject");
  params.addParamNamesToGroup(NS::fluid, "Material properties");

  // We could split between discretization and solver here.
  params.addParamNamesToGroup("use_nonorthogonal_correction system_names", "Numerical scheme");

  // Not implemented
  params.suppressParameter<bool>("effective_conductivity");
  // Not needed
  params.suppressParameter<bool>("add_energy_equation");
  return params;
}

WCNSLinearFVFluidHeatTransferPhysics::WCNSLinearFVFluidHeatTransferPhysics(
    const InputParameters & parameters)
  : WCNSFVFluidHeatTransferPhysicsBase(parameters)
{
  if (_porous_medium_treatment)
    paramError("porous_medium_treatment", "Porous media not supported at this time");
  checkSecondParamSetOnlyIfFirstOneTrue("solve_for_enthalpy", NS::fluid);
}

void
WCNSLinearFVFluidHeatTransferPhysics::addSolverVariables()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_energy_equation)
    return;

  const auto variable_name = _solve_for_enthalpy ? _fluid_enthalpy_name : _fluid_temperature_name;

  // Dont add if the user already defined the variable
  if (variableExists(variable_name,
                     /*error_if_aux=*/true))
    checkBlockRestrictionIdentical(variable_name,
                                   getProblem().getVariable(0, variable_name).blocks());
  else if (_define_variables)
  {
    const auto var_type = "MooseLinearVariableFVReal";
    auto params = getFactory().getValidParams(var_type);
    assignBlocks(params, _blocks);
    params.set<SolverSystemName>("solver_sys") = getSolverSystem(variable_name);
    getProblem().addVariable(var_type, variable_name, params);
  }
  else
    paramError(_solve_for_enthalpy ? "solve_for_enthalpy" : "fluid_temperature_variable",
               "Variable (" + variable_name +
                   ") supplied to the WCNSLinearFVFluidHeatTransferPhysics does not exist!");
}

void
WCNSLinearFVFluidHeatTransferPhysics::addEnergyTimeKernels()
{
  std::string kernel_type = "LinearFVTimeDerivative";
  std::string kernel_name = prefix() + "ins_energy_time";

  InputParameters params = getFactory().getValidParams(kernel_type);
  params.set<LinearVariableName>("variable") =
      _solve_for_enthalpy ? _fluid_enthalpy_name : _fluid_temperature_name;
  assignBlocks(params, _blocks);
  if (!_solve_for_enthalpy)
    params.set<MooseFunctorName>("factor") = "rho_cp";
  else
    params.set<MooseFunctorName>("factor") = "rho";

  getProblem().addLinearFVKernel(kernel_type, kernel_name, params);
}

void
WCNSLinearFVFluidHeatTransferPhysics::addEnergyAdvectionKernels()
{
  std::string kernel_type = "LinearFVEnergyAdvection";
  std::string kernel_name = prefix() + "ins_energy_advection";

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  if (!getParam<bool>("solve_for_enthalpy"))
  {
    params.set<LinearVariableName>("variable") = _fluid_temperature_name;
    params.set<MooseEnum>("advected_quantity") = "temperature";
    if (!MooseUtils::isFloat(_specific_heat_name))
      paramError(NS::cp, "Must be a Real number. Functors not supported at this time");
    params.set<Real>("cp") = std::atof(_specific_heat_name.c_str());
  }
  else
    params.set<LinearVariableName>("variable") = _fluid_enthalpy_name;
  params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
  params.set<MooseEnum>("advected_interp_method") =
      getParam<MooseEnum>("energy_advection_interpolation");

  getProblem().addLinearFVKernel(kernel_type, kernel_name, params);
}

void
WCNSLinearFVFluidHeatTransferPhysics::addEnergyHeatConductionKernels()
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
    if (!_solve_for_enthalpy)
    {
      params.set<LinearVariableName>("variable") = _fluid_temperature_name;
      params.set<MooseFunctorName>("diffusion_coeff") = _thermal_conductivity_name[block_i];
    }
    else
    {
      params.set<LinearVariableName>("variable") = _fluid_enthalpy_name;
      params.set<MooseFunctorName>("diffusion_coeff") =
          _thermal_conductivity_name[block_i] + "_by_cp";
    }
    std::vector<SubdomainName> block_names =
        num_blocks ? _thermal_conductivity_blocks[block_i] : _blocks;
    assignBlocks(params, block_names);
    // NOTE: vector conductivities not supported at this time
    bool has_vector = processThermalConductivity();
    if (has_vector)
      paramError("thermal_conductivity",
                 "Vector thermal conductivities not currently supported with the linear finite "
                 "volume discretization");

    params.set<bool>("use_nonorthogonal_correction") =
        getParam<bool>("use_nonorthogonal_correction");

    getProblem().addLinearFVKernel(
        kernel_type, prefix() + "ins_energy_diffusion_" + block_name, params);
  }
}

void
WCNSLinearFVFluidHeatTransferPhysics::addEnergyAmbientConvection()
{
  unsigned int num_convection_blocks = _ambient_convection_blocks.size();
  unsigned int num_used_blocks = num_convection_blocks ? num_convection_blocks : 1;

  const std::string kernel_type = "LinearFVVolumetricHeatTransfer";
  InputParameters params = getFactory().getValidParams(kernel_type);
  params.set<LinearVariableName>("variable") =
      _solve_for_enthalpy ? _fluid_enthalpy_name : _fluid_temperature_name;
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
WCNSLinearFVFluidHeatTransferPhysics::addEnergyExternalHeatSource()
{
  const std::string kernel_type = "LinearFVSource";
  InputParameters params = getFactory().getValidParams(kernel_type);
  params.set<LinearVariableName>("variable") =
      _solve_for_enthalpy ? _fluid_enthalpy_name : _fluid_temperature_name;
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>("source_density") =
      getParam<MooseFunctorName>("external_heat_source");
  params.set<Real>("scaling_factor") = getParam<Real>("external_heat_source_coeff");

  getProblem().addLinearFVKernel(kernel_type, prefix() + "external_heat_source", params);
}

void
WCNSLinearFVFluidHeatTransferPhysics::addAuxiliaryVariables()
{
  if (_solve_for_enthalpy)
  {
    // Dont add if the user already defined the variable
    if (variableExists(_fluid_temperature_name,
                       /*error_if_aux=*/false))
      checkBlockRestrictionIdentical(_fluid_temperature_name,
                                     getProblem().getVariable(0, _fluid_temperature_name).blocks());
    else if (_define_variables)
    {
      const auto var_type = "MooseLinearVariableFVReal";
      auto params = getFactory().getValidParams(var_type);
      assignBlocks(params, _blocks);
      getProblem().addAuxVariable(var_type, _fluid_temperature_name, params);
    }
    else
      paramError("fluid_temperature_variable",
                 "Variable (" + _fluid_temperature_name +
                     ") supplied to the WCNSLinearFVFluidHeatTransferPhysics does not exist!");
  }
}

void
WCNSLinearFVFluidHeatTransferPhysics::addAuxiliaryKernels()
{
  if (_solve_for_enthalpy)
  {
    // Keep temperature updated
    const std::string kernel_type = "FunctorAux";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<AuxVariableName>("variable") = _fluid_temperature_name;
    assignBlocks(params, _blocks);
    params.set<MooseFunctorName>("functor") = "T_from_p_h";
    params.set<ExecFlagEnum>("execute_on") = {EXEC_NONLINEAR};
    getProblem().addAuxKernel(kernel_type, prefix() + "update_temperature", params);
  }
}

void
WCNSLinearFVFluidHeatTransferPhysics::addEnergyInletBC()
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

      // Boundary condition on temperature must be forwarded to enthalpy
      if (_solve_for_enthalpy)
      {
        params.set<LinearVariableName>("variable") = _fluid_enthalpy_name;
        params.set<MooseFunctorName>("functor") = "h_from_p_T";
        getProblem().addLinearFVBC(
            bc_type, _fluid_enthalpy_name + "_" + inlet_boundaries[bc_ind], params);
      }
    }
    else if (_energy_inlet_types[bc_ind] == "heatflux")
      paramError("energy_inlet_types", "Heat flux inlet boundary conditions not yet supported");
    else if (_energy_inlet_types[bc_ind] == "flux-mass" ||
             _energy_inlet_types[bc_ind] == "flux-velocity")
      paramError("energy_inlet_types", "Flux inlet boundary conditions not yet supported");
  }
}

void
WCNSLinearFVFluidHeatTransferPhysics::addEnergyWallBC()
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

      // Boundary condition on temperature must be forwarded to enthalpy
      if (_solve_for_enthalpy)
      {
        params.set<LinearVariableName>("variable") = _fluid_enthalpy_name;
        params.set<MooseFunctorName>("functor") = "h_from_p_T";
        getProblem().addLinearFVBC(
            bc_type, _fluid_enthalpy_name + "_" + wall_boundaries[bc_ind], params);
      }
    }
    else if (_energy_wall_types[bc_ind] == "heatflux")
    {
      const std::string bc_type = "LinearFVAdvectionDiffusionFunctorNeumannBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      const auto var_name = _solve_for_enthalpy ? _fluid_enthalpy_name : _fluid_temperature_name;
      params.set<LinearVariableName>("variable") = var_name;
      params.set<MooseFunctorName>("functor") = _energy_wall_functors[bc_ind];
      params.set<std::vector<BoundaryName>>("boundary") = {wall_boundaries[bc_ind]};

      getProblem().addLinearFVBC(
          bc_type, var_name + "_heatflux_" + wall_boundaries[bc_ind], params);
    }
    else if (_energy_wall_types[bc_ind] == "convection")
    {
      const std::string bc_type = "LinearFVConvectiveHeatTransferBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<LinearVariableName>("variable") =
          _solve_for_enthalpy ? _fluid_enthalpy_name : _fluid_temperature_name;
      params.set<MooseFunctorName>(NS::T_fluid) = _fluid_temperature_name;
      const auto Tinf_htc_functors =
          MooseUtils::split(_energy_wall_functors[bc_ind], /*delimiter=*/":", /*max_count=*/1);
      if (Tinf_htc_functors.size() != 2)
        paramError("energy_wall_functors",
                   "'convection' wall types require two functors specified as "
                   "<Tinf_functor>:<htc_functor>.");
      params.set<MooseFunctorName>(NS::T_solid) = Tinf_htc_functors[0];
      params.set<MooseFunctorName>("h") = Tinf_htc_functors[1];
      params.set<std::vector<BoundaryName>>("boundary") = {wall_boundaries[bc_ind]};

      getProblem().addLinearFVBC(
          bc_type,
          (_solve_for_enthalpy ? _fluid_enthalpy_name : _fluid_temperature_name) + "_" +
              wall_boundaries[bc_ind],
          params);
    }
    else
      paramError(
          "energy_wall_types", _energy_wall_types[bc_ind], " wall type is currently unsupported.");
  }
}

void
WCNSLinearFVFluidHeatTransferPhysics::addEnergyOutletBC()
{
  const auto & outlet_boundaries = _flow_equations_physics->getOutletBoundaries();
  if (outlet_boundaries.empty())
    return;

  for (const auto & outlet_bdy : outlet_boundaries)
  {
    const std::string bc_type = "LinearFVAdvectionDiffusionOutflowBC";
    const auto variable_name = _solve_for_enthalpy ? _fluid_enthalpy_name : _fluid_temperature_name;
    InputParameters params = getFactory().getValidParams(bc_type);
    params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};
    params.set<bool>("use_two_term_expansion") = getParam<bool>("energy_two_term_bc_expansion");

    params.set<LinearVariableName>("variable") = variable_name;
    getProblem().addLinearFVBC(bc_type, variable_name + "_" + outlet_bdy, params);
  }
}

void
WCNSLinearFVFluidHeatTransferPhysics::addMaterials()
{
  WCNSFVFluidHeatTransferPhysicsBase::addMaterials();

  if (_solve_for_enthalpy)
  {
    // Define alpha, the diffusion coefficient when solving for enthalpy, on each block
    for (unsigned int i = 0; i < _thermal_conductivity_name.size(); ++i)
    {
      const auto object_type = "ADParsedFunctorMaterial";
      InputParameters params = getFactory().getValidParams(object_type);
      assignBlocks(params, _blocks);
      std::vector<std::string> f_names;
      if (!MooseUtils::parsesToReal(_thermal_conductivity_name[i]))
        f_names.push_back(_thermal_conductivity_name[i]);
      if (!MooseUtils::parsesToReal(getSpecificHeatName()))
        f_names.push_back(getSpecificHeatName());
      params.set<std::vector<std::string>>("functor_names") = f_names;
      params.set<std::string>("expression") =
          _thermal_conductivity_name[i] + "/" + getSpecificHeatName();
      params.set<std::string>("property_name") = _thermal_conductivity_name[i] + "_by_cp";
      getProblem().addMaterial(
          object_type, prefix() + "alpha_from_" + _thermal_conductivity_name[i], params);
    }
  }
}
