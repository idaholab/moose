//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SphereSurfaceMeshGenerator.h"
#include "CastUniquePointer.h"

// libMesh includes
#include "libmesh/face_tri3.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/mesh_refinement.h"
#include "libmesh/sphere.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/replicated_mesh.h"

registerMooseObject("PhaseFieldApp", SphereSurfaceMeshGenerator);

InputParameters
SphereSurfaceMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription(
      "Generated sphere mesh - a two dimensional manifold embedded in three dimensional space");
  params.addParam<Real>("radius", 1.0, "Sphere radius");
  params.addParam<Point>("center", Point(0, 0, 0), "Center of the sphere");
  params.addParam<unsigned int>(
      "depth", 3, "Iteration steps in the triangle bisection construction");
  return params;
}

SphereSurfaceMeshGenerator::SphereSurfaceMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _radius(getParam<Real>("radius")),
    _center(getParam<Point>("center")),
    _depth(getParam<unsigned int>("depth"))
{
}

std::unique_ptr<MeshBase>
SphereSurfaceMeshGenerator::generate()
{
  // Have MOOSE construct the correct libMesh::Mesh object using Mesh block and CLI parameters.
  auto mesh = buildMeshBaseObject();
  mesh->set_mesh_dimension(2);
  mesh->set_spatial_dimension(3);

  const Sphere sphere(_center, _radius);

  // icosahedron points (using golden ratio rectangle construction)
  const Real phi = (1.0 + std::sqrt(5.0)) / 2.0;
  const Real X = std::sqrt(1.0 / (phi * phi + 1.0));
  const Real Z = X * phi;
  const Point vdata[12] = {{-X, 0.0, Z},
                           {X, 0.0, Z},
                           {-X, 0.0, -Z},
                           {X, 0.0, -Z},
                           {0.0, Z, X},
                           {0.0, Z, -X},
                           {0.0, -Z, X},
                           {0.0, -Z, -X},
                           {Z, X, 0.0},
                           {-Z, X, 0.0},
                           {Z, -X, 0.0},
                           {-Z, -X, 0.0}};
  for (unsigned int i = 0; i < 12; ++i)
    mesh->add_point(vdata[i] * _radius + _center, i);

  // icosahedron faces
  const unsigned int tindices[20][3] = {{0, 4, 1},  {0, 9, 4},  {9, 5, 4},  {4, 5, 8},  {4, 8, 1},
                                        {8, 10, 1}, {8, 3, 10}, {5, 3, 8},  {5, 2, 3},  {2, 7, 3},
                                        {7, 10, 3}, {7, 6, 10}, {7, 11, 6}, {11, 0, 6}, {0, 1, 6},
                                        {6, 1, 10}, {9, 0, 11}, {9, 11, 2}, {9, 2, 5},  {7, 2, 11}};
  for (unsigned int i = 0; i < 20; ++i)
  {
    Elem * elem = mesh->add_elem(new Tri3);
    elem->set_node(0) = mesh->node_ptr(tindices[i][0]);
    elem->set_node(1) = mesh->node_ptr(tindices[i][1]);
    elem->set_node(2) = mesh->node_ptr(tindices[i][2]);
  }

  // we need to prepare distributed meshes before using refinement
  if (!mesh->is_replicated())
    mesh->prepare_for_use();

  // Now we have the beginnings of a sphere.
  // Add some more elements by doing uniform refinements and
  // popping nodes to the boundary.
  MeshRefinement mesh_refinement(*mesh);

  // Loop over the elements, refine, pop nodes to boundary.
  for (unsigned int r = 0; r < _depth; ++r)
  {
    mesh_refinement.uniformly_refine(1);

    auto it = mesh->active_nodes_begin();
    const auto end = mesh->active_nodes_end();

    for (; it != end; ++it)
    {
      Node & node = **it;
      node = sphere.closest_point(node);
    }
  }

  // Flatten the AMR mesh to get rid of inactive elements
  MeshTools::Modification::flatten(*mesh);

  return dynamic_pointer_cast<MeshBase>(mesh);
}
