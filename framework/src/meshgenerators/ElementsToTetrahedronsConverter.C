//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementsToTetrahedronsConverter.h"
#include "MooseMeshElementConversionUtils.h"

#include "libmesh/elem.h"
#include "libmesh/boundary_info.h"
#include "libmesh/mesh_base.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/cell_tet4.h"
#include "libmesh/face_tri3.h"

// C++ includes
#include <cmath>

registerMooseObject("MooseApp", ElementsToTetrahedronsConverter);

InputParameters
ElementsToTetrahedronsConverter::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>(
      "input", "The input mesh that needs to be converted to tetrahedral elements.");

  params.addClassDescription(
      "This ElementsToTetrahedronsConverter object is designed to convert all the elements in a 3D "
      "mesh consisting only linear elements into TET4 elements.");

  return params;
}

ElementsToTetrahedronsConverter::ElementsToTetrahedronsConverter(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input_name(getParam<MeshGeneratorName>("input")),
    _input(getMeshByName(_input_name))
{
}

std::unique_ptr<MeshBase>
ElementsToTetrahedronsConverter::generate()
{
  auto replicated_mesh_ptr = dynamic_cast<ReplicatedMesh *>(_input.get());
  if (!replicated_mesh_ptr)
    paramError("input", "Input is not a replicated mesh, which is required");
  if (*(replicated_mesh_ptr->elem_dimensions().begin()) != 3 ||
      *(replicated_mesh_ptr->elem_dimensions().rbegin()) != 3)
    paramError("input", "Only 3D meshes are supported.");

  ReplicatedMesh & mesh = *replicated_mesh_ptr;

  MooseMeshElementConversionUtils::convert3DMeshToAllTet4(mesh);

  return std::move(_input);
}
