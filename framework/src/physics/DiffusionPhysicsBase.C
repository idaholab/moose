//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionPhysicsBase.h"
#include "MatDiffusion.h"
#include "MoosePreconditioner.h"
#include "PetscSupport.h"
#include "MooseEnumItem.h"

InputParameters
DiffusionPhysicsBase::validParams()
{
  InputParameters params = PhysicsBase::validParams();
  params.addClassDescription("Base class for creating a diffusion equation");

  params.addParam<VariableName>("variable_name", "u", "Variable name for the equation");

  // Diffusivity
  params.addParam<MaterialPropertyName>("diffusivity_matprop",
                                        "Material property defining the diffusion coefficient");
  params.addParam<MooseFunctorName>("diffusivity_functor", "Functor specifying the diffusivity");

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

  // Preconditioning is implemented so let's use it by default
  MooseEnum pc_options("default none", "default");
  params.addParam<MooseEnum>(
      "preconditioning", pc_options, "Which preconditioning to use for this Physics");

  return params;
}

DiffusionPhysicsBase::DiffusionPhysicsBase(const InputParameters & parameters)
  : PhysicsBase(parameters),
    _var_name(getParam<VariableName>("variable_name")),
    _neumann_boundaries(getParam<std::vector<BoundaryName>>("neumann_boundaries")),
    _dirichlet_boundaries(getParam<std::vector<BoundaryName>>("dirichlet_boundaries"))
{
  // Parameter checking
  checkVectorParamsSameLength<BoundaryName, MooseFunctorName>("neumann_boundaries",
                                                              "boundary_fluxes");
  checkVectorParamsSameLength<BoundaryName, MooseFunctorName>("dirichlet_boundaries",
                                                              "boundary_values");
  checkVectorParamsNoOverlap<BoundaryName>({"neumann_boundaries", "dirichlet_boundaries"});
  checkParamsBothSetOrNotSet("source_functor", "source_coef");

  addRequiredPhysicsTask("add_preconditioning");
}

void
DiffusionPhysicsBase::addPreconditioning()
{
  // Use a multigrid method, known to work for elliptic problems such as diffusion
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
