//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  InputParameters params = StitchMeshGeneratorBase::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.renameParam("stitch_boundaries_pairs",
                     "stitch_boundaries_pair",
                     "Pair of boundaries to be stitched together.");
  params.addClassDescription("Allows a pair of boundaries to be stitched together.");

  return params;
}

StitchBoundaryMeshGenerator::StitchBoundaryMeshGenerator(const InputParameters & parameters)
  : StitchMeshGeneratorBase(parameters), _input(getMesh("input"))
{
  if (_stitch_boundaries_pairs.size() != 1 && _stitch_boundaries_pairs[0].size() != 2)
    paramError("stitch_boundaries_pair", "Can only stitch two boundaries together.");
}

std::unique_ptr<MeshBase>
StitchBoundaryMeshGenerator::generate()
{
  std::unique_ptr<UnstructuredMesh> mesh =
      dynamic_pointer_cast<UnstructuredMesh>(std::move(_input));

  const bool use_binary_search = (_algorithm == "BINARY");

  // stitch_surfaces only recognizes node_set boundaries
  mesh->get_boundary_info().build_node_list_from_side_list();

  // Check that boundaries exist
  const auto first_bid = getBoundaryIdToStitch(*mesh, name(), _stitch_boundaries_pairs[0][0]);
  const auto second_bid = getBoundaryIdToStitch(*mesh, name(), _stitch_boundaries_pairs[0][1]);

  // Stitch the boundaries
  mesh->stitch_surfaces(first_bid,
                        second_bid,
                        TOLERANCE,
                        _clear_stitched_boundary_ids,
                        getParam<bool>("verbose_stitching"),
                        use_binary_search,
                        getParam<Real>("stitching_hmin_tolerance_factor"));

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
