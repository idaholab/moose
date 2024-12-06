//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionFV.h"
#include "MooseUtils.h"

// Register the actions for the objects actually used
registerMooseAction("MooseApp", DiffusionFV, "add_fv_kernel");
registerMooseAction("MooseApp", DiffusionFV, "add_fv_bc");
registerMooseAction("MooseApp", DiffusionFV, "add_variable");
registerDiffusionPhysicsBaseTasks("MooseApp", DiffusionFV);

InputParameters
DiffusionFV::validParams()
{
  InputParameters params = DiffusionPhysicsBase::validParams();
  params.addClassDescription("Add diffusion physics discretized with cell-centered finite volume");
  // No kernel implemented in the framework for a material property diffusivity
  params.suppressParameter<MaterialPropertyName>("diffusivity_matprop");
  params.addParam<unsigned short>(
      "ghost_layers", 2, "Number of ghosting layers for distributed memory parallel calculations");
  params.addParamNamesToGroup("ghost_layers", "Advanced");
  return params;
}

DiffusionFV::DiffusionFV(const InputParameters & parameters) : DiffusionPhysicsBase(parameters) {}

void
DiffusionFV::initializePhysicsAdditional()
{
  getProblem().needFV();
}

void
DiffusionFV::addFVKernels()
{
  // Diffusion term
  {
    const std::string kernel_type = "FVDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<NonlinearVariableName>("variable") = _var_name;
    params.set<MooseFunctorName>("coeff") = isParamValid("diffusivity_functor")
                                                ? getParam<MooseFunctorName>("diffusivity_functor")
                                                : "1";
    getProblem().addFVKernel(kernel_type, prefix() + _var_name + "_diffusion", params);
  }
  // Source term
  if (isParamValid("source_functor"))
  {
    // Select the kernel type based on the user parameters
    std::string kernel_type;

    const auto & source = getParam<MooseFunctorName>("source_functor");
    if (MooseUtils::parsesToReal(source) || getProblem().hasFunction(source) ||
        getProblem().hasPostprocessorValueByName(source))
      kernel_type = "FVBodyForce";
    else if (getProblem().hasVariable(source))
      kernel_type = "FVCoupledForce";
    else
      paramError("source_functor",
                 "No kernel defined for a source term in FV for the type used for '",
                 source,
                 "'");

    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _var_name;
    assignBlocks(params, _blocks);

    // Transfer the source and coefficient parameter from the Physics to the kernel
    const auto coef = getParam<Real>("source_coef");
    if (MooseUtils::parsesToReal(source))
      params.set<Real>("value") = MooseUtils::convert<Real>(source) * coef;
    else if (getProblem().hasFunction(source))
    {
      params.set<Real>("value") = coef;
      params.set<FunctionName>("function") = source;
    }
    else if (getProblem().hasPostprocessorValueByName(source))
    {
      params.set<Real>("value") = coef;
      params.set<PostprocessorName>("postprocessor") = source;
    }
    else
    {
      params.set<Real>("coef") = coef;
      params.set<MooseFunctorName>("v") = {source};
    }

    getProblem().addFVKernel(kernel_type, prefix() + _var_name + "_source", params);
  }
  // Time derivative
  if (isTransient())
  {
    const std::string kernel_type = "FVTimeKernel";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _var_name;
    assignBlocks(params, _blocks);
    getProblem().addFVKernel(kernel_type, prefix() + _var_name + "_time", params);
  }
}

void
DiffusionFV::addFVBCs()
{
  if (isParamValid("neumann_boundaries"))
  {
    const auto & boundary_fluxes = getParam<std::vector<MooseFunctorName>>("boundary_fluxes");
    for (const auto i : index_range(_neumann_boundaries))
    {
      const auto & bc_flux = boundary_fluxes[i];
      // Select the boundary type based on the user parameters and what we know to be most efficient
      std::string bc_type = "";
      if (MooseUtils::parsesToReal(bc_flux))
        bc_type = "FVNeumannBC";
      else if (getProblem().hasFunction(bc_flux))
        bc_type = "FVFunctionNeumannBC";
      else
        bc_type = "FVFunctorNeumannBC";

      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _var_name;
      params.set<std::vector<BoundaryName>>("boundary") = {_neumann_boundaries[i]};

      // Set the boundary condition parameter for the specific boundary condition type used
      if (MooseUtils::parsesToReal(bc_flux))
        params.set<Real>("value") = MooseUtils::convert<Real>(bc_flux);
      else if (getProblem().hasFunction(bc_flux))
        params.set<FunctionName>("function") = bc_flux;
      else
        params.set<MooseFunctorName>("functor") = bc_flux;

      getProblem().addFVBC(
          bc_type, prefix() + _var_name + "_neumann_bc_" + _neumann_boundaries[i], params);
    }
  }

  if (isParamValid("dirichlet_boundaries"))
  {
    const auto & boundary_values = getParam<std::vector<MooseFunctorName>>("boundary_values");
    for (const auto i : index_range(_dirichlet_boundaries))
    {
      const auto & bc_value = boundary_values[i];
      // Select the boundary type based on the user parameters and what we know to be most efficient
      std::string bc_type = "";
      if (MooseUtils::parsesToReal(bc_value))
        bc_type = "FVDirichletBC";
      else if (getProblem().hasFunction(bc_value))
        bc_type = "FVFunctionDirichletBC";
      else
        bc_type = "FVADFunctorDirichletBC";

      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _var_name;
      params.set<std::vector<BoundaryName>>("boundary") = {_dirichlet_boundaries[i]};

      // Set the boundary condition parameter for the specific boundary condition type used
      if (MooseUtils::parsesToReal(bc_value))
        params.set<Real>("value") = MooseUtils::convert<Real>(bc_value);
      else if (getProblem().hasFunction(bc_value))
        params.set<FunctionName>("function") = bc_value;
      else
        params.set<MooseFunctorName>("functor") = bc_value;

      getProblem().addFVBC(
          bc_type, prefix() + _var_name + "_dirichlet_bc_" + _dirichlet_boundaries[i], params);
    }
  }
}

void
DiffusionFV::addSolverVariables()
{
  if (variableExists(_var_name, true))
    return;

  const std::string variable_type = "MooseVariableFVReal";
  InputParameters params = getFactory().getValidParams(variable_type);
  assignBlocks(params, _blocks);
  params.set<SolverSystemName>("solver_sys") = getSolverSystem(_var_name);

  // TODO: Do we need to use a different variable name maybe?
  // Or add API to extend block definition of a boundary
  getProblem().addVariable(variable_type, _var_name, params);
}

InputParameters
DiffusionFV::getAdditionalRMParams() const
{
  const auto necessary_layers =
      std::max(getParam<unsigned short>("ghost_layers"), (unsigned short)2);

  // Just an object that has a ghost_layers parameter
  const std::string kernel_type = "FVDiffusion";
  InputParameters params = getFactory().getValidParams(kernel_type);
  params.template set<unsigned short>("ghost_layers") = necessary_layers;

  return params;
}
