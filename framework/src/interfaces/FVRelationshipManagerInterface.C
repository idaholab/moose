//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVRelationshipManagerInterface.h"

InputParameters
FVRelationshipManagerInterface::validParams()
{
  // Create InputParameters object that will be appended to the parameters for the inheriting object
  InputParameters params = emptyInputParameters();

  params.addParam<unsigned short>("ghost_layers", 1, "The number of layers of elements to ghost.");
  params.addParam<bool>("use_point_neighbors",
                        false,
                        "Whether to use point neighbors, which introduces additional ghosting to "
                        "that used for simple face neighbors.");
  params.addParamNamesToGroup("ghost_layers use_point_neighbors", "Parallel ghosting");

  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation. Note that in "
                        "the case this is true but no displacements "
                        "are provided in the Mesh block the "
                        "undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  // FV Kernels always need at least one layer of ghosting because when looping over
  // faces to compute fluxes, the elements on each side of the face may be on
  // different MPI ranks, but we still need to access them as a pair to
  // compute the numerical face flux.
  params.addRelationshipManager(
      "ElementSideNeighborLayers",
      Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
          Moose::RelationshipManagerType::COUPLING,
      [](const InputParameters & obj_params, InputParameters & rm_params)
      {
        FVRelationshipManagerInterface::setRMParams(
            obj_params, rm_params, obj_params.get<unsigned short>("ghost_layers"));
      });

  return params;
}

void
FVRelationshipManagerInterface::setRMParams(const InputParameters & obj_params,
                                            InputParameters & rm_params,
                                            const unsigned short ghost_layers)
{
  rm_params.set<unsigned short>("layers") = ghost_layers;
  rm_params.set<bool>("use_point_neighbors") = obj_params.get<bool>("use_point_neighbors");
  rm_params.set<bool>("attach_geometric_early") = false;
  rm_params.set<bool>("use_displaced_mesh") = obj_params.get<bool>("use_displaced_mesh");
}
