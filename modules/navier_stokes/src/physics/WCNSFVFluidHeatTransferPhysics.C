//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVFluidHeatTransferPhysics.h"
#include "WCNSFVFlowPhysics.h"
#include "NSFVBase.h"

registerNavierStokesPhysicsBaseTasks("NavierStokesApp", WCNSFVFluidHeatTransferPhysics);
registerWCNSFVFluidHeatTransferPhysicsBaseTasks("NavierStokesApp", WCNSFVFluidHeatTransferPhysics);

InputParameters
WCNSFVFluidHeatTransferPhysics::validParams()
{
  InputParameters params = WCNSFVFluidHeatTransferPhysicsBase::validParams();
  params.transferParam<MooseEnum>(NSFVBase::validParams(), "energy_face_interpolation");
  params.addParamNamesToGroup("energy_face_interpolation", "Numerical scheme");
  return params;
}

WCNSFVFluidHeatTransferPhysics::WCNSFVFluidHeatTransferPhysics(const InputParameters & parameters)
  : WCNSFVFluidHeatTransferPhysicsBase(parameters)

{
}

void
WCNSFVFluidHeatTransferPhysics::addSolverVariables()
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
    auto params = getFactory().getValidParams("INSFVEnergyVariable");
    assignBlocks(params, _blocks);
    params.set<std::vector<Real>>("scaling") = {getParam<Real>("energy_scaling")};
    params.set<MooseEnum>("face_interp_method") = getParam<MooseEnum>("energy_face_interpolation");
    params.set<bool>("two_term_boundary_expansion") =
        getParam<bool>("energy_two_term_bc_expansion");
    params.set<SolverSystemName>("solver_sys") = getSolverSystem(_fluid_temperature_name);
    getProblem().addVariable("INSFVEnergyVariable", _fluid_temperature_name, params);
  }
  else
    paramError("fluid_temperature_variable",
               "Variable (" + _fluid_temperature_name +
                   ") supplied to the WCNSFVFluidHeatTransferPhysics does not exist!");
}

void
WCNSFVFluidHeatTransferPhysics::addINSEnergyTimeKernels()
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
  params.set<MooseFunctorName>(NS::time_deriv(NS::specific_enthalpy)) =
      NS::time_deriv(NS::specific_enthalpy);

  if (_porous_medium_treatment)
  {
    params.set<MooseFunctorName>(NS::porosity) =
        _flow_equations_physics->getPorosityFunctorName(/*smoothed=*/false);
    if (getProblem().hasFunctor(NS::time_deriv(_density_name),
                                /*thread_id=*/0))
    {
      params.set<MooseFunctorName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);
      params.set<MooseFunctorName>(NS::specific_enthalpy) = NS::specific_enthalpy;
    }
    params.set<bool>("is_solid") = false;
  }

  getProblem().addFVKernel(kernel_type, kernel_name, params);
}

void
WCNSFVFluidHeatTransferPhysics::addWCNSEnergyTimeKernels()
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
  params.set<MooseFunctorName>(NS::specific_enthalpy) = NS::specific_enthalpy;
  params.set<MooseFunctorName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);

  if (_porous_medium_treatment)
  {
    params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;
    params.set<MooseFunctorName>(NS::porosity) =
        _flow_equations_physics->getPorosityFunctorName(/*smoothed=*/false);
    params.set<bool>("is_solid") = false;
  }

  getProblem().addFVKernel(en_kernel_type, kernel_name, params);
}

void
WCNSFVFluidHeatTransferPhysics::addINSEnergyAdvectionKernels()
{
  std::string kernel_type = "INSFVEnergyAdvection";
  std::string kernel_name = prefix() + "ins_energy_advection";
  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVEnergyAdvection";
    kernel_name = prefix() + "pins_energy_advection";
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  assignBlocks(params, _blocks);
  params.set<MooseEnum>("velocity_interp_method") = _velocity_interpolation;
  params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
  params.set<MooseEnum>("advected_interp_method") =
      getParam<MooseEnum>("energy_advection_interpolation");

  getProblem().addFVKernel(kernel_type, kernel_name, params);
}

