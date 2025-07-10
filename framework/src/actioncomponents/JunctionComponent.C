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
registerActionComponent("MooseApp", JunctionComponent);

InputParameters
JunctionComponent::validParams()
{
  InputParameters params = ActionComponent::validParams();
  params.addClassDescription("Component to join two other components.");

  params.addParam<ComponentName>("first_component", "First component to join");
  params.addParam<ComponentName>("second_component", "Second component to join");
  MooseEnum junction_type("physics_defined stitch_meshes fill_gap");
  params.addParam<MooseEnum>("junction_methods", junction_type, "How to join the two components");

  // Parameters for the region between meshes
  params.addParam<unsigned int>(
      "n_elem_normal", 0, "Number of elements in the normal direction of the junction");
  params.addParam<SubdomainName>("block", "Block name for the junction, if a block is created.");

  return params;
}

JunctionComponent::JunctionComponent(const InputParameters & params)
  : ActionComponent(params), _elem(getParam<unsigned int>("n_elem_normal"))
{
  addRequiredTask("add_mesh_generator");

  // Check parameters
  if (!_junction_method.contains("fill_gap"))
    errorDependentParameters("junction_method", "fill_gap_and_stitch", {"n_elem_normal", "block"});
}

void
JunctionComponent::addMeshGenerators()
{
  const auto & first_component = getActionComponent(getParam<ComponentName>("first_component"));
  const auto & second_component = getActionComponent(getParam<ComponentName>("second_component"));

  // Get the dimension of the components
  const auto dimension_first = first_component.dimension();
  const auto dimension_second = second_component.dimension();

  // Perform junction
  if (_junction_method == "stitch_meshes")
  {
    // Fairly easy to stitch this
    if (dimension_first == dimension_second)
    {
      // Stitch the two meshes
    }
    else
      mooseError("Stiching meshes of different dimensions is not implemented");
  }
  else if (_junction_method == "fill_gap")
  {
    if (std::min(dimension_first, dimension_second) <= 1)
    {
      // Create a 1D line of the length between the two meshes
    }
    else if (std::min(dimension_first, dimension_second) <= 2)
    {
      // Use a 2D mesh
    }
    else
    {
      // Build 3D sides around the junction
      // Tetrahedralize inside
    }
  }
}