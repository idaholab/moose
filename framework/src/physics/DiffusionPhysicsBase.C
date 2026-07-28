//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionPhysicsBase.h"
#include "PetscSupport.h"
#include "MooseEnumItem.h"

InputParameters
DiffusionPhysicsBase::validParams()
{
  InputParameters params = PhysicsBase::validParams();
  params += PhysicsComponentInterface::validParams();
  params.addClassDescription("Base class for creating a diffusion equation");

  // Variable parameters
  params.addParam<VariableName>("variable_name", "u", "Variable name for the equation");
  params.addParam<FunctionName>("initial_condition", "Initial condition for the diffused variable");

  // Diffusivity
  params.addParam<std::vector<MaterialPropertyName>>(
      "diffusivity_matprop",
      "Material property(ies) defining the diffusion coefficient (on each group of blocks)");
  params.addParam<std::vector<MooseFunctorName>>(
      "diffusivity_functor", "Functor(s) specifying the diffusivity (on each group of blocks)");
  params.addParam<std::vector<std::vector<SubdomainName>>>(
      "diffusivity_blocks", "Subdomains on which each diffusivity is defined");

  // Source term
  params.addParam<MooseFunctorName>("source_functor", "Source term in the diffusion problem");
  params.addParam<Real>("source_coef", 1, "Coefficient multiplying the source");

  // Boundary conditions
  params.addParam<std::vector<BoundaryName>>(
      "neumann_boundaries", {}, "Boundaries on which to apply a diffusive flux");
  params.addParam<std::vector<BoundaryName>>(
      "dirichlet_boundaries", {}, "Boundaries on which to apply a fixed value");
  params.addParam<std::vector<MooseFunctorName>>(
      "boundary_fluxes", {}, "Functors to compute the diffusive flux on each Neumann boundary'");
  params.addParam<std::vector<MooseFunctorName>>(
      "boundary_values", {}, "Functors to compute the diffusive flux on each Dirichlet boundary'");
  params.addParamNamesToGroup("neumann_boundaries dirichlet_boundaries boundary_fluxes "
                              "boundary_values",
                              "Boundary conditions");

  // Postprocessing
  params.addParam<std::vector<BoundaryName>>(
      "compute_diffusive_fluxes_on", {}, "Surfaces to compute the diffusive flux on");

  // Preconditioning is implemented so let's use it by default
  MooseEnum pc_options("default defer", "default");
  params.addParam<MooseEnum>(
      "preconditioning", pc_options, "Which preconditioning to use for this Physics");

  return params;
}

DiffusionPhysicsBase::DiffusionPhysicsBase(const InputParameters & parameters)
  : PhysicsBase(parameters),
    PhysicsComponentInterface(parameters),
    _var_name(getParam<VariableName>("variable_name")),
    _neumann_boundaries(getParam<std::vector<BoundaryName>>("neumann_boundaries")),
    _dirichlet_boundaries(getParam<std::vector<BoundaryName>>("dirichlet_boundaries"))
{
  // Keep track of variables
  saveSolverVariableName(_var_name);

  // Parameter checking
  checkVectorParamsSameLength<BoundaryName, MooseFunctorName>("neumann_boundaries",
                                                              "boundary_fluxes");
  checkVectorParamsSameLength<BoundaryName, MooseFunctorName>("dirichlet_boundaries",
                                                              "boundary_values");
  checkVectorParamsNoOverlap<BoundaryName>({"neumann_boundaries", "dirichlet_boundaries"});
  if (isParamSetByUser("source_coef"))
    checkParamsBothSetOrNotSet("source_functor", "source_coef");
  if (isParamValid("diffusivity_matprop"))
    checkVectorParamsSameLength<MaterialPropertyName, std::vector<SubdomainName>>(
        "diffusivity_matprop", "diffusivity_blocks");
  else if (isParamValid("diffusivity_functor"))
    checkVectorParamsSameLength<MooseFunctorName, std::vector<SubdomainName>>("diffusivity_functor",
                                                                              "diffusivity_blocks");

  addRequiredPhysicsTask("add_preconditioning");
  addRequiredPhysicsTask("add_postprocessor");
  addRequiredPhysicsTask("add_ics_physics");
}

void
DiffusionPhysicsBase::addPreconditioning()
{
  // Use a multigrid method, known to work for elliptic problems such as diffusion
  if (_preconditioning == "default")
  {
    // We only pass petsc options as that's all that's needed to set up the preconditioner
    const auto option_pair1 =
        std::make_pair<MooseEnumItem, std::string>(MooseEnumItem("-pc_type"), "hypre");
    const auto option_pair2 =
        std::make_pair<MooseEnumItem, std::string>(MooseEnumItem("-pc_hypre_type"), "boomeramg");
    addPetscPairsToPetscOptions({option_pair1, option_pair2});
  }
}