void
WCNSFVFluidHeatTransferPhysics::addINSEnergyHeatConductionKernels()
{
  const auto vector_conductivity = processThermalConductivity();
  const auto num_blocks = _thermal_conductivity_blocks.size();
  const auto num_used_blocks = num_blocks ? num_blocks : 1;

  for (const auto block_i : make_range(num_used_blocks))
  {
    std::string block_name = "";
    if (num_blocks)
      block_name = Moose::stringify(_thermal_conductivity_blocks[block_i]);
    else
      block_name = "all";

    if (_porous_medium_treatment)
    {
      const auto kernel_type =
          vector_conductivity ? "PINSFVEnergyAnisotropicDiffusion" : "PINSFVEnergyDiffusion";

      InputParameters params = getFactory().getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      const auto block_names = num_blocks ? _thermal_conductivity_blocks[block_i] : _blocks;
      assignBlocks(params, block_names);
      const auto conductivity_name = vector_conductivity ? NS::kappa : NS::k;
      params.set<MooseFunctorName>(conductivity_name) = _thermal_conductivity_name[block_i];
      params.set<MooseFunctorName>(NS::porosity) =
          _flow_equations_physics->getPorosityFunctorName(true);
      params.set<bool>("effective_conductivity") = getParam<bool>("effective_conductivity");

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
WCNSFVFluidHeatTransferPhysics::addINSEnergyAmbientConvection()
{
  unsigned int num_convection_blocks = _ambient_convection_blocks.size();
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
      params.set<std::vector<SubdomainName>>("block") = _ambient_convection_blocks[block_i];
      block_name = Moose::stringify(_ambient_convection_blocks[block_i]);
    }
    else
    {
      assignBlocks(params, _blocks);
      block_name = std::to_string(block_i);
    }

    params.set<MooseFunctorName>("h_solid_fluid") = _ambient_convection_alpha[block_i];
    params.set<MooseFunctorName>(NS::T_solid) = _ambient_temperature[block_i];

    getProblem().addFVKernel(kernel_type, prefix() + "ambient_convection_" + block_name, params);
  }
}

void
WCNSFVFluidHeatTransferPhysics::addINSEnergyExternalHeatSource()
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
WCNSFVFluidHeatTransferPhysics::addINSEnergyInletBC()
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

  unsigned int flux_bc_counter = 0;
  for (const auto bc_ind : index_range(_energy_inlet_types))
  {
    if (_energy_inlet_types[bc_ind] == "fixed-temperature")
    {
      const std::string bc_type = "FVADFunctorDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      params.set<MooseFunctorName>("functor") = _energy_inlet_functors[bc_ind];
      params.set<std::vector<BoundaryName>>("boundary") = {inlet_boundaries[bc_ind]};

      getProblem().addFVBC(
          bc_type, _fluid_temperature_name + "_" + inlet_boundaries[bc_ind], params);
    }
    else if (_energy_inlet_types[bc_ind] == "heatflux")
    {
      const std::string bc_type = "FVFunctionNeumannBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      params.set<FunctionName>("function") = _energy_inlet_functors[bc_ind];
      params.set<std::vector<BoundaryName>>("boundary") = {inlet_boundaries[bc_ind]};

      getProblem().addFVBC(
          bc_type, _fluid_temperature_name + "_" + inlet_boundaries[bc_ind], params);
    }
    else if (_energy_inlet_types[bc_ind] == "flux-mass" ||
             _energy_inlet_types[bc_ind] == "flux-velocity")
    {
      const std::string bc_type = "WCNSFVEnergyFluxBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      const auto flux_inlet_directions = _flow_equations_physics->getFluxInletDirections();
      const auto flux_inlet_pps = _flow_equations_physics->getFluxInletPPs();

      if (flux_inlet_directions.size())
        params.set<Point>("direction") = flux_inlet_directions[flux_bc_counter];
      if (_energy_inlet_types[bc_ind] == "flux-mass")
      {
        params.set<PostprocessorName>("mdot_pp") = flux_inlet_pps[flux_bc_counter];
        params.set<PostprocessorName>("area_pp") = "area_pp_" + inlet_boundaries[bc_ind];
      }
      else
        params.set<PostprocessorName>("velocity_pp") = flux_inlet_pps[flux_bc_counter];

      params.set<PostprocessorName>("temperature_pp") = _energy_inlet_functors[bc_ind];
      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;
      params.set<MooseFunctorName>(NS::T_fluid) = _fluid_temperature_name;

      for (const auto d : make_range(dimension()))
        params.set<MooseFunctorName>(NS::velocity_vector[d]) = _velocity_names[d];

      params.set<std::vector<BoundaryName>>("boundary") = {inlet_boundaries[bc_ind]};

      getProblem().addFVBC(
          bc_type, _fluid_temperature_name + "_" + inlet_boundaries[bc_ind], params);
      flux_bc_counter += 1;
    }
  }
}

