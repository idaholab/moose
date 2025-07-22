//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "JunctionComponent.h"

registerMooseAction("MooseApp", JunctionComponent, "add_mesh_generator");
// JunctionComponent is an example of ComponentPhysicsInterface
registerMooseAction("MooseApp", JunctionComponent, "init_component_physics");
// JunctionComponent is an example of ComponentMaterialPropertyInterface
registerMooseAction("MooseApp", JunctionComponent, "add_material");
// JunctionComponent is an example of ComponentInitialConditionInterface
registerMooseAction("MooseApp", JunctionComponent, "check_integrity");
registerActionComponent("MooseApp", JunctionComponent);

InputParameters
JunctionComponent::validParams()
{
  InputParameters params = ActionComponent::validParams();
  params.addClassDescription("Component to join two other components.");

  params.addRequiredParam<ComponentName>("first_component", "First component to join");
  params.addRequiredParam<BoundaryName>("first_boundary", "First boundary to connect to.");
  params.addRequiredParam<ComponentName>("second_component", "Second component to join");
  params.addRequiredParam<BoundaryName>("second_boundary", "Second boundary to connect to.");

  MooseEnum junction_type("stitch_meshes fill_gap", "fill_gap");
  params.addParam<MooseEnum>("junction_method", junction_type, "How to join the two components");

  // Parameters for changing radius -- final radius will be calculated under the hood
  MooseEnum radial_growth_methods("LINEAR CUBIC", "LINEAR");
  params.addParam<MooseEnum>("radial_growth_method",
                             radial_growth_methods,
                             "Functional form to change radius while extruding along curve.");
  params.addParam<Real>("start_radial_growth_rate", 0, "Starting rate of radial expansion.");
  params.addParam<Real>("end_radial_growth_rate", 0, "Ending rate of radial expansion.");

  // Parameters for the region between meshes
  params.addParam<unsigned int>(
      "n_elem_normal", 0, "Number of elements in the normal direction of the junction");
  params.addParam<SubdomainName>("block", "Block name for the junction, if a block is created.");

  // add curve controls
  params.addRangeCheckedParam<libMesh::Real>(
      "sharpness", 0.6, "sharpness>0 & sharpness<=1", "Sharpness of curve bend.");
  params.addParam<unsigned int>(
      "num_cps",
      6,
      "Number of control points used to draw the curve. Miniumum of degree+1 points are required.");
  MooseEnum edge_elem_type("EDGE2 EDGE3 EDGE4", "EDGE2");
  params.addParam<MooseEnum>(
      "edge_element_type", edge_elem_type, "Type of the EDGE elements to be generated.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_elements", "num_elements>=1", "Numer of elements to be drawn. Must be at least 1.");

  // add parameters to an Advanced group for additional controls
  params.addParamNamesToGroup("sharpness num_cps edge_element_type num_elements", "Advanced");

  return params;
}

JunctionComponent::JunctionComponent(const InputParameters & params)
  : ActionComponent(params),
    ComponentPhysicsInterface(params),
    ComponentMaterialPropertyInterface(params),
    ComponentInitialConditionInterface(params),
    ComponentBoundaryConditionInterface(params),
    ComponentMeshTransformHelper(params),
    _junction_method(getParam<MooseEnum>("junction_method"))
{
  addRequiredTask("add_mesh_generator");

  // Check parameters
  // if (!_junction_method.contains("fill_gap"))
  //   errorDependentParameter("junction_method", "fill_gap_and_stitch", {"n_elem_normal",
  //   "block"});
}

// void
// JunctionComponent::addMeshGenerators()
// {
//   const auto & first_component = ActionComponent(getParam<ComponentName>("first_component"));
//   const auto & second_component = ActionComponent(getParam<ComponentName>("second_component"));
//   const auto first_boundary = getParam<BoundaryName>("first_boundary");
//   const auto second_boundary = getParam<BoundaryName>("second_boundary");

//   // Get the dimension of the components
//   const auto dimension_first = first_component.dimension();
//   const auto dimension_second = second_component.dimension();

//   if (dimension_first == 0 || dimension_second == 0)
//     mooseError("Connecting 0 dimension meshes not implemented!");

//   // Perform junction
//   if (_junction_method == "stitch_meshes")
//   {
//     // Fairly easy to stitch this
//     if (dimension_first == dimension_second)
//     {
//       // Stitch the two meshes
//       InputParameters params = _factory.getValidParams("StitchedMeshGenerator");
//       params.set<std::vector<MeshGeneratorName>>("inputs") =
//           _mg_names; // protected member of ActionComponent
//       params.set<std::vector<std::string>>("stitch_boundaries_pairs") = {first_boundary,
//                                                                          second_boundary};
//     }
//     else
//       mooseError("Stiching meshes of different dimensions is not implemented");
//   }
//   else if (_junction_method == "fill_gap")
//   {
//     // This method is set to use a B-Spline to draw a 1D curve between

//     // get mesh from boundaries

//     // determine start and end points

//     // find start and end directions (may need to take the negative of the negative direction)

//     // if (std::min(dimension_first, dimension_second) <= 1)
//     // {
//     //   // Create a 1D line of the length between the two meshes
//     // }
//     // else if (std::min(dimension_first, dimension_second) <= 2)
//     // {
//     //   // Use a 2D mesh
//     // }
//     // else
//     // {
//     //   // Build 3D sides around the junction
//     //   // Tetrahedralize inside
//     // }
//   }
//   else
//     mooseError("junction_method specified is invalid!");
// }

void
JunctionComponent::checkIntegrity()
{
  ComponentInitialConditionInterface::checkIntegrity();
  ComponentBoundaryConditionInterface::checkIntegrity();
}