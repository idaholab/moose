//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneratedMeshComponent.h"
#include "THMMesh.h"

InputParameters
GeneratedMeshComponent::validParams()
{
  InputParameters params = GeometricalComponent::validParams();
  params += DiscreteLineSegmentInterface::validParams();

  params.addParam<std::vector<std::string>>("axial_region_names",
                                            "Names to assign to axial regions");

  return params;
}

GeneratedMeshComponent::GeneratedMeshComponent(const InputParameters & parameters)
  : GeometricalComponent(parameters),
    DiscreteLineSegmentInterface(this),

    _axial_region_names(getParam<std::vector<std::string>>("axial_region_names"))
{
  checkSizeGreaterThan<Real>("length", 0);
  checkEqualSize<Real, unsigned int>("length", "n_elems");
  if (_axial_region_names.size() > 0)
    checkEqualSize<Real, std::string>("length", "axial_region_names");
}

void
GeneratedMeshComponent::setupMesh()
{
  generateNodeLocations();

  buildMesh();

  // displace nodes
  for (auto && node_id : _node_ids)
  {
    Node & curr_node = mesh().nodeRef(node_id);
    RealVectorValue p(curr_node(0), curr_node(1), curr_node(2));
    curr_node = computeRealPointFromReferencePoint(p);
  }
}

void
GeneratedMeshComponent::check() const
{
  GeometricalComponent::check();

  // Do not use TRAP q-rule with 2nd order FEs
  if (usingSecondOrderMesh())
  {
    auto actions = _app.actionWarehouse().getActionListByName("setup_quadrature");
    const MooseEnum & quadrature_type = (*actions.begin())->getParam<MooseEnum>("type");

    if (quadrature_type == "TRAP")
      logError("Cannot use TRAP quadrature rule with 2nd order elements.  Use SIMPSON or GAUSS "
               "instead.");
  }
}

unsigned int
GeneratedMeshComponent::computeNumberOfNodes(unsigned int n_elems)
{
  return usingSecondOrderMesh() ? (2 * n_elems) + 1 : n_elems + 1;
}

void
GeneratedMeshComponent::generateNodeLocations()
{
  unsigned int n_nodes = computeNumberOfNodes(_n_elem);
  unsigned int start_node = 0;
  Real start_length = 0.0;
  _node_locations = std::vector<Real>(n_nodes);
  _node_locations[0] = start_length;

  for (unsigned int i = 0; i < _n_sections; ++i)
  {
    Real section_length = _lengths[i];
    Real section_n_elems = _n_elems[i];
    Real section_n_nodes = computeNumberOfNodes(section_n_elems);

    std::vector<Real> section_node_array = getUniformNodeLocations(section_length, section_n_nodes);
    placeLocalNodeLocations(start_length, start_node, section_node_array);

    start_length += section_length;
    start_node += (section_n_nodes - 1);
  }
}

std::vector<Real>
GeneratedMeshComponent::getUniformNodeLocations(Real length, unsigned int n_nodes)
{
  std::vector<Real> node_locations(n_nodes);
  Real dx = length / (n_nodes - 1);

  node_locations[0] = 0.0;

  for (unsigned int i = 1; i < (n_nodes - 1); ++i)
    node_locations[i] = node_locations[i - 1] + dx;

  node_locations[n_nodes - 1] = length;
  return node_locations;
}

void
GeneratedMeshComponent::placeLocalNodeLocations(Real start_length,
                                                unsigned int start_node,
                                                std::vector<Real> & local_node_locations)
{
  unsigned int n_nodes = local_node_locations.size();
  for (unsigned int i = 1; i < n_nodes; ++i)
  {
    unsigned int global_i = i + start_node;
    Real local_node_location = local_node_locations[i];
    _node_locations[global_i] = start_length + local_node_location;
  }
}