void
WCNSFVFluidHeatTransferPhysics::addINSEnergyWallBC()
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
      const std::string bc_type = "FVFunctorDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      params.set<MooseFunctorName>("functor") = _energy_wall_functors[bc_ind];
      params.set<std::vector<BoundaryName>>("boundary") = {wall_boundaries[bc_ind]};

      getProblem().addFVBC(
          bc_type, _fluid_temperature_name + "_" + wall_boundaries[bc_ind], params);
    }
    else if (_energy_wall_types[bc_ind] == "heatflux")
    {
      const std::string bc_type = "FVFunctorNeumannBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      params.set<MooseFunctorName>("functor") = _energy_wall_functors[bc_ind];
      params.set<std::vector<BoundaryName>>("boundary") = {wall_boundaries[bc_ind]};

      getProblem().addFVBC(
          bc_type, _fluid_temperature_name + "_" + wall_boundaries[bc_ind], params);
    }
    // We add this boundary condition here to facilitate the input of wall boundaries / functors for
    // energy. If there are too many turbulence options and this gets out of hand we will have to
    // move this to the turbulence Physics
    else if (_energy_wall_types[bc_ind] == "wallfunction")
    {
      if (!_turbulence_physics)
        paramError("coupled_turbulence_physics",
                   "A coupled turbulence Physics was not found for defining the wall function "
                   "boundary condition on boundary: " +
                       wall_boundaries[bc_ind]);
      const std::string bc_type = "INSFVTurbulentTemperatureWallFunction";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      params.set<std::vector<BoundaryName>>("boundary") = {wall_boundaries[bc_ind]};
      params.set<MooseEnum>("wall_treatment") =
          _turbulence_physics->turbulenceTemperatureWallTreatment();
      params.set<MooseFunctorName>("T_w") = _energy_wall_functors[bc_ind];
      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;
      params.set<MooseFunctorName>(NS::TKE) = _turbulence_physics->tkeName();
      if (_thermal_conductivity_name.size() != 1)
        mooseError("Several anisotropic thermal conductivity (kappa) regions have been specified. "
                   "Selecting the right kappa coefficient for the turbulence boundaries is not "
                   "currently implemented.\nBoundaries:\n" +
                   Moose::stringify(_turbulence_physics->turbulenceWalls()) +
                   "\nKappa(s) specified:\n" + Moose::stringify(_thermal_conductivity_name));
      params.set<MooseFunctorName>(NS::kappa) = _thermal_conductivity_name[0];
      params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;
      const std::string u_names[3] = {"u", "v", "w"};
      for (const auto d : make_range(dimension()))
        params.set<MooseFunctorName>(u_names[d]) = _velocity_names[d];
      // Currently only Newton method for WCNSFVFluidHeatTransferPhysics
      params.set<bool>("newton_solve") = true;
      getProblem().addFVBC(bc_type, prefix() + "wallfunction_" + wall_boundaries[bc_ind], params);
    }
  }
}
