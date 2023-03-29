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
#include "DistributedRectilinearMeshGenerator.h"
#include "InputParameters.h"

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

  params += DistributedRectilinearMeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The base mesh we want to overlay");

  params.addClassDescription("Creates a Cartesian mesh overlaying "
                             "the input mesh region.");

  return params;
}

OverlayMeshGenerator::OverlayMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _dim(getParam<MooseEnum>("dim")),
    _mesh_name(getParam<MeshGeneratorName>("input"))
{
  // Declare that all of the meshes in the "inputs" parameter are to be used by
  // a sub mesh generator
  declareMeshForSub("input");

  _input_mesh = &getMeshByName(_mesh_name);

  auto input_params = _app.getFactory().getValidParams("DistributedRectilinearMeshGenerator");

  input_params.applySpecificParameters(parameters,
                                       {"dim",
                                        "nx",
                                        "ny",
                                        "nz",
                                        "xmin",
                                        "ymin",
                                        "zmin",
                                        "xmax",
                                        "ymax",
                                        "zmax",
                                        "bias_x",
                                        "bias_y",
                                        "bias_z",
                                        "num_side_layers",
                                        "num_cores_for_partition",
                                        "partition",
                                        "elem_type"});

  addMeshSubgenerator("DistributedRectilinearMeshGenerator",
                      _mesh_name + "_distributedrectilinearmeshgenerator",
                      input_params);
  _build_mesh = &getMeshByName(_mesh_name + "_distributedrectilinearmeshgenerator");
}
std::unique_ptr<MeshBase>
OverlayMeshGenerator::generate()
{
  std::unique_ptr<MeshBase> input_mesh = std::move(*_input_mesh);
  std::unique_ptr<MeshBase> build_mesh = std::move(*_build_mesh);

  // find the boundary of the input mesh box
  auto bbox_input = MeshTools::create_bounding_box(*input_mesh);

  // Transform the generated DistributedRectilinearMesh to overlay with the input mesh
  RealVectorValue scale_factor;
  scale_factor = bbox_input.max() - bbox_input.min();

  // scale
  if (scale_factor(0) != 1 || scale_factor(1) != 1 || scale_factor(2) != 1)
    MeshTools::Modification::scale(*build_mesh, scale_factor(0), scale_factor(1), scale_factor(2));

  RealVectorValue translation_vector;
  translation_vector = bbox_input.min();

  // translate
  if (translation_vector(0) != 0 || translation_vector(1) != 0 || translation_vector(2) != 0)
    MeshTools::Modification::translate(
        *build_mesh, translation_vector(0), translation_vector(1), translation_vector(2));

  return dynamic_pointer_cast<MeshBase>(build_mesh);
}
