//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Boundary2DDelaunayGenerator.h"

#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/boundary_info.h"
#include "libmesh/poly2tri_triangulator.h"
#include "libmesh/mesh_triangle_holes.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/parallel_algebra.h"

registerMooseObject("MooseApp", Boundary2DDelaunayGenerator);

InputParameters
Boundary2DDelaunayGenerator::validParams()
{
  InputParameters params = SurfaceDelaunayGeneratorBase::validParams();
  params += LevelSetMeshingHelper::validParams();

  params.addClassDescription(
      "Mesh generator that convert a 2D surface given as one or a few boundaries of a 3D mesh into "
      "a 2D mesh using Delaunay triangulation.");
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::vector<BoundaryName>>("boundary_names", "The boundaries to be used");
  params.addParam<std::vector<std::vector<BoundaryName>>>(
      "hole_boundary_names",
      std::vector<std::vector<BoundaryName>>(),
      "The optional boundaries to be used as the holes in the mesh during triangulation. Note that "
      "this is a vector of vectors, which allows each hole to be defined as a combination of "
      "multiple boundaries.");

  params.addParam<BoundaryName>(
      "output_external_boundary_name",
      "",
      "The optional name of the external boundary of the mesh to generate. If not provided, the "
      "external boundary will be have a trivial id of 0.");

  return params;
}

Boundary2DDelaunayGenerator::Boundary2DDelaunayGenerator(const InputParameters & parameters)
  : SurfaceDelaunayGeneratorBase(parameters),
    LevelSetMeshingHelper(parameters),
    _input(getMesh("input")),
    _boundary_names(getParam<std::vector<BoundaryName>>("boundary_names")),
    _hole_boundary_names(getParam<std::vector<std::vector<BoundaryName>>>("hole_boundary_names")),
    _output_external_boundary_name(getParam<BoundaryName>("output_external_boundary_name"))
{
}

std::unique_ptr<MeshBase>
Boundary2DDelaunayGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh_3d = std::move(_input);

  // Generate a new 2D block based on the sidesets
  const auto new_block_id = MooseMeshUtils::getNextFreeSubdomainID(*mesh_3d);
  try
  {
    MooseMeshUtils::createSubdomainFromSidesets(
        *mesh_3d, _boundary_names, new_block_id, SubdomainName(), type());
  }
  catch (MooseException & e)
  {
    if (((std::string)e.what()).compare(0, 12, "The sideset ") == 0)
      paramError("boundary_names", e.what());
    else
      mooseError(e.what());
  }

  // If holes are provided, we need to create new blocks for them too
  std::vector<subdomain_id_type> hole_block_ids;
  for (const auto & hole : _hole_boundary_names)
  {
    hole_block_ids.push_back(MooseMeshUtils::getNextFreeSubdomainID(*mesh_3d));
    try
    {
      MooseMeshUtils::createSubdomainFromSidesets(
          *mesh_3d, hole, hole_block_ids.back(), SubdomainName(), type());
    }
    catch (MooseException & e)
    {
      if (((std::string)e.what()).compare(0, 12, "The sideset ") == 0)
        paramError("hole_boundary_names", e.what());
      else
        mooseError(e.what());
    }
  }

  // Create a 2D mesh form the 2D block
  auto mesh_2d = buildMeshBaseObject();
  MooseMeshUtils::convertBlockToMesh(*mesh_3d, *mesh_2d, {std::to_string(new_block_id)});
  // If holes are provided, we need to create a 2D mesh for each hole
  std::vector<std::unique_ptr<MeshBase>> hole_meshes_2d;
  for (const auto & hole_block_id : hole_block_ids)
  {
    hole_meshes_2d.push_back(buildMeshBaseObject());
    MooseMeshUtils::convertBlockToMesh(
        *mesh_3d, *hole_meshes_2d.back(), {std::to_string(hole_block_id)});
  }

  // We do not need the 3D mesh anymore
  mesh_3d->clear();

  return General2DDelaunay(mesh_2d, hole_meshes_2d);
}

