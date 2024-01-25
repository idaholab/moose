//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StitchBoundaryMeshGenerator.h"
#include "CastUniquePointer.h"
#include "MooseUtils.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("MooseApp", StitchBoundaryMeshGenerator);

InputParameters
StitchBoundaryMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  MooseEnum algorithm("BINARY EXHAUSTIVE", "BINARY");

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addParam<bool>(
      "clear_stitched_boundary_ids", true, "Whether or not to clear the stitched boundary IDs");
  params.addParam<MooseEnum>(
      "algorithm",
      algorithm,
      "Control the use of binary search for the nodes of the stitched surfaces.");
  params.addRequiredParam<std::vector<boundary_id_type>>(
      "stitch_boundaries_pair", "Pair of boundaries to be stitched together.");
  params.addClassDescription("Allows a pair of boundaries to be stitched together.");

  return params;
}

StitchBoundaryMeshGenerator::StitchBoundaryMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _clear_stitched_boundary_ids(getParam<bool>("clear_stitched_boundary_ids")),
    _stitch_boundaries_pair(getParam<std::vector<boundary_id_type>>("stitch_boundaries_pair")),
    _algorithm(parameters.get<MooseEnum>("algorithm"))
{
}

std::unique_ptr<MeshBase>
StitchBoundaryMeshGenerator::generate()
{
  std::unique_ptr<UnstructuredMesh> mesh =
      dynamic_pointer_cast<UnstructuredMesh>(std::move(_input));

  const bool use_binary_search = (_algorithm == "BINARY");

  // stitch_surfaces only recognizes node_set boundaries
  mesh->get_boundary_info().build_node_list_from_side_list();

  mesh->stitch_surfaces(_stitch_boundaries_pair[0],
                        _stitch_boundaries_pair[1],
                        TOLERANCE,
                        _clear_stitched_boundary_ids,
                        /*verbose = */ true,
                        use_binary_search);

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