void
DiffusionPhysicsBase::addPostprocessors()
{
  for (const auto & boundary_name :
       getParam<std::vector<BoundaryName>>("compute_diffusive_fluxes_on"))
  {
    // Create the boundary integration of the flux
    const bool use_ad = isParamValid("use_automatic_differentiation")
                            ? getParam<bool>("use_automatic_differentiation")
                            : false;
    const std::string pp_type =
        use_ad ? "ADSideDiffusiveFluxIntegral" : "SideDiffusiveFluxIntegral";
    auto params = _factory.getValidParams(pp_type);
    params.set<std::vector<VariableName>>("variable") = {_var_name};
    if (isParamValid("diffusivity_matprop"))
    {
      const auto & diffusivities =
          getParam<std::vector<MaterialPropertyName>>("diffusivity_matprop");
      if (diffusivities.size() != 1)
        paramError("diffusivity_matprop",
                   "Computing diffusive fluxes is not implemented for multiple diffusivities");
      params.set<MaterialPropertyName>("diffusivity") = diffusivities[0];
    }
    else if (isParamValid("diffusivity_functor"))
    {
      const auto & diffusivities = getParam<std::vector<MooseFunctorName>>("diffusivity_functor");
      if (diffusivities.size() != 1)
        paramError("diffusivity_functor",
                   "Computing diffusive fluxes is not implemented for multiple diffusivities");
      params.set<MooseFunctorName>("functor_diffusivity") = diffusivities[0];
    }
    else
      params.set<MooseFunctorName>("functor_diffusivity") = "1";
    params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};
    // Default to maximum computation
    params.set<ExecFlagEnum>("execute_on") = {
        EXEC_INITIAL, EXEC_TIMESTEP_END, EXEC_NONLINEAR, EXEC_LINEAR};
    getProblem().addPostprocessor(pp_type, prefix() + "diffusive_flux_" + boundary_name, params);
  }
}

void
DiffusionPhysicsBase::addInitialConditions()
{
  InputParameters params = getFactory().getValidParams("FunctionIC");

  // Get the list of blocks that have ics from components
  std::vector<SubdomainName> component_ic_blocks;
  for (const auto & [component_name, component_bc_map] : _components_initial_conditions)
  {
    if (!component_bc_map.count(_var_name))
      continue;
    const auto & comp_blocks = getActionComponent(component_name).blocks();
    component_ic_blocks.insert(component_ic_blocks.end(), comp_blocks.begin(), comp_blocks.end());
  }

  // Keep only blocks that have no component IC
  std::vector<SubdomainName> remaining_blocks;
  for (const auto & block : _blocks)
    if (std::find(component_ic_blocks.begin(), component_ic_blocks.end(), block) ==
        component_ic_blocks.end())
      remaining_blocks.push_back(block);

  // No need to add BCs on the Physics block restriction if Components are covering all of it
  if (remaining_blocks.empty())
    return;
  assignBlocks(params, remaining_blocks);

  // first obey any component-specific initial condition
  // then obey the user specification of initial conditions
  // NOTE: we may conflict with ICs in the input
  // there are no default initial conditions
  mooseAssert(parameters().isParamSetByUser("initial_condition") ||
                  !parameters().hasDefault("initial_condition"),
              "Should not have a default");
  if (isParamValid("initial_condition") &&
      shouldCreateIC(
          _var_name, remaining_blocks, /*ic is a default*/ false, /*error if defined*/ true))
  {
    params.set<VariableName>("variable") = _var_name;
    params.set<FunctionName>("function") = getParam<FunctionName>("initial_condition");

    getProblem().addInitialCondition("FunctionIC", prefix() + _var_name + "_ic", params);
  }
}

void
DiffusionPhysicsBase::addInitialConditionsFromComponents()
{
  InputParameters params = getFactory().getValidParams("FunctorIC");

  // ICs from components are considered always set by the user, so we do not skip them when
  // restarting
  for (const auto & [component_name, component_bc_map] : _components_initial_conditions)
  {
    if (!component_bc_map.count(_var_name))
      continue;
    assignBlocks(params, getActionComponent(component_name).blocks());
    params.set<VariableName>("variable") = _var_name;
    params.set<MooseFunctorName>("functor") = libmesh_map_find(component_bc_map, _var_name);

    getProblem().addInitialCondition(
        "FunctorIC", prefix() + _var_name + "_ic_" + component_name, params);
  }
}
