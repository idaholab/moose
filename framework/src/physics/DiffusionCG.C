//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionCG.h"
#include "MooseVariableBase.h"

// Register the actions for the objects actually used
registerMooseAction("MooseApp", DiffusionCG, "add_kernel");
registerMooseAction("MooseApp", DiffusionCG, "add_bc");
registerMooseAction("MooseApp", DiffusionCG, "add_variable");
registerDiffusionPhysicsBaseTasks("MooseApp", DiffusionCG);

InputParameters
DiffusionCG::validParams()
{
  InputParameters params = DiffusionPhysicsBase::validParams();
  params.addClassDescription(
      "Discretizes a diffusion equation with the continuous Galerkin finite element method");
  params.addParam<bool>(
      "use_automatic_differentiation",
      true,
      "Whether to use automatic differentiation for all the terms in the equation");
  params.transferParam<MooseEnum>(MooseVariableBase::validParams(), "order", "variable_order");

  return params;
}

DiffusionCG::DiffusionCG(const InputParameters & parameters)
  : DiffusionPhysicsBase(parameters), _use_ad(getParam<bool>("use_automatic_differentiation"))
{
}

void
DiffusionCG::addFEKernels()
{
  // Diffusion term
  {
    // Select the kernel type based on the user parameters
    std::string kernel_type;
    if (isParamValid("diffusivity_matprop"))
      kernel_type = _use_ad ? "ADMatDiffusion" : "MatDiffusion";
    else if (isParamValid("diffusivity_functor"))
    {
      const auto & d = getParam<MooseFunctorName>("diffusivity_functor");
      if (getProblem().hasFunction(d))
        kernel_type = "FunctionDiffusion";
      else
        paramError(
            "diffusivity_functor", "No diffusion kernel implemented for the source type of", d);
    }
    else
      kernel_type = _use_ad ? "ADDiffusion" : "Diffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _var_name;
    assignBlocks(params, _blocks);

    // Transfer the diffusivity parameter from the Physics to the kernel
    if (isParamValid("diffusivity_matprop"))
      params.set<MaterialPropertyName>("diffusivity") =
          getParam<MaterialPropertyName>("diffusivity_matprop");
    else if (isParamValid("diffusivity_functor"))
      params.set<MaterialPropertyName>("diffusivity") =
          getParam<MaterialPropertyName>("diffusivity_matprop");

    getProblem().addKernel(kernel_type, prefix() + _var_name + "_diffusion", params);
  }

  // Source term
  if (isParamValid("source_functor"))
  {
    // Select the kernel type based on the user parameters
    std::string kernel_type;
    const auto & source = getParam<MooseFunctorName>("source_functor");
    if (MooseUtils::parsesToReal(source) || getProblem().hasFunction(source) ||
        getProblem().hasPostprocessorValueByName(source))
      kernel_type = _use_ad ? "ADBodyForce" : "BodyForce";
    else if (getProblem().hasVariable(source))
      kernel_type = _use_ad ? "ADCoupledForce" : "CoupledForce";
    else
      paramError("source_functor",
                 "No kernel defined for a source term in CG for the type of '",
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
      params.set<std::vector<VariableName>>("v") = {source};
    }

    getProblem().addKernel(kernel_type, prefix() + _var_name + "_source", params);
  }

  // Time derivative term
  if (isTransient())
  {
    const std::string kernel_type = _use_ad ? "ADTimeDerivative" : "TimeDerivative";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _var_name;
    assignBlocks(params, _blocks);
    getProblem().addKernel(kernel_type, prefix() + _var_name + "_time", params);
  }
}

