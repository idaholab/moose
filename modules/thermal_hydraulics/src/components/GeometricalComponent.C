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
  return params;
}

GeometricalComponent::GeometricalComponent(const InputParameters & parameters)
  : Component(parameters)
{
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
