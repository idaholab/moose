//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVKernel.h"
#include "Assembly.h"
#include "SubProblem.h"

void
LinearFVKernel::setRMParams(const InputParameters & obj_params,
                            InputParameters & rm_params,
                            const unsigned short ghost_layers)
{
  rm_params.set<unsigned short>("layers") = ghost_layers;
  rm_params.set<bool>("use_point_neighbors") = obj_params.get<bool>("use_point_neighbors");
  rm_params.set<bool>("attach_geometric_early") = false;
  rm_params.set<bool>("use_displaced_mesh") = false;
}

InputParameters
LinearFVKernel::validParams()
{
  InputParameters params = LinearSystemContributionObject::validParams();
  params += BlockRestrictable::validParams();
  params += ADFunctorInterface::validParams();

  params.addParam<unsigned short>("ghost_layers", 1, "The number of layers of elements to ghost.");
  params.addParam<bool>("use_point_neighbors",
                        false,
                        "Whether to use point neighbors, which introduces additional ghosting to "
                        "that used for simple face neighbors.");
  params.addParamNamesToGroup("ghost_layers use_point_neighbors", "Parallel ghosting");

  // FV Kernels always need one layer of ghosting because when looping over
  // faces to compute fluxes, the elements on each side of the face may be on
  // different MPI ranks, but we still need to access them as a pair to
  // compute the numerical face flux.
  params.addRelationshipManager(
      "ElementSideNeighborLayers",
      Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
          Moose::RelationshipManagerType::COUPLING,
      [](const InputParameters & obj_params, InputParameters & rm_params)
      {
        LinearFVKernel::setRMParams(
            obj_params, rm_params, obj_params.get<unsigned short>("ghost_layers"));
      });

  params.registerBase("LinearFVKernel");
  return params;
}

LinearFVKernel::LinearFVKernel(const InputParameters & params)
  : LinearSystemContributionObject(params),
    BlockRestrictable(this),
    ADFunctorInterface(this),
    MooseVariableInterface(this,
                           false,
                           "variable",
                           Moose::VarKindType::VAR_LINEAR,
                           Moose::VarFieldType::VAR_FIELD_STANDARD),
    MooseVariableDependencyInterface(this),
    _var(mooseLinearVariableFV())
{
  addMooseVariableDependency(_var);
}
