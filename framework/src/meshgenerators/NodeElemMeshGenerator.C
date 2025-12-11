//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodeElemMeshGenerator.h"

#include "libmesh/node.h"
#include "libmesh/node_elem.h"

registerMooseObject("MooseApp", NodeElemMeshGenerator);

InputParameters
NodeElemMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<Point>("point", "Point where NodeElem is to be located");
  params.addRequiredParam<SubdomainID>("subdomain_id", "Subdomain ID to assign to the new element");
  params.addRequiredParam<SubdomainName>("subdomain_name",
                                         "Subdomain name to assign to the new element");

  params.addClassDescription("Generates a NodeElem at a given point.");

  return params;
}

NodeElemMeshGenerator::NodeElemMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _point(getParam<Point>("point")),
    _subdomain_id(getParam<SubdomainID>("subdomain_id")),
    _subdomain_name(getParam<SubdomainName>("subdomain_name"))
{
}

std::unique_ptr<MeshBase>
NodeElemMeshGenerator::generate()
{
  auto mesh = buildMeshBaseObject(0);

  // add the Node
  const dof_id_type node_id = 0;
  Node * node = mesh->add_point(_point, node_id);

  // add the NodeElem
  const dof_id_type elem_id = 0;
  Elem * elem = new libMesh::NodeElem;
  elem->set_id(elem_id);
  elem->set_node(0, node);
  elem->subdomain_id() = _subdomain_id;
  mesh->add_elem(elem);

  // set the subdomain name
  mesh->subdomain_name(_subdomain_id) = _subdomain_name;

  mesh->set_isnt_prepared();

  return dynamic_pointer_cast<MeshBase>(mesh);
}
