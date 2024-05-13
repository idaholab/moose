//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Cylinder.h"
#include "RotationMatrix.h"

registerMooseAction("MooseApp", Cylinder, "add_mesh_generator");
registerMooseAction("MooseApp", Cylinder, "add_positions");
registerMooseAction("MooseApp", Cylinder, "init_physics");

InputParameters
Cylinder::validParams()
{
  InputParameters params = ComponentAction::validParams();
  params.addClassDescription("Cylindrical component.");
  MooseEnum dims("0 1 2 3");
  params.addRequiredParam<MooseEnum>("dimension",
                                     dims,
                                     "Dimension of the cylinder. 1 for an (axial) 1D line, 2 for a "
                                     "2D-RZ cylinder, and 3 for a 3D cylinder");
  params.addRequiredRangeCheckedParam<Real>("radius", "radius>0", "Radius of the cylinder");
  params.addRequiredRangeCheckedParam<Real>("height", "height>0", "Height of the cylinder");
  params.addRequiredParam<Point>("position", "Positional offset of the cylinder");
  params.addRequiredParam<Point>("direction", "Direction of the cylinder");

  params.addParam<unsigned int>("n_axial", 1, "Axial mesh of the cylinder");
  params.addParam<unsigned int>("n_radial", 1, "Radial mesh of the cylinder");
  params.addParam<unsigned int>("n_azimuthal", 1, "Azimuthal mesh of the cylinder");

  params.addParam<std::vector<SubdomainName>>("block", "Block for the cylinder");

  params.addParam<std::vector<PhysicsName>>(
      "physics", {}, "Physics object(s) active on the Component");

  return params;
}

Cylinder::Cylinder(const InputParameters & params) : ComponentAction(params)
{
  _dimension = getParam<MooseEnum>("dimension");
}

void
Cylinder::addMeshGenerators()
{
  if (_dimension == 1 || _dimension == 2)
  {
    InputParameters params = _factory.getValidParams("GeneratedMeshGenerator");
    params.set<MooseEnum>("dim") = _dimension;
    params.set<Real>("xmax") = {getParam<Real>("height")};
    params.set<unsigned int>("nx") = {getParam<unsigned int>("n_axial")};
    params.set<std::string>("boundary_name_prefix") = name();
    if (_dimension == 2)
    {
      params.set<Real>("ymax") = {getParam<Real>("radius")};
      params.set<unsigned int>("ny") = {getParam<unsigned int>("n_radial")};
    }
    if (isParamValid("block"))
    {
      params.set<SubdomainName>("subdomain_name") =
          getParam<std::vector<SubdomainName>>("block")[0];
      mooseAssert(getParam<std::vector<SubdomainName>>("block").size() == 1,
                  "Coded only for size 1");
    }
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "GeneratedMeshGenerator", name() + "_base", params);
    _mg_name = name() + "_base";

    // TODO: Change coordinate system of block
  }
  else
  {
    mooseError("3D unsupported");
  }

  // Place it in position
  if (isParamValid("position"))
  {
    InputParameters params = _factory.getValidParams("TransformGenerator");
    params.set<MeshGeneratorName>("input") = _mg_name;
    params.set<MooseEnum>("transform") = "TRANSLATE";
    params.set<RealVectorValue>("vector_value") = getParam<Point>("position");
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "TransformGenerator", name() + "_translated", params);
    _mg_name = name() + "_translated";
  }
  // Rotate it as desired
  if (isParamValid("direction"))
  {
    InputParameters params = _factory.getValidParams("TransformGenerator");
    params.set<MeshGeneratorName>("input") = _mg_name;
    params.set<MooseEnum>("transform") = "ROTATE";
    const RealVectorValue direction = getParam<Point>("direction");
    const auto rotation_matrix =
        RotationMatrix::rotVec1ToVec2<false>(RealVectorValue(1, 0, 0), direction);
    RealVectorValue angles;
    angles(0) = std::atan2(rotation_matrix(1, 0), rotation_matrix(0, 0));
    angles(1) = std::asin(-rotation_matrix(2, 0));
    angles(2) = std::atan2(rotation_matrix(2, 1), rotation_matrix(2, 2));
    params.set<RealVectorValue>("vector_value") = angles / M_PI_2 * 90;
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "TransformGenerator", name() + "_rotated", params);
    _mg_name = name() + "_rotated";
  }
}

void
Cylinder::addPositionsObject()
{
  std::cout << "add_position got registered, wow. My name is " << name() << std::endl;
}

void
Cylinder::addPhysics()
{
  for (const auto & physics_name : getParam<std::vector<PhysicsName>>("physics"))
    _physics.push_back(getMooseApp().actionWarehouse().getPhysics<PhysicsBase>(physics_name));

  if (isParamValid("block"))
  {
    std::cout << "Block " << Moose::stringify(getParam<std::vector<SubdomainName>>("block"))
              << std::endl;
    for (auto physics : _physics)
    {
      _console << "adding " << physics->name() << std::endl;
      physics->addBlocks(getParam<std::vector<SubdomainName>>("block"));
    }
  }
}
