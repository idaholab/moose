//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElemUniqueSubdomains.h"
#include "MooseMesh.h"

#include "libmesh/elem.h"

registerMooseObject("MooseTestApp", ElemUniqueSubdomains);

template <>
InputParameters
validParams<ElemUniqueSubdomains>()
{
  auto p = validParams<MeshModifier>();
  p.addParam<subdomain_id_type>("mod",
                                std::numeric_limits<subdomain_id_type>::max(),
                                "elems are assigned their ids as block ids mod this");
  return p;
}

ElemUniqueSubdomains::ElemUniqueSubdomains(const InputParameters & parameters)
  : MeshModifier(parameters)
{
}

void
ElemUniqueSubdomains::modify()
{
  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling ElemUniqueSubdomains::modify()");

  auto mod = getParam<subdomain_id_type>("mod");

  MeshBase & mesh = _mesh_ptr->getMesh();
  for (auto elem : mesh.active_local_element_ptr_range())
    elem->subdomain_id() = elem->id() % mod;
}
