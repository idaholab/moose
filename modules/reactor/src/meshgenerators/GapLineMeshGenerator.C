//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapLineMeshGenerator.h"

#include "MooseMeshUtils.h"
#include "BoundaryLayerUtils.h"
#include "MooseUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/mesh_triangle_holes.h"

registerMooseObject("ReactorApp", GapLineMeshGenerator);

InputParameters
GapLineMeshGenerator::validParams()
{
  InputParameters params = PolygonMeshGeneratorBase::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The input mesh to create the gap based on.");

  params.addRequiredParam<Real>("thickness", "The thickness of the gap to be created.");

  MooseEnum gap_direction("OUTWARD INWARD", "OUTWARD");

  params.addParam<MooseEnum>("gap_direction",
                             gap_direction,
                             "In which direction the gap is created with respect to the side "
                             "normal of the elements along the boundary of the input mesh.");

  params.addParam<std::vector<boundary_id_type>>(
      "boundary_ids",
      std::vector<boundary_id_type>(),
      "The boundary IDs around which the gap will be created.");

  params.addParam<Real>("max_elem_size", "The maximum element size for the generated gap mesh.");

  params.addClassDescription(
      "Generates a polyline mesh that is based on an input 2D-XY mesh. The 2D-XY mesh needs to be "
      "a "
      "connected mesh with only one outer boundary manifold. The polyline mesh generated along "
      "with the boundary of the input mesh form an unmeshed gap with a specified thickness.");

  return params;
}

GapLineMeshGenerator::GapLineMeshGenerator(const InputParameters & parameters)
  : PolygonMeshGeneratorBase(parameters),
    _input(getMesh("input")),
    _thickness(getParam<Real>("thickness")),
    _gap_direction(getParam<MooseEnum>("gap_direction").template getEnum<GapDirection>()),
    _boundary_ids(getParam<std::vector<boundary_id_type>>("boundary_ids"))
{
}

std::unique_ptr<MeshBase>
GapLineMeshGenerator::generate()
{
  // Put the boundary mesh in a local pointer
  std::unique_ptr<UnstructuredMesh> mesh =
      dynamic_pointer_cast<UnstructuredMesh>(std::move(_input));

  std::set<std::size_t> mesh_bdry_ids(_boundary_ids.begin(), _boundary_ids.end());
  // MeshedHole is a good tool to extract and sort boundary points
  TriangulatorInterface::MeshedHole bdry_mh(*mesh, mesh_bdry_ids);

  // Reduce the point list to only contain vertices (linear-only: no midpoints)
  std::vector<Point> reduced_pts_list;
  std::vector<Point> reduced_mid_pts_list;
  BoundaryLayerUtils::collectExteriorVertexPointsFromMesh(
      bdry_mh, reduced_pts_list, reduced_mid_pts_list, /*skip_node_reduction=*/false);

  // Build the input polyline mesh to drive the normal computation in generateOffsetPolyline
  auto ply_mesh = buildMeshBaseObject();
  MooseMeshUtils::buildPolyLineMesh(*ply_mesh,
                                    reduced_pts_list,
                                    /*loop*/ true,
                                    BoundaryName(),
                                    BoundaryName(),
                                    std::vector<unsigned int>({1}));

  std::unique_ptr<UnstructuredMesh> ply_mesh_u =
      dynamic_pointer_cast<UnstructuredMesh>(std::move(ply_mesh));

  std::vector<Point> mod_reduced_pts_list =
      BoundaryLayerUtils::generateOffsetPolyline(this,
                                                 ply_mesh_u,
                                                 reduced_pts_list,
                                                 reduced_mid_pts_list,
                                                 _gap_direction == GapDirection::OUTWARD,
                                                 _thickness);

  auto ply_mesh_2 = buildMeshBaseObject();

  if (isParamValid("max_elem_size"))
    MooseMeshUtils::buildPolyLineMesh(*ply_mesh_2,
                                      mod_reduced_pts_list,
                                      true,
                                      BoundaryName(),
                                      BoundaryName(),
                                      getParam<Real>("max_elem_size"));
  else
    MooseMeshUtils::buildPolyLineMesh(*ply_mesh_2,
                                      mod_reduced_pts_list,
                                      true,
                                      BoundaryName(),
                                      BoundaryName(),
                                      std::vector<unsigned int>({1}));

  return ply_mesh_2;
}
