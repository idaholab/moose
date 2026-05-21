//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  // We're querying elem dim caches from our input mesh
  if (!_input->preparation().has_cached_elem_data)
    _input->cache_elem_data();

  if (!_input->is_serial())
    paramError("input", "Input is mesh not serialized, which is required");
  if (*(_input->elem_dimensions().begin()) != 3 || *(_input->elem_dimensions().rbegin()) != 3)
    paramError("input", "Only 3D meshes are supported.");

  MooseMeshElementConversionUtils::convert3DMeshToAllTet4(*_input);

  return std::move(_input);
}
