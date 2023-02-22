//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "OverlayMeshGenerator.h"
#include "CastUniquePointer.h"
// libMesh includes
#include "libmesh/mesh_generation.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/mesh_tools.h"

registerMooseObject("MooseApp", OverlayMeshGenerator);
InputParameters
OverlayMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription("This OverlayMeshGenerator creates a Cartesian mesh using "
                             "DistributedRectilinearMeshGenerator in "
                             "the mesh block region.");
  params.addRequiredParam<MeshGeneratorName>("input", "The base mesh we want to overlay");
  MooseEnum dims("1=1 2 3");
  params.addRequiredParam<MooseEnum>(
      "dim", dims, "The dimension of the mesh to be generated"); // Make this parameter required
  params.addParam<dof_id_type>("nx", 1, "Number of elements in the X direction");
  params.addParam<dof_id_type>("ny", 1, "Number of elements in the Y direction");
  params.addParam<dof_id_type>("nz", 1, "Number of elements in the Z direction");
  params.addParam<processor_id_type>(
      "num_cores_for_partition",
      0,
      "Number of cores for partitioning the graph (dafaults to the number of MPI ranks)");
  params.addRangeCheckedParam<unsigned>(
      "num_side_layers",
      2,
      "num_side_layers>=1 & num_side_layers<5",
      "Number of layers of off-processor side neighbors is reserved during mesh generation");
  MooseEnum partition("graph linear square", "graph", false);
  params.addParam<MooseEnum>(
      "partition", partition, "Which method (graph linear square) use to partition mesh");
  MooseEnum elem_types(
      "EDGE EDGE2 EDGE3 EDGE4 QUAD QUAD4 QUAD8 QUAD9 TRI3 TRI6 HEX HEX8 HEX20 HEX27 TET4 TET10 "
      "PRISM6 PRISM15 PRISM18 PYRAMID5 PYRAMID13 PYRAMID14"); // no default
  params.addParam<MooseEnum>("elem_type",
                             elem_types,
                             "The type of element from libMesh to "
                             "generate (default: linear element for "
                             "requested dimension)");

  params.addRangeCheckedParam<Real>(
      "bias_x",
      1.,
      "bias_x>=0.5 & bias_x<=2",
      "The amount by which to grow (or shrink) the cells in the x-direction.");
  params.addRangeCheckedParam<Real>(
      "bias_y",
      1.,
      "bias_y>=0.5 & bias_y<=2",
      "The amount by which to grow (or shrink) the cells in the y-direction.");
  params.addRangeCheckedParam<Real>(
      "bias_z",
      1.,
      "bias_z>=0.5 & bias_z<=2",
      "The amount by which to grow (or shrink) the cells in the z-direction.");
  return params;
}

OverlayMeshGenerator::OverlayMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), _dim(getParam<MooseEnum>("dim")), _mesh_input(getMesh("input"))
{
  auto params = _app.getFactory().getValidParams("DistributedRectilinearMeshGenerator");
  params.set<MooseEnum>("dim") = _dim;

  params.set<dof_id_type>("nx") = getParam<dof_id_type>("nx");
  params.set<dof_id_type>("ny") = getParam<dof_id_type>("ny");
  params.set<dof_id_type>("nz") = getParam<dof_id_type>("nz");

  params.set<Real>("xmin") = 0;
  params.set<Real>("ymin") = 0;
  params.set<Real>("zmin") = 0;
  params.set<Real>("xmax") = 1;
  params.set<Real>("ymax") = 1;
  params.set<Real>("zmax") = 1;

  params.set<Real>("bias_x") = getParam<Real>("bias_x");
  params.set<Real>("bias_y") = getParam<Real>("bias_y");
  params.set<Real>("bias_z") = getParam<Real>("bias_z");

  params.set<unsigned>("num_side_layers") = getParam<unsigned>("num_side_layers");
  params.set<processor_id_type>("num_cores_for_partition") =
      getParam<processor_id_type>("num_cores_for_partition");

  params.set<MooseEnum>("partition") = getParam<MooseEnum>("partition");
  params.set<MooseEnum>("elem_type") = getParam<MooseEnum>("elem_type");

  _build_mesh = &addMeshSubgenerator("DistributedRectilinearMeshGenerator",
                                     name() + "_distributedrectilinearmeshgenerator",
                                     params);
}
std::unique_ptr<MeshBase>
OverlayMeshGenerator::generate()
{
  auto bbox_input = MeshTools::create_bounding_box(*_mesh_input);

  RealVectorValue scale_factor;
  scale_factor = bbox_input.max() - bbox_input.min();

  if (scale_factor(0) != 1 || scale_factor(1) != 1 || scale_factor(2) != 1)
    MeshTools::Modification::scale(
        *(*_build_mesh), scale_factor(0), scale_factor(1), scale_factor(2));

  RealVectorValue translate_factor;
  translate_factor = bbox_input.min();
  if (translate_factor(0) != 0 || translate_factor(1) != 0 || translate_factor(2) != 0)
    MeshTools::Modification::translate(
        *(*_build_mesh), translate_factor(0), translate_factor(1), translate_factor(2));

  return std::move(*_build_mesh);
}
