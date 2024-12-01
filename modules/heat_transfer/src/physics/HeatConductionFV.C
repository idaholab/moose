//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatConductionFV.h"

// Register the actions for the objects actually used
registerPhysicsBaseTasks("HeatTransferApp", HeatConductionFV);
registerMooseAction("HeatTransferApp", HeatConductionFV, "add_fv_kernel");
registerMooseAction("HeatTransferApp", HeatConductionFV, "add_fv_bc");
registerMooseAction("HeatTransferApp", HeatConductionFV, "add_variable");
registerMooseAction("HeatTransferApp", HeatConductionFV, "add_ic");
registerMooseAction("HeatTransferApp", HeatConductionFV, "add_preconditioning");

InputParameters
HeatConductionFV::validParams()
{
  InputParameters params = HeatConductionPhysicsBase::validParams();
  params.addClassDescription(
      "Creates the heat conduction equation discretized with nonlinear finite volume");

  // Material properties
  params.addRequiredParam<MooseFunctorName>("thermal_conductivity_functor",
                                            "Thermal conductivity functor (material property)");
  params.addParam<MaterialPropertyName>("specific_heat", "cp", "Specific heat  material property");
  params.addParam<MaterialPropertyName>("density", "density", "Density material property");
  params.addParamNamesToGroup("thermal_conductivity_functor specific_heat density",
                              "Thermal properties");

  params.addRangeCheckedParam<Real>("temperature_scaling",
                                    1,
                                    "temperature_scaling > 0",
                                    "Scaling factor for the heat conduction equation");

  return params;
}

HeatConductionFV::HeatConductionFV(const InputParameters & parameters)
  : HeatConductionPhysicsBase(parameters)
{
}

void
HeatConductionFV::initializePhysicsAdditional()
{
  getProblem().needFV();
}

void
HeatConductionFV::addFVKernels()
{
  {
    const std::string kernel_type = "FVDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _temperature_name;
    params.set<MooseFunctorName>("coeff") =
        getParam<MooseFunctorName>("thermal_conductivity_functor");
    getProblem().addFVKernel(kernel_type, prefix() + _temperature_name + "_conduction", params);
  }
  if (isParamValid("heat_source_var"))
  {
    const std::string kernel_type = "FVCoupledForce";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _temperature_name;
    params.set<MooseFunctorName>("v") = getParam<VariableName>("heat_source_var");
    if (isParamValid("heat_source_blocks"))
      params.set<std::vector<SubdomainName>>("block") =
          getParam<std::vector<SubdomainName>>("heat_source_blocks");
    getProblem().addFVKernel(kernel_type, prefix() + _temperature_name + "_source", params);
  }
  if (isParamValid("heat_source_functor"))
  {
    const std::string kernel_type = "FVBodyForce";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _temperature_name;
    const auto & functor_name = getParam<MooseFunctorName>("heat_source_functor");
    if (MooseUtils::parsesToReal(functor_name))
      params.set<Real>("value") = std::stod(functor_name);
    else if (getProblem().hasFunction(functor_name))
      params.set<FunctionName>("function") = functor_name;
    else if (getProblem().hasPostprocessorValueByName(functor_name))
      params.set<PostprocessorName>("postprocessor") = functor_name;
    else
      paramError("heat_source_functor",
                 "Unsupported functor type. Consider using 'heat_source_var'.");
    getProblem().addFVKernel(kernel_type, prefix() + _temperature_name + "_source_functor", params);
  }
  if (isTransient())
  {
    const std::string kernel_type = "FVHeatConductionTimeDerivative";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _temperature_name;
    params.applyParameter(parameters(), "specific_heat");
    params.set<MaterialPropertyName>("density_name") = getParam<MaterialPropertyName>("density");
    getProblem().addFVKernel(kernel_type, prefix() + _temperature_name + "_time", params);
  }
}

