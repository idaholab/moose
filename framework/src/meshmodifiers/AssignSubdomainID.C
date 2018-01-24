//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AssignSubdomainID.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<AssignSubdomainID>()
{
  InputParameters params = validParams<MeshModifier>();
  params.addRequiredParam<SubdomainID>("subdomain_id", "New subdomain IDs of all elements");
  return params;
}

AssignSubdomainID::AssignSubdomainID(const InputParameters & parameters)
  : MeshModifier(parameters), _subdomain_id(getParam<SubdomainID>("subdomain_id"))
{
}

void
AssignSubdomainID::modify()
{
  for (auto & elem : _mesh_ptr->getMesh().element_ptr_range())
    elem->subdomain_id() = _subdomain_id;
}