void
DiffusionCG::addFEBCs()
{
  if (isParamSetByUser("neumann_boundaries"))
  {
    const auto & boundary_fluxes = getParam<std::vector<MooseFunctorName>>("boundary_fluxes");
    for (const auto i : index_range(_neumann_boundaries))
    {
      const auto & bc_flux = boundary_fluxes[i];
      // Select the boundary type based on the user parameters and what we know to be most efficient
      // We could actually just use the very last option for everything but the performance is
      // better if one uses the specialized objects
      std::string bc_type = "";
      if (MooseUtils::parsesToReal(bc_flux))
        bc_type = _use_ad ? "ADNeumannBC" : "NeumannBC";
      else if (getProblem().hasVariable(bc_flux))
        bc_type = "CoupledVarNeumannBC"; // not AD, but still perfect Jacobian
      else if (getProblem().hasFunction(bc_flux))
        bc_type = _use_ad ? "ADFunctionNeumannBC" : "FunctionNeumannBC";
      else if (getProblem().hasPostprocessorValueByName(bc_flux))
        bc_type = "PostprocessorNeumannBC";
      else // this is AD, but we can mix AD and non-AD
        bc_type = "FunctorNeumannBC";

      // Get the parameters for the object type chosen and set the common parameters
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _var_name;
      params.set<std::vector<BoundaryName>>("boundary") = {_neumann_boundaries[i]};

      // Set the flux parameter for the specific type of NeumannBC used
      if (MooseUtils::parsesToReal(bc_flux))
        params.set<Real>("value") = MooseUtils::convert<Real>(bc_flux);
      else if (getProblem().hasVariable(bc_flux))
        params.set<std::vector<VariableName>>("v") = {bc_flux};
      else if (getProblem().hasFunction(bc_flux))
        params.set<FunctionName>("function") = bc_flux;
      else if (getProblem().hasPostprocessorValueByName(bc_flux))
        params.set<PostprocessorName>("postprocessor") = bc_flux;
      else
        params.set<MooseFunctorName>("functor") = bc_flux;

      getProblem().addBoundaryCondition(
          bc_type, prefix() + _var_name + "_neumann_bc_" + _neumann_boundaries[i], params);
    }
  }
  if (isParamSetByUser("dirichlet_boundaries"))
  {
    const auto & boundary_values = getParam<std::vector<MooseFunctorName>>("boundary_values");
    for (const auto i : index_range(_dirichlet_boundaries))
    {
      const auto & bc_value = boundary_values[i];
      // Select the boundary type based on the user parameters and what we know to be most efficient
      std::string bc_type = "";
      if (MooseUtils::parsesToReal(bc_value))
        bc_type = _use_ad ? "ADDirichletBC" : "DirichletBC";
      else if (getProblem().hasVariable(bc_value))
        bc_type = _use_ad ? "ADMatchedValueBC" : "MatchedValueBC";
      else if (getProblem().hasFunction(bc_value))
        bc_type = _use_ad ? "ADFunctionDirichletBC" : "FunctionDirichletBC";
      else if (getProblem().hasPostprocessorValueByName(bc_value))
        bc_type = "PostprocessorDirichletBC";
      else // this is AD, but we can mix AD and non-AD
        bc_type = "FunctorDirichletBC";

      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _var_name;
      params.set<std::vector<BoundaryName>>("boundary") = {_dirichlet_boundaries[i]};

      // Set the flux parameter for the specific type of DirichletBC used
      if (MooseUtils::parsesToReal(bc_value))
        params.set<Real>("value") = MooseUtils::convert<Real>(bc_value);
      else if (getProblem().hasVariable(bc_value))
        params.set<std::vector<VariableName>>("v") = {bc_value};
      else if (getProblem().hasFunction(bc_value))
        params.set<FunctionName>("function") = bc_value;
      else if (getProblem().hasPostprocessorValueByName(bc_value))
        params.set<PostprocessorName>("postprocessor") = bc_value;
      else
        params.set<MooseFunctorName>("functor") = bc_value;

      getProblem().addBoundaryCondition(
          bc_type, prefix() + _var_name + "_dirichlet_bc_" + _dirichlet_boundaries[i], params);
    }
  }
}

void
DiffusionCG::addSolverVariables()
{
  // If the variable was added outside the Physics
  if (variableExists(_var_name, /*error_if_aux*/ true))
  {
    if (isParamValid("variable_order"))
      paramError("variable_order",
                 "Cannot specify the variable order if variable " + _var_name +
                     " is defined outside the Physics block");
    else
      return;
  }

  const std::string variable_type = "MooseVariable";
  InputParameters params = getFactory().getValidParams(variable_type);
  params.set<MooseEnum>("order") = getParam<MooseEnum>("variable_order");
  assignBlocks(params, _blocks);
  params.set<SolverSystemName>("solver_sys") = getSolverSystem(_var_name);

  getProblem().addVariable(variable_type, _var_name, params);
}
