//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiSpeciesDiffusionPhysicsBase.h"
#include "MatDiffusion.h"
#include "MoosePreconditioner.h"
#include "PetscSupport.h"
#include "MooseEnumItem.h"
#include "ActionComponent.h"

InputParameters
MultiSpeciesDiffusionPhysicsBase::validParams()
{
  InputParameters params = PhysicsBase::validParams();
  params.addClassDescription(
      "Base class for creating a diffusion equation for multiple diffused species");

  params.addRequiredParam<std::vector<VariableName>>("species", "Species being diffused");

  // Diffusivity
  params.addParam<std::vector<MaterialPropertyName>>(
      "diffusivity_matprops",
      "Material properties defining the diffusion coefficient for each species");
  params.addParam<std::vector<MooseFunctorName>>(
      "diffusivity_functors", "Functors specifying the diffusivity for each species");

  // Source term
  params.addParam<std::vector<MooseFunctorName>>(
      "source_functors", "Source terms in the diffusion problem for each species");
  params.addParam<std::vector<Real>>("source_coefs", {1}, "Coefficient multiplying the source");

  // Boundary conditions
  params.addParam<std::vector<std::vector<BoundaryName>>>(
      "neumann_boundaries", {}, "Boundaries on which to apply a diffusive flux for each species");
  params.addParam<std::vector<std::vector<BoundaryName>>>(
      "dirichlet_boundaries", {}, "Boundaries on which to apply a fixed value for each species");
  params.addParam<std::vector<std::vector<MooseFunctorName>>>(
      "boundary_fluxes",
      {},
      "Functors to compute the diffusive flux on each Neumann boundary for each species");
  params.addParam<std::vector<std::vector<MooseFunctorName>>>(
      "boundary_values",
      {},
      "Functors to compute the diffusive flux on each Dirichlet boundary for each species");
  params.addParamNamesToGroup("neumann_boundaries dirichlet_boundaries boundary_fluxes "
                              "boundary_values",
                              "Boundary conditions");

  // Initial conditions
  params.addParam<std::vector<FunctionName>>(
      "initial_conditions_species", "Functions describing the initial conditions for the species");

  // Postprocessing
  params.addParam<std::vector<BoundaryName>>(
      "compute_diffusive_fluxes_on", {}, "Surfaces to compute the diffusive flux on");

  // Preconditioning is implemented so let's use it by default
  MooseEnum pc_options("default none", "default");
  params.addParam<MooseEnum>(
      "preconditioning", pc_options, "Which preconditioning to use for this Physics");

  params.addParam<bool>(
      "use_automatic_differentiation",
      true,
      "Whether to use automatic differentiation for all the terms in the equation");

  return params;
}

MultiSpeciesDiffusionPhysicsBase::MultiSpeciesDiffusionPhysicsBase(
    const InputParameters & parameters)
  : PhysicsBase(parameters),
    _species_names(getParam<std::vector<VariableName>>("species")),
    _num_species(_species_names.size()),
    _neumann_boundaries(getParam<std::vector<std::vector<BoundaryName>>>("neumann_boundaries")),
    _dirichlet_boundaries(getParam<std::vector<std::vector<BoundaryName>>>("dirichlet_boundaries")),
    _use_ad(getParam<bool>("use_automatic_differentiation"))
{
  // Keep track of variables
  for (const auto & var_name : _species_names)
    saveSolverVariableName(var_name);

  // Parameter checking
  checkTwoDVectorParamsSameLength<BoundaryName, MooseFunctorName>("neumann_boundaries",
                                                                  "boundary_fluxes");
  checkTwoDVectorParamsSameLength<BoundaryName, MooseFunctorName>("dirichlet_boundaries",
                                                                  "boundary_values");
  checkTwoDVectorParamsNoRespectiveOverlap<BoundaryName>(
      {"neumann_boundaries", "dirichlet_boundaries"});
  if (isParamSetByUser("source_coefs"))
    checkParamsBothSetOrNotSet("source_functors", "source_coefs");
  if (isParamValid("source_functors"))
    checkVectorParamsSameLength<VariableName, MooseFunctorName>("species", "source_functors");
  if (isParamValid("initial_conditions_species"))
    checkVectorParamsSameLength<VariableName, FunctionName>("species",
                                                            "initial_conditions_species");

  addRequiredPhysicsTask("add_preconditioning");
  addRequiredPhysicsTask("add_ic");
}

void
MultiSpeciesDiffusionPhysicsBase::addPreconditioning()
{
  // Use a multi-grid method, known to work for elliptic problems such as diffusion
  if (_preconditioning == "default")
  {
    // We only pass petsc options as that's all that's needed to set up the preconditioner
    Moose::PetscSupport::PetscOptions & po = _problem->getPetscOptions();
    const auto option_pair1 =
        std::make_pair<MooseEnumItem, std::string>(MooseEnumItem("-pc_type"), "hypre");
    const auto option_pair2 =
        std::make_pair<MooseEnumItem, std::string>(MooseEnumItem("-pc_hypre_type"), "boomeramg");
    processPetscPairs({option_pair1, option_pair2}, _problem->mesh().dimension(), po);
  }
}

void
MultiSpeciesDiffusionPhysicsBase::addPostprocessors()
{
  for (const auto i : index_range(_species_names))
  {
    const auto & var_name = _species_names[i];
    for (const auto & boundary_name :
         getParam<std::vector<BoundaryName>>("compute_diffusive_fluxes_on"))
    {
      // Create the boundary integration of the flux
      const std::string pp_type =
          _use_ad ? "ADSideDiffusiveFluxIntegral" : "SideDiffusiveFluxIntegral";
      auto params = _factory.getValidParams(pp_type);
      params.set<std::vector<VariableName>>("variable") = {var_name};
      // FE variables require matprops for this postprocessor at the moment
      if (isParamValid("diffusivity_matprops"))
        params.set<MaterialPropertyName>("diffusivity") =
            getParam<std::vector<MaterialPropertyName>>("diffusivity_matprops")[i];
      // FV variables require functors
      else if (isParamValid("diffusivity_functors"))
        params.set<MooseFunctorName>("functor_diffusivity") =
            getParam<std::vector<MooseFunctorName>>("diffusivity_functors")[i];
      else
        params.set<MooseFunctorName>("functor_diffusivity") = "1";
      params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};
      // Default to maximum computation
      params.set<ExecFlagEnum>("execute_on") = {
          EXEC_INITIAL, EXEC_TIMESTEP_END, EXEC_NONLINEAR, EXEC_LINEAR};
      getProblem().addPostprocessor(
          pp_type, prefix() + "diffusive_flux_" + var_name + "_" + boundary_name, params);
    }
  }
}

void
MultiSpeciesDiffusionPhysicsBase::addComponent(const ActionComponent & component)
{
  for (const auto & block : component.blocks())
    _blocks.push_back(block);
}

void
MultiSpeciesDiffusionPhysicsBase::addInitialConditions()
{
  InputParameters params = getFactory().getValidParams("FunctionIC");
  assignBlocks(params, _blocks);

  // always obey the user specification of initial conditions
  // There are no default values, so no need to consider whether the app is restarting
  if (isParamValid("initial_conditions_species"))
    for (const auto i : index_range(_species_names))
    {
      const auto & var_name = _species_names[i];
      params.set<VariableName>("variable") = var_name;
      params.set<FunctionName>("function") =
          getParam<std::vector<FunctionName>>("initial_conditions_species")[i];

      getProblem().addInitialCondition("FunctionIC", prefix() + var_name + "_ic", params);
    }
}
