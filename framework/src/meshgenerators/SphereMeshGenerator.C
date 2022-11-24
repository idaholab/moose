//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  params.addClassDescription("Generate a 3-D sphere mesh centered on the origin");
  params.addRequiredRangeCheckedParam<Real>("radius", "radius > 0.0", "Sphere radius");
  params.addRequiredRangeCheckedParam<unsigned int>("nr", "nr > 0", "Number of radial elements");

  MooseEnum types("HEX8 HEX27", "HEX8");
  params.addParam<MooseEnum>("elem_type", types, "The type of element to generate");
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
  mesh->set_mesh_dimension(3);
  mesh->set_spatial_dimension(3);

  ElemType et = Utility::string_to_enum<ElemType>(_elem_type);

  MeshTools::Generation::build_sphere(static_cast<UnstructuredMesh &>(*mesh),
                                      _radius,
                                      _nr,
                                      et,
                                      _n_smooth,
                                      false /* dummy value; not used for 3-D meshes */);

  return dynamic_pointer_cast<MeshBase>(mesh);
}