std::unique_ptr<MeshBase>
Boundary2DDelaunayGenerator::General2DDelaunay(
    std::unique_ptr<MeshBase> & mesh_2d, std::vector<std::unique_ptr<MeshBase>> & hole_meshes_2d)
{
  // If a level set is provided, we need to check if the nodes in the original 2D mesh match the
  // level set
  if (_func_level_set)
  {
    for (const auto & node : mesh_2d->node_ptr_range())
    {
      if (std::abs(levelSetEvaluator(*node)) > libMesh::TOLERANCE)
      {
        paramError("level_set",
                   "The level set function does not match the nodes in the given boundary of the "
                   "input mesh.");
      }
    }
  }

  // Easier to work with a TRI3 mesh
  // all_tri() also prepares the mesh for use
  mesh_2d->prepare_for_use();
  MeshTools::Modification::all_tri(*mesh_2d);
  const auto mesh_2d_ext_bdry = MooseMeshUtils::getNextFreeBoundaryID(*mesh_2d);
  for (const auto & elem : mesh_2d->active_element_ptr_range())
    for (const auto & i_side : elem->side_index_range())
      if (elem->neighbor_ptr(i_side) == nullptr)
        mesh_2d->get_boundary_info().add_side(elem, i_side, mesh_2d_ext_bdry);

  // Create a clone of the 2D mesh to be used for the 1D mesh generation
  auto mesh_2d_dummy = dynamic_pointer_cast<MeshBase>(mesh_2d->clone());
  // Generate a new 1D block based on the external boundary
  const auto new_block_id_1d = MooseMeshUtils::getNextFreeSubdomainID(*mesh_2d_dummy);

  MooseMeshUtils::createSubdomainFromSidesets(
      *mesh_2d_dummy, {std::to_string(mesh_2d_ext_bdry)}, new_block_id_1d, SubdomainName(), type());

  // Create a 1D mesh form the 1D block
  auto mesh_1d = buildMeshBaseObject();
  MooseMeshUtils::convertBlockToMesh(*mesh_2d_dummy, *mesh_1d, {std::to_string(new_block_id_1d)});
  mesh_2d_dummy->clear();

  // If we have holes, we need to create a 1D mesh for each hole
  std::vector<std::unique_ptr<MeshBase>> hole_meshes_1d;
  for (auto & hole_mesh_2d : hole_meshes_2d)
  {
    // As we do not need these holes for reverse projection, we do not need to convert them to TRI3
    // meshes, but we still need to create a 1D mesh for each hole
    hole_mesh_2d->find_neighbors();
    const auto hole_mesh_2d_ext_bdry = MooseMeshUtils::getNextFreeBoundaryID(*hole_mesh_2d);
    for (const auto & elem : hole_mesh_2d->active_element_ptr_range())
      for (const auto & i_side : elem->side_index_range())
        if (elem->neighbor_ptr(i_side) == nullptr)
          hole_mesh_2d->get_boundary_info().add_side(elem, i_side, hole_mesh_2d_ext_bdry);
    const auto new_hole_block_id_1d = MooseMeshUtils::getNextFreeSubdomainID(*hole_mesh_2d);
    MooseMeshUtils::createSubdomainFromSidesets(*hole_mesh_2d,
                                                {std::to_string(hole_mesh_2d_ext_bdry)},
                                                new_hole_block_id_1d,
                                                SubdomainName(),
                                                type());
    // Create a 1D mesh form the 1D block
    hole_meshes_1d.push_back(buildMeshBaseObject());
    MooseMeshUtils::convertBlockToMesh(
        *hole_mesh_2d, *hole_meshes_1d.back(), {std::to_string(new_hole_block_id_1d)});
    hole_mesh_2d->clear();
  }

  // Find centroid of the 2D mesh
  const Point centroid = MooseMeshUtils::meshCentroidCalculator(*mesh_2d);
  // calculate an average normal vector of the 2D mesh
  const Point mesh_norm = meshNormal2D(*mesh_2d);
  // Check the deviation of the mesh normal vector from the global average normal vector
  if (meshNormalDeviation2D(*mesh_2d, mesh_norm) > _max_angle_deviation)
    paramError("boundary_names",
               "The normal vector of some elements in the 2D mesh deviates too much from the "
               "global average normal vector. The maximum deviation is " +
                   std::to_string(_max_angle_deviation) +
                   ". Consider dividing the boundary into several parts to "
                   "reduce the angle deviation.");

  // Move both 2d and 1d meshes to the centroid of the 2D mesh
  MeshTools::Modification::translate(*mesh_1d, -centroid(0), -centroid(1), -centroid(2));
  MeshTools::Modification::translate(*mesh_2d, -centroid(0), -centroid(1), -centroid(2));
  // Alsop need to translate the 1D hole meshes if applicable
  for (auto & hole_mesh_1d : hole_meshes_1d)
    MeshTools::Modification::translate(*hole_mesh_1d, -centroid(0), -centroid(1), -centroid(2));

  // Calculate the Euler angles to rotate the meshes so that the 2D mesh is close to the XY plane
  // (i.e., the normal vector of the 2D mesh is aligned with the Z axis)
  const Real theta = std::acos(mesh_norm(2)) / M_PI * 180.0;
  const Real phi =
      (MooseUtils::absoluteFuzzyLessThan(mesh_norm(2), 1.0) ? std::atan2(mesh_norm(1), mesh_norm(0))
                                                            : 0.0) /
      M_PI * 180.0;
  MeshTools::Modification::rotate(*mesh_1d, 90.0 - phi, theta, 0.0);
  MeshTools::Modification::rotate(*mesh_2d, 90.0 - phi, theta, 0.0);
  // Also rotate the 1D hole meshes if applicable
  for (auto & hole_mesh_1d : hole_meshes_1d)
    MeshTools::Modification::rotate(*hole_mesh_1d, 90.0 - phi, theta, 0.0);

  // Clone the 2D mesh to be used for reverse projection later
  auto mesh_2d_xyz = dynamic_pointer_cast<MeshBase>(mesh_2d->clone());

  // Project the 2D mesh to the XY plane so that XYDelaunay can be used
  for (const auto & node : mesh_2d->node_ptr_range())
    (*node)(2) = 0;
  // Project the 1D mesh to the XY plane as well
  for (const auto & node : mesh_1d->node_ptr_range())
    (*node)(2) = 0;
  // Also project the 1D hole meshes to the XY plane if applicable
  for (auto & hole_mesh_1d : hole_meshes_1d)
  {
    for (const auto & node : hole_mesh_1d->node_ptr_range())
      (*node)(2) = 0;
  }

  std::vector<libMesh::TriangulatorInterface::MeshedHole> meshed_holes;
  std::vector<libMesh::TriangulatorInterface::Hole *> triangulator_hole_ptrs(hole_meshes_1d.size());
  meshed_holes.reserve(hole_meshes_1d.size());
  for (auto hole_i : index_range(hole_meshes_1d))
  {
    hole_meshes_1d[hole_i]->prepare_for_use();
    meshed_holes.emplace_back(*hole_meshes_1d[hole_i]);
    triangulator_hole_ptrs[hole_i] = &meshed_holes.back();
  }

  // Finally, triangulation
  std::unique_ptr<UnstructuredMesh> mesh =
      dynamic_pointer_cast<UnstructuredMesh>(std::move(mesh_1d));

  Poly2TriTriangulator poly2tri(*mesh);
  poly2tri.triangulation_type() = TriangulatorInterface::PSLG;

  poly2tri.set_interpolate_boundary_points(0);
  poly2tri.set_refine_boundary_allowed(false);
  poly2tri.set_verify_hole_boundaries(false);
  poly2tri.desired_area() = 0;
  poly2tri.minimum_angle() = 0; // Not yet supported
  poly2tri.smooth_after_generating() = false;
  if (!triangulator_hole_ptrs.empty())
    poly2tri.attach_hole_list(&triangulator_hole_ptrs);
  // Future TODO: correct the area function based on the local normal vector
  if (_use_auto_area_func)
    poly2tri.set_auto_area_function(this->comm(),
                                    _auto_area_function_num_points,
                                    _auto_area_function_power,
                                    _auto_area_func_default_size,
                                    _auto_area_func_default_size_dist);
  poly2tri.triangulate();

  // Reverse the projection based on the original 2D mesh
  for (const auto & node : mesh->node_ptr_range())
  {
    bool node_mod = false;
    // Try to find the element in mesh_2d that contains the new node
    for (const auto & elem : mesh_2d->active_element_ptr_range())
    {
      if (elem->contains_point(Point((*node)(0), (*node)(1), 0.0)))
      {
        // Element id
        const auto elem_id = elem->id();
        // element in xyz_in_xyz
        const Elem & elem_xyz = *mesh_2d_xyz->elem_ptr(elem_id);

        const Point elem_normal = elemNormal(elem_xyz);
        const Point & elem_p = *mesh_2d_xyz->elem_ptr(elem_id)->node_ptr(0);

        // if the x and y values of the node is the same as the elem_p's first node, we can just
        // move it to that node's position
        if (MooseUtils::absoluteFuzzyEqual((*node)(0), elem_p(0)) &&
            MooseUtils::absoluteFuzzyEqual((*node)(1), elem_p(1)))
        {
          (*node)(2) = elem_p(2);
          node_mod = true;
          break;
        }
        // Otherwise, we need to find a position inside the 2D element
        // It has the same x and y coordinates as the node in the projected mesh;
        (*node)(2) = elem_p(2) - (((*node)(0) - elem_p(0)) * elem_normal(0) +
                                  ((*node)(1) - elem_p(1)) * elem_normal(1)) /
                                     elem_normal(2);
        node_mod = true;
        break;
      }
    }
    if (!node_mod)
      mooseError("Node not found in mesh_in_xy");
  }

  // Rotate the mesh back
  MeshTools::Modification::rotate(*mesh, 0.0, -theta, phi - 90.0);
  // Translate the mesh back
  MeshTools::Modification::translate(*mesh, centroid(0), centroid(1), centroid(2));

  // Correct the nodes based on the level set function
  if (_func_level_set)
  {
    for (const auto & node : mesh->node_ptr_range())
    {
      unsigned int iter_ct = 0;
      while (iter_ct < _max_level_set_correction_iterations &&
             std::abs(levelSetEvaluator(*node)) > libMesh::TOLERANCE * libMesh::TOLERANCE)
      {
        levelSetCorrection(*node);
        ++iter_ct;
      }
    }
  }

  // Assign the external boundary name if provided
  if (!_output_external_boundary_name.empty())
  {
    const std::vector<BoundaryID> output_boundary_id =
        MooseMeshUtils::getBoundaryIDs(*mesh, {_output_external_boundary_name}, true);

    libMesh::MeshTools::Modification::change_boundary_id(*mesh, 0, output_boundary_id[0]);
    mesh->get_boundary_info().sideset_name(output_boundary_id[0]) = _output_external_boundary_name;
  }

  mesh->unset_has_boundary_id_sets();

  return mesh;
}
