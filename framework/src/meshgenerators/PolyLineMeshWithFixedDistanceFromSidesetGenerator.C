//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolyLineMeshWithFixedDistanceFromSidesetGenerator.h"

#include "MooseMeshUtils.h"
#include "MooseUtils.h"
#include "GeometryUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/mesh_serializer.h"
#include "libmesh/mesh_triangle_holes.h"
#include "libmesh/poly2tri_triangulator.h"

registerMooseObject("MooseApp", PolyLineMeshWithFixedDistanceFromSidesetGenerator);
registerMooseObjectRenamed("MooseApp",
                           GapLineMeshGenerator,
                           "06/30/2027 24:00",
                           PolyLineMeshWithFixedDistanceFromSidesetGenerator);

InputParameters
PolyLineMeshWithFixedDistanceFromSidesetGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The input mesh to create the gap based on.");

  params.addRequiredParam<Real>("thickness", "The thickness of the gap to be created.");

  MooseEnum gap_direction("OUTWARD INWARD", "OUTWARD");

  params.addParam<MooseEnum>("gap_direction",
                             gap_direction,
                             "In which direction the gap is created with respect to the side "
                             "normal of the elements along the boundary of the input mesh.");

  params.addParam<std::vector<BoundaryName>>(
      "boundary_names",
      std::vector<BoundaryName>(),
      "The boundary names around which the gap will be created.");

  params.addParam<Real>("max_elem_size", "The maximum element size for the generated gap mesh.");

  params.addParam<bool>("skip_node_reduction",
                        false,
                        "Whether to skip the node reduction step after generating the gap mesh.");

  params.addClassDescription(
      "Generates a polyline mesh that is based on an input 2D-XY mesh. The 2D-XY mesh needs to be "
      "a "
      "connected mesh with only one outer boundary manifold. The polyline mesh generated along "
      "with the boundary of the input mesh form an unmeshed gap with a specified thickness.");

  return params;
}

PolyLineMeshWithFixedDistanceFromSidesetGenerator::
    PolyLineMeshWithFixedDistanceFromSidesetGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _thickness(getParam<Real>("thickness")),
    _gap_direction(getParam<MooseEnum>("gap_direction").template getEnum<GapDirection>()),
    _boundary_names(getParam<std::vector<BoundaryName>>("boundary_names")),
    _skip_node_reduction(getParam<bool>("skip_node_reduction"))
{
}

std::unique_ptr<MeshBase>
PolyLineMeshWithFixedDistanceFromSidesetGenerator::generate()
{
  // Put the boundary mesh in a local pointer
  std::unique_ptr<UnstructuredMesh> mesh =
      dynamic_pointer_cast<UnstructuredMesh>(std::move(_input));
  // Check if boundary names exist in the mesh and convert them to boundary IDs
  auto boundary_ids = MooseMeshUtils::getBoundaryIDs(*mesh, _boundary_names, false);
  for (const auto & i : index_range(boundary_ids))
  {
    if (boundary_ids[i] == BoundaryInfo::invalid_id ||
        !MooseMeshUtils::hasBoundaryID(*mesh, boundary_ids[i]))
      paramError("boundary_names",
                 "the provided boundary name " + _boundary_names[i] +
                     " was not found in the input mesh");
  }

  std::set<std::size_t> mesh_bdry_ids(boundary_ids.begin(), boundary_ids.end());
  // MeshedHole is a good tool to extract and sort boundary points
  TriangulatorInterface::MeshedHole bdry_mh(*mesh, mesh_bdry_ids);

  if (bdry_mh.n_midpoints() > 1)
    paramError("input", "Only linear and quadratic elements are supported for gap generation.");
  else if (bdry_mh.n_midpoints() == 1)
  {
    if (!_skip_node_reduction)
      paramWarning(
          "skip_node_reduction",
          "The input mesh contains quadratic elements. If you want to keep the midpoints in "
          "the generated gap mesh, please set skip_node_reduction to true. Otherwise a linear gap "
          "mesh will be generated.");
    if (isParamValid("max_elem_size"))
      paramError(
          "max_elem_size",
          "This parameter should not be provided as the input mesh contains quadratic elements.");
  }

  // Reduce the point list to only contain vertices
  std::vector<Point> reduced_pts_list;
  std::vector<Point> reduced_mid_pts_list;
  for (const auto i : make_range(bdry_mh.n_points()))
  {
    if (_skip_node_reduction ||
        !geom_utils::arePointsColinear(
            bdry_mh.point((i - 1 + bdry_mh.n_points()) % bdry_mh.n_points()),
            bdry_mh.point(i),
            bdry_mh.point((i + 1) % bdry_mh.n_points())))
    {
      reduced_pts_list.push_back(bdry_mh.point(i));
      if (bdry_mh.n_midpoints() == 1 && _skip_node_reduction)
        reduced_mid_pts_list.push_back(bdry_mh.midpoint(0, i));
    }
  }
  // Here we need a method to generate the outward normals of each external side
  auto ply_mesh = buildMeshBaseObject();

  MooseMeshUtils::buildPolyLineMesh(*ply_mesh,
                                    reduced_pts_list,
                                    reduced_mid_pts_list,
                                    /*loop*/ true,
                                    BoundaryName(),
                                    BoundaryName(),
                                    std::vector<unsigned int>({1}));

  std::unique_ptr<UnstructuredMesh> ply_mesh_u =
      dynamic_pointer_cast<UnstructuredMesh>(std::move(ply_mesh));

  auto tmp_mod_reduced_pts_list =
      MooseMeshUtils::generateLayerPoints(this,
                                          ply_mesh_u,
                                          reduced_pts_list,
                                          reduced_mid_pts_list,
                                          _gap_direction == GapDirection::OUTWARD,
                                          _thickness);

  std::vector<Point> mod_reduced_pts_list;
  std::vector<Point> mod_reduced_mid_pts_list;
  if (reduced_mid_pts_list.size())
  {
    mod_reduced_pts_list.insert(mod_reduced_pts_list.end(),
                                tmp_mod_reduced_pts_list.begin(),
                                tmp_mod_reduced_pts_list.begin() + reduced_pts_list.size());
    mod_reduced_mid_pts_list.insert(mod_reduced_mid_pts_list.end(),
                                    tmp_mod_reduced_pts_list.begin() + reduced_pts_list.size(),
                                    tmp_mod_reduced_pts_list.end());
  }
  else
  {
    mod_reduced_pts_list = std::move(tmp_mod_reduced_pts_list);
  }

  auto ply_mesh_2 = buildMeshBaseObject();

  if (isParamValid("max_elem_size"))
    MooseMeshUtils::buildPolyLineMesh(*ply_mesh_2,
                                      mod_reduced_pts_list,
                                      mod_reduced_mid_pts_list,
                                      true,
                                      BoundaryName(),
                                      BoundaryName(),
                                      getParam<Real>("max_elem_size"));
  else
    MooseMeshUtils::buildPolyLineMesh(*ply_mesh_2,
                                      mod_reduced_pts_list,
                                      mod_reduced_mid_pts_list,
                                      true,
                                      BoundaryName(),
                                      BoundaryName(),
                                      std::vector<unsigned int>({1}));

  return ply_mesh_2;
}
