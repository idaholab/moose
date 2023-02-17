//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BlockCartesianGenerator.h"
#include "CastUniquePointer.h"

// libMesh includes
#include "libmesh/mesh_generation.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/mesh_communication.h"
#include "libmesh/remote_elem.h"
#include "libmesh/partitioner.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/periodic_boundary_base.h"

registerMooseObject("MooseApp", BlockCartesianGenerator);

InputParameters
BlockCartesianGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription("This BlockCartesianGenerator creates a Cartesian mesh using "
                             "DistributedRectilinearMeshGenerator in "
                             "the mesh block region.");
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  MooseEnum dims("1=1 2 3");
  params.addRequiredParam<MooseEnum>("dim", dims, "The dimension of the mesh to be generated");

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

  params.addParamNamesToGroup("dim", "Main");

  return params;
}

BlockCartesianGenerator::BlockCartesianGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _dim(getParam<MooseEnum>("dim")),
    _nx(declareMeshProperty("num_elements_x", getParam<dof_id_type>("nx"))),
    _ny(declareMeshProperty("num_elements_y", getParam<dof_id_type>("ny"))),
    _nz(declareMeshProperty("num_elements_z", getParam<dof_id_type>("nz"))),
    // _num_cores_for_partition(getParam<processor_id_type>("num_cores_for_partition")),
    _bias_x(getParam<Real>("bias_x")),
    _bias_y(getParam<Real>("bias_y")),
    _bias_z(getParam<Real>("bias_z")),
    //  _num_parts_per_compute_node(getParam<processor_id_type>("num_cores_per_compute_node")),
    _partition_method(getParam<MooseEnum>("partition")),
    _num_side_layers(getParam<unsigned>("num_side_layers")),
    _mesh_input(getMesh("input"))
{
  auto bbox_input = MeshTools::create_bounding_box(*_mesh_input);
  Real xmin = bbox_input.min()(0);
  Real ymin = bbox_input.min()(1);
  Real zmin = bbox_input.min()(2);
  Real xmax = bbox_input.max()(0);
  Real ymax = bbox_input.max()(1);
  Real zmax = bbox_input.max()(2);

  auto params = _app.getFactory().getValidParams("DistributedRectilinearMeshGenerator");

  params.set<MooseEnum>("dim") = _dim;
  params.set<Real>("xmin") = xmin;
  params.set<Real>("ymin") = ymin;
  params.set<Real>("zmin") = zmin;
  params.set<Real>("xmax") = xmax;
  params.set<Real>("ymax") = ymax;
  params.set<Real>("zmax") = zmax;

  params.set<dof_id_type>("num_elements_x") = _nx;
  params.set<dof_id_type>("num_elements_y") = _ny;
  params.set<dof_id_type>("num_elements_z") = _nz;

  // params.set<processor_id_type>("num_cores_for_partition") = _num_cores_for_partition;

  // generate lower dimensional mesh from the given sideset
  _build_mesh = &addMeshSubgenerator("DistributedRectilinearMeshGenerator",
                                     name() + "_DistributedRectilinearmeshgenerator",
                                     params);
}

std::unique_ptr<MeshBase>
BlockCartesianGenerator::generate()
{
  // auto bbox_input = MeshTools::create_bounding_box(*_mesh_input);
  //  auto bbox = MeshTools::create_bounding_box(_build_mesh);

  // Real diff = 0;
  // for (auto i = 0; i < _dim; i++)
  // {
  //   diff = std::abs(bbox_input.max()(i) - bbox_input.min()(i) - bbox.max()(i) + bbox.min()(i));
  //   if (diff > TOLERANCE)
  //     mooseError("The Intervals in ", i, "th dimension doesn't match!");
  // }

  return std::move(*_build_mesh);
}
