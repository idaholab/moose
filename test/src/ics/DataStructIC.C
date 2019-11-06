//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DataStructIC.h"

registerMooseObject("MooseTestApp", DataStructIC);

InputParameters
DataStructIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  return params;
}

DataStructIC::DataStructIC(const InputParameters & parameters)
  : InitialCondition(parameters), _mesh(_fe_problem.mesh())
{
}

DataStructIC::~DataStructIC() {}

void
DataStructIC::initialSetup()
{
  MeshBase::const_element_iterator elem_end = _mesh.activeLocalElementsEnd();
  for (MeshBase::const_element_iterator elem_it = _mesh.activeLocalElementsBegin();
       elem_it != elem_end;
       ++elem_it)
  {
    const Elem * current_elem = *elem_it;

    unsigned int n_nodes = current_elem->n_vertices();
    for (unsigned int i = 0; i < n_nodes; ++i)
    {
      const Node * current_node = current_elem->node_ptr(i);

      _data[current_node->id()] = current_node->id() * 2.0; // double the node_id
    }
  }
}

Real
DataStructIC::value(const Point & /*p*/)
{
  if (_current_node == NULL)
    return -1.0;

  // Make sure the id is in our data structure
  std::map<dof_id_type, Real>::const_iterator it = _data.find(_current_node->id());

  if (it != _data.end())
    return it->second;

  mooseError("The following id is not in the data structure: ", _current_node->id());
}
