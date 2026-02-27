//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "CylinderComponent.h"

registerMooseAction("MooseApp", CylinderComponent, "add_mesh_generator");
// CylinderComponent is an example of ComponentPhysicsInterface
registerMooseAction("MooseApp", CylinderComponent, "init_component_physics");
// CylinderComponent is an example of ComponentMaterialPropertyInterface
registerMooseAction("MooseApp", CylinderComponent, "add_material");
// CylinderComponent is an example of ComponentInitialConditionInterface
registerMooseAction("MooseApp", CylinderComponent, "check_integrity");
registerActionComponent("MooseApp", CylinderComponent);

InputParameters
CylinderComponent::validParams()
{
  InputParameters params = ActionComponent::validParams();
  params += ComponentPhysicsInterface::validParams();
  params += ComponentMaterialPropertyInterface::validParams();
  params += ComponentInitialConditionInterface::validParams();
  params += ComponentBoundaryConditionInterface::validParams();
  params += ComponentMeshTransformHelper::validParams();
  params.addClassDescription("Cylindrical component.");
  MooseEnum dims("0 1 2 3");
  params.addRequiredParam<MooseEnum>("dimension",
                                     dims,
                                     "Dimension of the cylinder. 0 for a point (not implemented), "
                                     "1 for an (axial) 1D line, 2 for a 2D-RZ cylinder, and 3 for "
                                     "a 3D cylinder (not implemented)");
  params.addRequiredRangeCheckedParam<Real>("radius", "radius>0", "Radius of the cylinder");
  params.addRequiredRangeCheckedParam<Real>("length", "length>0", "Length/Height of the cylinder");

  params.addParam<unsigned int>("n_radial",
                                "Number of radial elements. Only assign if mesh is 2D!");
  params.addParam<unsigned int>(
      "n_radial_rings",
      1,
      "Number of radial rings within the mesh (does not affect boundary layers).");

  params.addRequiredParam<unsigned int>("n_axial", "Number of axial elements of the cylinder");
  params.addRangeCheckedParam<Real>("boundary_layer_width",
                                    "boundary_layer_width>=0",
                                    "The width of the boundary layer (if assigned).");
  params.addParam<unsigned int>("n_boundary_layers", 1, "The number of boundary layers.");
  params.addParam<unsigned int>("n_sectors",
                                "Number of azimuthal sectors in each quadrant of cylinder face.");

  params.addParam<SubdomainName>("block", "Block name for the cylinder");

  return params;
}

CylinderComponent::CylinderComponent(const InputParameters & params)
  : ActionComponent(params),
    ComponentPhysicsInterface(params),
    ComponentMaterialPropertyInterface(params),
    ComponentInitialConditionInterface(params),
    ComponentBoundaryConditionInterface(params),
    ComponentMeshTransformHelper(params),
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
      params.set<Real>("ymin") = {-getParam<Real>("radius")};
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
  else if (_dimension == 3)
  {
    if (!isParamValid("n_axial"))
      paramError("n_axial", "Should be provided for a 3D cylinder");
    if (!isParamValid("n_sectors"))
      paramError("n_sectors", "Should be provided in 3D");

    // create circular face
    InputParameters circle_params = _factory.getValidParams("ConcentricCircleMeshGenerator");
    circle_params.set<bool>("preserve_volumes") = true;
    circle_params.set<bool>("has_outer_square") = false;
    if (isParamValid("boundary_layer_width"))
    {
      Real inner_radius = getParam<Real>("radius") - getParam<Real>("boundary_layer_width");
      circle_params.set<std::vector<Real>>("radii") = {inner_radius, getParam<Real>("radius")};
      circle_params.set<std::vector<unsigned int>>("rings") = {
          getParam<unsigned int>("n_radial_rings"), getParam<unsigned int>("n_boundary_layers")};
    }
    else
    {
      circle_params.set<std::vector<Real>>("radii") = {getParam<Real>("radius")};
      circle_params.set<std::vector<unsigned int>>("rings") = {
          getParam<unsigned int>("n_radial_rings")};
    }

    circle_params.set<unsigned int>("num_sectors") = getParam<unsigned int>("n_sectors");
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "ConcentricCircleMeshGenerator", name() + "_circle_base", circle_params);
    _mg_names.push_back(name() + "_circle_base");

    // rotate to have extrusion axis be along x-axis
    InputParameters rotate_params = _factory.getValidParams("TransformGenerator");
    rotate_params.set<MeshGeneratorName>("input") = _mg_names.back();
    rotate_params.set<MooseEnum>("transform") = "ROTATE_EXT";
    RealVectorValue angles(-90, -90, 90);
    rotate_params.set<RealVectorValue>("vector_value") = angles;
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "TransformGenerator", name() + "_3D_init_rotate", rotate_params);
    _mg_names.push_back(name() + "_3D_init_rotate");

    // extrude surface
    InputParameters ext_params = _factory.getValidParams("AdvancedExtruderGenerator");
    ext_params.set<std::vector<unsigned int>>("num_layers") = {getParam<unsigned int>("n_axial")};
    ext_params.set<MeshGeneratorName>("input") = _mg_names.back();
    ext_params.set<std::vector<Real>>("heights") = {_height};
    ext_params.set<BoundaryName>("bottom_boundary") = name() + "_bottom_boundary";
    ext_params.set<BoundaryName>("top_boundary") = name() + "_top_boundary";

    const Point default_direction(1, 0, 0);
    ext_params.set<Point>("direction") = default_direction;
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "AdvancedExtruderGenerator", name() + "_base", ext_params);
    _mg_names.push_back(name() + "_base");
  }

  ComponentMeshTransformHelper::addMeshGenerators();
}

void
CylinderComponent::setupComponent()
{
  if (_dimension == 2)
    _awh.getMesh()->setCoordSystem(_blocks, MultiMooseEnum("COORD_RZ"));
}

void
CylinderComponent::checkIntegrity()
{
  ComponentInitialConditionInterface::checkIntegrity();
  ComponentBoundaryConditionInterface::checkIntegrity();
}
