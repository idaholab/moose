//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiSpeciesDiffusionCG.h"
#include "MooseVariableBase.h"

// Register the actions for the objects actually used
registerMooseAction("ScalarTransportApp", MultiSpeciesDiffusionCG, "add_kernel");
registerMooseAction("ScalarTransportApp", MultiSpeciesDiffusionCG, "add_bc");
registerMooseAction("ScalarTransportApp", MultiSpeciesDiffusionCG, "add_variable");
registerMultiSpeciesDiffusionPhysicsBaseTasks("ScalarTransportApp", MultiSpeciesDiffusionCG);

InputParameters
MultiSpeciesDiffusionCG::validParams()
{
  InputParameters params = MultiSpeciesDiffusionPhysicsBase::validParams();
  params.addClassDescription("Discretizes diffusion equations for several species with the "
                             "continuous Galerkin finite element method");
  params.transferParam<MooseEnum>(MooseVariableBase::validParams(), "order", "variable_order");

  return params;
}

MultiSpeciesDiffusionCG::MultiSpeciesDiffusionCG(const InputParameters & parameters)
  : MultiSpeciesDiffusionPhysicsBase(parameters)
{
}

void
MultiSpeciesDiffusionCG::addFEKernels()
{
  for (const auto s : index_range(_species_names))
  {
    const auto & var_name = _species_names[s];
    // Diffusion term
    {
      // Select the kernel type based on the user parameters
      std::string kernel_type;
      if (isParamValid("diffusivity_matprops"))
        kernel_type = _use_ad ? "ADMatDiffusion" : "MatDiffusion";
      else if (isParamValid("diffusivity_functors"))
      {
        const auto & d = getParam<std::vector<MooseFunctorName>>("diffusivity_functors")[s];
        if (getProblem().hasFunction(d))
          kernel_type = "FunctionDiffusion";
        else
          paramError(
              "diffusivity_functors", "No diffusion kernel implemented for the source type of", d);
      }
      else
        kernel_type = _use_ad ? "ADDiffusion" : "Diffusion";
      InputParameters params = getFactory().getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = var_name;
      assignBlocks(params, _blocks);

      // Transfer the diffusivity parameter from the Physics to the kernel
      if (isParamValid("diffusivity_matprops"))
        params.set<MaterialPropertyName>("diffusivity") =
            getParam<std::vector<MaterialPropertyName>>("diffusivity_matprops")[s];
      else if (isParamValid("diffusivity_functors"))
        params.set<FunctionName>("function") =
            getParam<std::vector<MooseFunctorName>>("diffusivity_functors")[s];

      getProblem().addKernel(kernel_type, prefix() + var_name + "_diffusion", params);
    }

    // Source term
    if (isParamValid("source_functors"))
    {
      // Select the kernel type based on the user parameters
      std::string kernel_type;
      const auto & sources = getParam<std::vector<MooseFunctorName>>("source_functors");
      const auto & source = sources[s];
      if (MooseUtils::parsesToReal(source) || getProblem().hasFunction(source) ||
          getProblem().hasPostprocessorValueByName(source))
        kernel_type = _use_ad ? "ADBodyForce" : "BodyForce";
      else if (getProblem().hasVariable(source))
        kernel_type = _use_ad ? "ADCoupledForce" : "CoupledForce";
      else
        paramError("source_functors",
                   "No kernel defined for a source term in CG for the type of '",
                   source,
                   "'");

      InputParameters params = getFactory().getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = var_name;
      assignBlocks(params, _blocks);

      // Transfer the source and coefficient parameter from the Physics to the kernel
      const auto coefs = getParam<std::vector<Real>>("source_coefs");
      const auto coef = coefs[s];
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

      getProblem().addKernel(kernel_type, prefix() + var_name + "_source", params);
    }

    // Time derivative term
    if (isTransient())
    {
      const std::string kernel_type = _use_ad ? "ADTimeDerivative" : "TimeDerivative";
      InputParameters params = getFactory().getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = var_name;
      assignBlocks(params, _blocks);
      getProblem().addKernel(kernel_type, prefix() + var_name + "_time", params);
    }
  }
}

void
MultiSpeciesDiffusionCG::addFEBCs()
{
  if (isParamSetByUser("neumann_boundaries"))
  {
    const auto & boundary_fluxes =
        getParam<std::vector<std::vector<MooseFunctorName>>>("boundary_fluxes");

    for (const auto s : index_range(_species_names))
    {
      const auto & var_name = _species_names[s];

      for (const auto i : index_range(_neumann_boundaries[s]))
      {
        const auto & bc_flux = boundary_fluxes[s][i];
        // Select the boundary type based on the user parameters and what we know to be most
        // efficient We could actually just use the very last option for everything but the
        // performance is better if one uses the specialized objects
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
        params.set<NonlinearVariableName>("variable") = var_name;
        params.set<std::vector<BoundaryName>>("boundary") = {_neumann_boundaries[s][i]};

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
            bc_type, prefix() + var_name + "_neumann_bc_" + _neumann_boundaries[s][i], params);
      }
    }
  }
  if (isParamSetByUser("dirichlet_boundaries"))
  {
    const auto & boundary_values =
        getParam<std::vector<std::vector<MooseFunctorName>>>("boundary_values");
    for (const auto s : index_range(_species_names))
    {
      const auto & var_name = _species_names[s];
      for (const auto i : index_range(_dirichlet_boundaries[s]))
      {
        const auto & bc_value = boundary_values[s][i];
        // Select the boundary type based on the user parameters and what we know to be most
        // efficient
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
        params.set<NonlinearVariableName>("variable") = var_name;
        params.set<std::vector<BoundaryName>>("boundary") = {_dirichlet_boundaries[s][i]};

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
            bc_type, prefix() + var_name + "_dirichlet_bc_" + _dirichlet_boundaries[s][i], params);
      }
    }
  }
}

void
MultiSpeciesDiffusionCG::addSolverVariables()
{
  for (const auto & var_name : _species_names)
  {
    // If the variable was added outside the Physics
    if (variableExists(var_name, /*error_if_aux*/ true))
    {
      if (isParamValid("variable_order"))
        paramError("variable_order",
                   "Cannot specify the variable order if variable " + var_name +
                       " is defined outside the Physics block");
      else
        return;
    }

    const std::string variable_type = "MooseVariable";
    InputParameters params = getFactory().getValidParams(variable_type);
    params.set<MooseEnum>("order") = getParam<MooseEnum>("variable_order");
    assignBlocks(params, _blocks);
    params.set<SolverSystemName>("solver_sys") = getSolverSystem(var_name);

    getProblem().addVariable(variable_type, var_name, params);
  }
}
