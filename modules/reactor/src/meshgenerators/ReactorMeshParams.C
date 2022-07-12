//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReactorMeshParams.h"
#include "CastUniquePointer.h"

// libMesh includes
#include "libmesh/mesh_generation.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/point.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"

registerMooseObject("ReactorApp", ReactorMeshParams);

InputParameters
ReactorMeshParams::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  MooseEnum dims("2=2 3", "2");
  params.addRequiredParam<MooseEnum>("dim", dims, "The dimension of the mesh to be generated");

  MooseEnum geoms("Square Hex", "Square");
  params.addRequiredParam<MooseEnum>("geom", geoms, "The geometry type of the reactor mesh");

  params.addRequiredParam<Real>("assembly_pitch", "Center to center distance of assemblies");
  params.addParam<boundary_id_type>("top_boundary_id",
                                    "The boundary ID to set on top boundary of the extruded mesh");
  params.addParam<boundary_id_type>(
      "bottom_boundary_id", "The boundary ID to set on bottom boundary of the extruded mesh");
  params.addParam<boundary_id_type>(
      "radial_boundary_id",
      "The boundary ID to set on the outer radial boundary of a CoreMeshGenerator object");
  params.addParam<std::vector<Real>>(
      "axial_regions", std::vector<Real>(1), "Length of each axial region");
  params.addParam<std::vector<unsigned int>>(
      "axial_mesh_intervals",
      std::vector<unsigned int>(1),
      "Number of elements in the Z direction for each axial region");
  params.addClassDescription("This ReactorMeshParams object acts as storage for persistent "
                             "information about the reactor geometry.");
  return params;
}

ReactorMeshParams::ReactorMeshParams(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _dim(getParam<MooseEnum>("dim")),
    _geom(getParam<MooseEnum>("geom")),
    _assembly_pitch(getParam<Real>("assembly_pitch")),
    _axial_regions(getParam<std::vector<Real>>("axial_regions")),
    _axial_mesh_intervals(getParam<std::vector<unsigned int>>("axial_mesh_intervals"))
{
  if (_axial_regions.size() != _axial_mesh_intervals.size())
    mooseError("The number of axial regions is not consistent with the number of axial intervals.");

  this->declareMeshProperty("mesh_dimensions", int(_dim));
  this->declareMeshProperty("mesh_geometry", std::string(_geom));
  this->declareMeshProperty("axial_boundaries", _axial_regions);
  this->declareMeshProperty("axial_mesh_intervals", _axial_mesh_intervals);
  this->declareMeshProperty("assembly_pitch", _assembly_pitch);

  this->declareMeshProperty("name_id_map", _name_id_map);

  if (isParamValid("top_boundary_id"))
  {
    _top_boundary = getParam<boundary_id_type>("top_boundary_id");
    this->declareMeshProperty("top_boundary_id", _top_boundary);
  }
  if (isParamValid("bottom_boundary_id"))
  {
    _bottom_boundary = getParam<boundary_id_type>("bottom_boundary_id");
    this->declareMeshProperty("bottom_boundary_id", _bottom_boundary);
  }
  if (isParamValid("radial_boundary_id"))
  {
    _radial_boundary = getParam<boundary_id_type>("radial_boundary_id");
    this->declareMeshProperty("radial_boundary_id", _radial_boundary);
    if (isParamValid("top_boundary_id") && _radial_boundary == _top_boundary)
      mooseError("top_boundary_id and radial_boundary_id must be unique values");
    if (isParamValid("bottom_boundary_id") && _radial_boundary == _bottom_boundary)
      mooseError("bottom_boundary_id and radial_boundary_id must be unique values");
  }
  if (isParamValid("top_boundary_id") && isParamValid("bottom_boundary_id") &&
      (_bottom_boundary == _top_boundary))
    mooseError("top_boundary_id and bottom_boundary_id must be unique values");
}

std::unique_ptr<MeshBase>
ReactorMeshParams::generate()
{
  auto mesh = buildMeshBaseObject();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
