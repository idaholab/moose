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

defineLegacyParams(ReactorMeshParams);

InputParameters
ReactorMeshParams::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  // MooseEnum dims = dimensions;
  MooseEnum dims("2=2 3", "2");
  params.addRequiredParam<MooseEnum>("dim", dims, "The dimension of the mesh to be generated");

  MooseEnum geoms("Square Hex", "Square");
  params.addRequiredParam<MooseEnum>("geom", geoms, "The geometry type of the reactor mesh");

  params.addRequiredParam<Real>("assembly_pitch", "Center to center distance of assemblies");
  params.addParam<std::vector<Real>>(
      "axial_regions", std::vector<Real>(1), "Length of each axial region");
  params.addParam<std::vector<unsigned int>>(
      "axial_mesh_intervals",
      std::vector<unsigned int>(1),
      "Number of elements in the Z direction for each axial region");
  params.addParam<bool>("procedural_region_ids",
                        false,
                        "Whether to enable the automatic generation of region and block IDs in "
                        "subsequent MeshGenerators");
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
    mooseError("The number of axial regions is not consistant.");


  this->declareMeshProperty("mesh_dimensions", int(_dim));
  this->declareMeshProperty("mesh_geometry", std::string(_geom));
  this->declareMeshProperty("axial_boundaries", _axial_regions);
  this->declareMeshProperty("axial_mesh_intervals", _axial_mesh_intervals);
  this->declareMeshProperty("assembly_pitch", _assembly_pitch);
  this->declareMeshProperty("procedural_ids", getParam<bool>("procedural_region_ids"));
}

std::unique_ptr<MeshBase>
ReactorMeshParams::generate()
{
  auto mesh = buildMeshBaseObject();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