void
HeatConductionFV::addFVBCs()
{
  // We dont need to add anything for insulated boundaries, 0 flux is the default boundary condition
  if (isParamValid("heat_flux_boundaries"))
  {
    const std::string bc_type = "FVFunctorNeumannBC";
    InputParameters params = getFactory().getValidParams(bc_type);
    params.set<NonlinearVariableName>("variable") = _temperature_name;

    const auto & heat_flux_boundaries = getParam<std::vector<BoundaryName>>("heat_flux_boundaries");
    const auto & boundary_heat_fluxes =
        getParam<std::vector<MooseFunctorName>>("boundary_heat_fluxes");
    // Optimization if all the same
    if (std::set<MooseFunctorName>(boundary_heat_fluxes.begin(), boundary_heat_fluxes.end())
                .size() == 1 &&
        heat_flux_boundaries.size() > 1)
    {
      params.set<std::vector<BoundaryName>>("boundary") = heat_flux_boundaries;
      params.set<MooseFunctorName>("functor") = boundary_heat_fluxes[0];
      getProblem().addFVBC(bc_type, prefix() + _temperature_name + "_heat_flux_bc_all", params);
    }
    else
    {
      for (const auto i : index_range(heat_flux_boundaries))
      {
        params.set<std::vector<BoundaryName>>("boundary") = {heat_flux_boundaries[i]};
        params.set<MooseFunctorName>("functor") = boundary_heat_fluxes[i];
        getProblem().addFVBC(bc_type,
                             prefix() + _temperature_name + "_heat_flux_bc_" +
                                 heat_flux_boundaries[i],
                             params);
      }
    }
  }
  if (isParamValid("fixed_temperature_boundaries"))
  {
    const std::string bc_type = "FVFunctorDirichletBC";
    InputParameters params = getFactory().getValidParams(bc_type);
    params.set<NonlinearVariableName>("variable") = _temperature_name;

    const auto & temperature_boundaries =
        getParam<std::vector<BoundaryName>>("fixed_temperature_boundaries");
    const auto & boundary_temperatures =
        getParam<std::vector<MooseFunctorName>>("boundary_temperatures");
    // Optimization if all the same
    if (std::set<MooseFunctorName>(boundary_temperatures.begin(), boundary_temperatures.end())
                .size() == 1 &&
        temperature_boundaries.size() > 1)
    {
      params.set<std::vector<BoundaryName>>("boundary") = temperature_boundaries;
      params.set<MooseFunctorName>("functor") = boundary_temperatures[0];
      getProblem().addFVBC(bc_type, prefix() + _temperature_name + "_dirichlet_bc_all", params);
    }
    else
    {
      for (const auto i : index_range(temperature_boundaries))
      {
        params.set<std::vector<BoundaryName>>("boundary") = {temperature_boundaries[i]};
        params.set<MooseFunctorName>("functor") = boundary_temperatures[i];
        getProblem().addFVBC(bc_type,
                             prefix() + _temperature_name + "_dirichlet_bc_" +
                                 temperature_boundaries[i],
                             params);
      }
    }
  }
  if (isParamValid("fixed_convection_boundaries"))
  {
    const std::string bc_type = "FVFunctorConvectiveHeatFluxBC";
    InputParameters params = getFactory().getValidParams(bc_type);
    params.set<NonlinearVariableName>("variable") = _temperature_name;
    params.set<bool>("is_solid") = true;
    params.set<MooseFunctorName>("T_solid") = _temperature_name;

    const auto & convective_boundaries =
        getParam<std::vector<BoundaryName>>("fixed_convection_boundaries");
    const auto & boundary_T_fluid =
        getParam<std::vector<MooseFunctorName>>("fixed_convection_T_fluid");
    const auto & boundary_htc = getParam<std::vector<MooseFunctorName>>("fixed_convection_htc");
    // Optimization if all the same
    if (std::set<MooseFunctorName>(boundary_T_fluid.begin(), boundary_T_fluid.end()).size() == 1 &&
        std::set<MooseFunctorName>(boundary_htc.begin(), boundary_htc.end()).size() == 1 &&
        convective_boundaries.size() > 1)
    {
      params.set<std::vector<BoundaryName>>("boundary") = convective_boundaries;
      params.set<MooseFunctorName>("T_bulk") = boundary_T_fluid[0];
      params.set<MooseFunctorName>("heat_transfer_coefficient") = boundary_htc[0];
      getProblem().addFVBC(
          bc_type, prefix() + _temperature_name + "_fixed_convection_bc_all", params);
    }
    else
    {
      for (const auto i : index_range(convective_boundaries))
      {
        params.set<std::vector<BoundaryName>>("boundary") = {convective_boundaries[i]};
        params.set<MooseFunctorName>("T_bulk") = boundary_T_fluid[i];
        params.set<MooseFunctorName>("heat_transfer_coefficient") = boundary_htc[i];
        getProblem().addFVBC(bc_type,
                             prefix() + _temperature_name + "_fixed_convection_bc_" +
                                 convective_boundaries[i],
                             params);
      }
    }
  }
}

void
HeatConductionFV::addSolverVariables()
{
  if (variableExists(_temperature_name, /*error_if_aux=*/true))
    return;

  const std::string variable_type = "MooseVariableFVReal";
  InputParameters params = getFactory().getValidParams(variable_type);
  params.set<std::vector<Real>>("scaling") = {getParam<Real>("temperature_scaling")};
  params.set<SolverSystemName>("solver_sys") = getSolverSystem(_temperature_name);

  getProblem().addVariable(variable_type, _temperature_name, params);
}
