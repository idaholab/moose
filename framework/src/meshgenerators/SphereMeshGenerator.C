//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SphereMeshGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/mesh_generation.h"
#include "libmesh/string_to_enum.h"

registerMooseObject("MooseApp", SphereMeshGenerator);

InputParameters
SphereMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription("Generate a sphere (ball) mesh centered on the origin");
  params.addRequiredRangeCheckedParam<Real>("radius", "radius > 0.0", "Sphere (ball) radius");
  params.addRequiredRangeCheckedParam<unsigned int>("nr", "nr > 0", "Number of radial elements");

  params.addParam<MooseEnum>(
      "elem_type", MooseMesh::elemTypes(), "The type of element to generate");
  params.addParam<unsigned int>("n_smooth", 0, "Number of smoothing operations");
  return params;
}

SphereMeshGenerator::SphereMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _radius(getParam<Real>("radius")),
    _nr(getParam<unsigned int>("nr")),
    _elem_type(getParam<MooseEnum>("elem_type")),
    _n_smooth(getParam<unsigned int>("n_smooth"))
{
}

std::unique_ptr<MeshBase>
SphereMeshGenerator::generate()
{
  auto mesh = buildMeshBaseObject();

  if (!isParamValid("elem_type"))
    _elem_type = "HEX8";

  // libMesh will determine sphere dimension based on ElemType, and
  // will throw an error with error message if the ElemType is not yet
  // implemented (prisms and pyramids, currently)
  ElemType et = Utility::string_to_enum<ElemType>(_elem_type);

  MeshTools::Generation::build_sphere(
      cast_ref<UnstructuredMesh &>(*mesh), _radius, _nr, et, _n_smooth);

  return dynamic_pointer_cast<MeshBase>(mesh);
}
