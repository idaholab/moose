//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeometricalComponent.h"
#include "ConstantFunction.h"
#include "THMMesh.h"

InputParameters
GeometricalComponent::validParams()
{
  InputParameters params = Component::validParams();
  params += DiscreteLineSegmentInterface::validParams();

  params.addParam<std::vector<std::string>>("axial_region_names",
                                            "Names to assign to axial regions");

  return params;
}

GeometricalComponent::GeometricalComponent(const InputParameters & parameters)
  : Component(parameters),
    DiscreteLineSegmentInterface(this),

    _axial_region_names(getParam<std::vector<std::string>>("axial_region_names")),
    _displace_node_user_object_name(genName(name(), "displace_node"))
{
  checkSizeGreaterThan<Real>("length", 0);
  checkEqualSize<Real, unsigned int>("length", "n_elems");
  if (_axial_region_names.size() > 0)
    checkEqualSize<Real, std::string>("length", "axial_region_names");
}

unsigned int
GeometricalComponent::computeNumberOfNodes(unsigned int n_elems)
{
  return usingSecondOrderMesh() ? (2 * n_elems) + 1 : n_elems + 1;
}

Node *
GeometricalComponent::addNode(const Point & pt)
{
  auto node = mesh().addNode(pt);
  _node_ids.push_back(node->id());
  return node;
}

Elem *
GeometricalComponent::addElement(libMesh::ElemType elem_type,
                                 const std::vector<dof_id_type> & node_ids)
{
  auto elem = mesh().addElement(elem_type, node_ids);
  _elem_ids.push_back(elem->id());
  return elem;
}

Elem *
GeometricalComponent::addElementEdge2(dof_id_type node0, dof_id_type node1)
{
  auto elem = mesh().addElementEdge2(node0, node1);
  _elem_ids.push_back(elem->id());
  return elem;
}

Elem *
GeometricalComponent::addElementEdge3(dof_id_type node0, dof_id_type node1, dof_id_type node2)
{
  auto elem = mesh().addElementEdge3(node0, node1, node2);
  _elem_ids.push_back(elem->id());
  return elem;
}

Elem *
GeometricalComponent::addElementQuad4(dof_id_type node0,
                                      dof_id_type node1,
                                      dof_id_type node2,
                                      dof_id_type node3)
{
  auto elem = mesh().addElementQuad4(node0, node1, node2, node3);
  _elem_ids.push_back(elem->id());
  return elem;
}

Elem *
GeometricalComponent::addElementQuad9(dof_id_type node0,
                                      dof_id_type node1,
                                      dof_id_type node2,
                                      dof_id_type node3,
                                      dof_id_type node4,
                                      dof_id_type node5,
                                      dof_id_type node6,
                                      dof_id_type node7,
                                      dof_id_type node8)
{
  auto elem = mesh().addElementQuad9(node0, node1, node2, node3, node4, node5, node6, node7, node8);
  _elem_ids.push_back(elem->id());
  return elem;
}

void
GeometricalComponent::setupMesh()
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
GeometricalComponent::check() const
{
  Component::check();

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

const std::vector<SubdomainName> &
GeometricalComponent::getSubdomainNames() const
{
  checkSetupStatus(MESH_PREPARED);

  return _subdomain_names;
}

const std::vector<Moose::CoordinateSystemType> &
GeometricalComponent::getCoordSysTypes() const
{
  checkSetupStatus(MESH_PREPARED);

  return _coord_sys;
}

void
GeometricalComponent::generateNodeLocations()
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
GeometricalComponent::getUniformNodeLocations(Real length, unsigned int n_nodes)
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
GeometricalComponent::placeLocalNodeLocations(Real start_length,
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

const FunctionName &
GeometricalComponent::getVariableFn(const FunctionName & fn_param_name)
{
  const FunctionName & fn_name = getParam<FunctionName>(fn_param_name);
  const Function & fn = getTHMProblem().getFunction(fn_name);

  if (dynamic_cast<const ConstantFunction *>(&fn) != nullptr)
  {
    connectObject(fn.parameters(), fn_name, fn_param_name, "value");
  }

  return fn_name;
}

void
GeometricalComponent::setSubdomainInfo(SubdomainID subdomain_id,
                                       const std::string & subdomain_name,
                                       const Moose::CoordinateSystemType & coord_system)
{
  _subdomain_ids.push_back(subdomain_id);
  _subdomain_names.push_back(subdomain_name);
  _coord_sys.push_back(coord_system);
  if (_parent)
  {
    GeometricalComponent * gc = dynamic_cast<GeometricalComponent *>(_parent);
    gc->_subdomain_ids.push_back(subdomain_id);
    gc->_subdomain_names.push_back(subdomain_name);
    gc->_coord_sys.push_back(coord_system);
  }
  mesh().setSubdomainName(subdomain_id, subdomain_name);
}

const std::vector<dof_id_type> &
GeometricalComponent::getNodeIDs() const
{
  checkSetupStatus(MESH_PREPARED);

  return _node_ids;
}

const std::vector<dof_id_type> &
GeometricalComponent::getElementIDs() const
{
  checkSetupStatus(MESH_PREPARED);

  return _elem_ids;
}
