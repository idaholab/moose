//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatConductionCG.h"
#include "ADHeatConduction.h"
#include "HeatConductionTimeDerivative.h"

// Register the actions for the objects actually used
registerPhysicsBaseTasks("HeatTransferApp", HeatConductionCG);
registerMooseAction("HeatTransferApp", HeatConductionCG, "add_kernel");
registerMooseAction("HeatTransferApp", HeatConductionCG, "add_bc");
registerMooseAction("HeatTransferApp", HeatConductionCG, "add_variable");
registerMooseAction("HeatTransferApp", HeatConductionCG, "add_ic");
registerMooseAction("HeatTransferApp", HeatConductionCG, "add_preconditioning");

InputParameters
HeatConductionCG::validParams()
{
  InputParameters params = HeatConductionPhysicsBase::validParams();
  params.addClassDescription("Creates the heat conduction equation discretized with CG");

  // Material properties
  params.transferParam<MaterialPropertyName>(ADHeatConduction::validParams(),
                                             "thermal_conductivity");
  params.transferParam<MaterialPropertyName>(HeatConductionTimeDerivative::validParams(),
                                             "specific_heat");
  params.addParam<MaterialPropertyName>("density", "density", "Density material property");
  params.addParamNamesToGroup("thermal_conductivity specific_heat density", "Thermal properties");

  return params;
}

HeatConductionCG::HeatConductionCG(const InputParameters & parameters)
  : HeatConductionPhysicsBase(parameters)
{
}

void
HeatConductionCG::addFEKernels()
{
  {
    const std::string kernel_type = "ADHeatConduction";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _temperature_name;
    params.applyParameter(parameters(), "thermal_conductivity");
    getProblem().addKernel(kernel_type, prefix() + _temperature_name + "_conduction", params);
  }
  if (isParamValid("heat_source_var"))
  {
    const std::string kernel_type = "ADCoupledForce";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _temperature_name;
    params.set<std::vector<VariableName>>("v") = {getParam<VariableName>("heat_source_var")};
    if (isParamValid("heat_source_blocks"))
      params.set<std::vector<SubdomainName>>("block") =
          getParam<std::vector<SubdomainName>>("heat_source_blocks");
    getProblem().addKernel(kernel_type, prefix() + _temperature_name + "_source", params);
  }
  if (isParamValid("heat_source_functor"))
  {
    const std::string kernel_type = "BodyForce";
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
      paramError("heat_source_functor", "Unsupported functor type.");
    getProblem().addKernel(kernel_type, prefix() + _temperature_name + "_source_functor", params);
  }
  if (isTransient())
  {
    const std::string kernel_type = "ADHeatConductionTimeDerivative";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _temperature_name;
    params.applyParameter(parameters(), "density_name");
    params.applyParameter(parameters(), "specific_heat");
    getProblem().addKernel(kernel_type, prefix() + _temperature_name + "_time", params);
  }
}

void
HeatConductionCG::addFEBCs()
{
  // We dont need to add anything for insulated boundaries, 0 flux is the default boundary condition
  if (isParamValid("heat_flux_boundaries"))
  {
    const std::string bc_type = "FunctorNeumannBC";
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
      getProblem().addBoundaryCondition(
          bc_type, prefix() + _temperature_name + "_heat_flux_bc_all", params);
    }
    else
    {
      for (const auto i : index_range(heat_flux_boundaries))
      {
        params.set<std::vector<BoundaryName>>("boundary") = {heat_flux_boundaries[i]};
        params.set<MooseFunctorName>("functor") = boundary_heat_fluxes[i];
        getProblem().addBoundaryCondition(bc_type,
                                          prefix() + _temperature_name + "_heat_flux_bc_" +
                                              heat_flux_boundaries[i],
                                          params);
      }
    }
  }
  if (isParamValid("fixed_temperature_boundaries"))
  {
    const std::string bc_type = "FunctorDirichletBC";
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
      getProblem().addBoundaryCondition(
          bc_type, prefix() + _temperature_name + "_dirichlet_bc_all", params);
    }
    else
    {
      for (const auto i : index_range(temperature_boundaries))
      {
        params.set<std::vector<BoundaryName>>("boundary") = {temperature_boundaries[i]};
        params.set<MooseFunctorName>("functor") = boundary_temperatures[i];
        getProblem().addBoundaryCondition(bc_type,
                                          prefix() + _temperature_name + "_dirichlet_bc_" +
                                              temperature_boundaries[i],
                                          params);
      }
    }
  }
  if (isParamValid("fixed_convection_boundaries"))
  {
    const std::string bc_type = "ADConvectiveHeatFluxBC";
    InputParameters params = getFactory().getValidParams(bc_type);
    params.set<NonlinearVariableName>("variable") = _temperature_name;

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
      params.set<MooseFunctorName>("T_infinity_functor") = boundary_T_fluid[0];
      params.set<MooseFunctorName>("heat_transfer_coefficient_functor") = boundary_htc[0];
      getProblem().addBoundaryCondition(
          bc_type, prefix() + _temperature_name + "_fixed_convection_bc_all", params);
    }
    else
    {
      // Check sizes
      if (convective_boundaries.size() != boundary_T_fluid.size())
        paramError("fixed_convection_T_fluid",
                   "Should be as many convection boundaries (" +
                       std::to_string(convective_boundaries.size()) +
                       ") as fixed convection temperatures (" +
                       std::to_string(boundary_T_fluid.size()) + ")");
      if (convective_boundaries.size() != boundary_htc.size())
        paramError("fixed_convection_htc",
                   "Should be as many convection boundaries (" +
                       std::to_string(convective_boundaries.size()) +
                       ") as fixed convection heat exchange coefficients (" +
                       std::to_string(boundary_htc.size()) + ")");
      for (const auto i : index_range(convective_boundaries))
      {
        params.set<std::vector<BoundaryName>>("boundary") = {convective_boundaries[i]};
        params.set<MooseFunctorName>("T_infinity_functor") = boundary_T_fluid[i];
        params.set<MooseFunctorName>("heat_transfer_coefficient_functor") = boundary_htc[i];
        getProblem().addBoundaryCondition(bc_type,
                                          prefix() + _temperature_name + "_fixed_convection_bc_" +
                                              convective_boundaries[i],
                                          params);
      }
    }
  }
}

void
HeatConductionCG::addSolverVariables()
{
  if (variableExists(_temperature_name, /*error_if_aux=*/true))
    return;

  const std::string variable_type = "MooseVariable";
  // defaults to linear lagrange FE family
  InputParameters params = getFactory().getValidParams(variable_type);
  params.set<SolverSystemName>("solver_sys") = getSolverSystem(_temperature_name);

  getProblem().addVariable(variable_type, _temperature_name, params);
}
