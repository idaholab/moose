/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
