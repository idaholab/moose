//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "CylinderComponent.h"
#include "RotationMatrix.h"

registerMooseAction("MooseApp", CylinderComponent, "add_mesh_generator");
registerMooseAction("MooseApp", CylinderComponent, "init_component_physics");
registerActionComponent("MooseApp", CylinderComponent);

InputParameters
CylinderComponent::validParams()
{
  InputParameters params = ActionComponent::validParams();
  params += PhysicsComponentBase::validParams();
  params.addClassDescription("Cylindrical component.");
  MooseEnum dims("0 1 2 3");
  params.addRequiredParam<MooseEnum>("dimension",
                                     dims,
                                     "Dimension of the cylinder. 0 for a point (not implemented), "
                                     "1 for an (axial) 1D line, 2 for a 2D-RZ cylinder, and 3 for "
                                     "a 3D cylinder (not implemented)");
  params.addRequiredRangeCheckedParam<Real>("radius", "radius>0", "Radius of the cylinder");
  params.addRequiredRangeCheckedParam<Real>("length", "length>0", "Length/Height of the cylinder");
  params.addRequiredParam<Point>("position", "Positional offset of the cylinder");
  params.addRequiredParam<Point>("direction", "Direction of the cylinder");

  params.addRequiredParam<unsigned int>("n_axial", "Number of axial elements of the cylinder");
  params.addParam<unsigned int>("n_radial", "Number of radial elements of the cylinder");
  params.addParam<unsigned int>("n_azimuthal", "Number of azimuthal elements of the cylinder");

  params.addParam<SubdomainName>("block", "Block name for the cylinder");

  return params;
}

CylinderComponent::CylinderComponent(const InputParameters & params)
  : ActionComponent(params),
    PhysicsComponentBase(params),
    _radius(getParam<Real>("radius")),
    _height(getParam<Real>("length"))
{
  _dimension = getParam<MooseEnum>("dimension");
  addRequiredTask("add_mesh_generator");
}

void
CylinderComponent::addMeshGenerators()
{
  // Create the base mesh for the component using a mesh generator
  if (_dimension == 0)
    paramError("dimension", "0D cylinder not implemented");
  else if (_dimension == 1 || _dimension == 2)
  {
    InputParameters params = _factory.getValidParams("GeneratedMeshGenerator");
    params.set<MooseEnum>("dim") = _dimension;
    params.set<Real>("xmax") = {getParam<Real>("length")};
    params.set<unsigned int>("nx") = {getParam<unsigned int>("n_axial")};
    params.set<std::string>("boundary_name_prefix") = name();
    if (_dimension == 2)
    {
      params.set<Real>("ymax") = {getParam<Real>("radius")};
      if (!isParamValid("n_radial"))
        paramError("n_radial", "Should be provided for a 2D cylinder");
      params.set<unsigned int>("ny") = {getParam<unsigned int>("n_radial")};
    }
    else if (isParamValid("n_radial"))
      paramError("n_radial", "Should not be provided for a 1D cylinder");
    if (isParamValid("block"))
    {
      const auto block_name = getParam<SubdomainName>("block");
      params.set<SubdomainName>("subdomain_name") = block_name;
      _blocks.push_back(block_name);
    }
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "GeneratedMeshGenerator", name() + "_base", params);
    _mg_names.push_back(name() + "_base");
  }
  else
  {
    paramError("dimension", "3D cylinder is not implemented");
    if (!isParamValid("n_radial"))
      paramError("n_radial", "Should be provided for a 3D cylinder");
    if (!isParamValid("n_azimuthal"))
      paramError("n_azimuthal", "Should be provided in 3D");
  }

  // Place mesh into position
  {
    InputParameters params = _factory.getValidParams("TransformGenerator");
    params.set<MeshGeneratorName>("input") = _mg_names.back();
    params.set<MooseEnum>("transform") = "TRANSLATE";
    params.set<RealVectorValue>("vector_value") = getParam<Point>("position");
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "TransformGenerator", name() + "_translated", params);
    _mg_names.push_back(name() + "_translated");
  }
  // Rotate the mesh as desired
  {
    InputParameters params = _factory.getValidParams("TransformGenerator");
    params.set<MeshGeneratorName>("input") = _mg_names.back();
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
    _mg_names.push_back(name() + "_rotated");
  }
}

void
CylinderComponent::setupComponent()
{
  if (_dimension == 2)
    _awh.getMesh()->setCoordSystem(_blocks, MultiMooseEnum("COORD_RZ"));
}
