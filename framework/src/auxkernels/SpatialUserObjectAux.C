//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SpatialUserObjectAux.h"
#include "UserObject.h"

registerMooseObject("MooseApp", SpatialUserObjectAux);

InputParameters
SpatialUserObjectAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Populates an auxiliary variable with a spatial value returned from a "
                             "UserObject spatialValue method.");
  params.addRequiredParam<UserObjectName>(
      "user_object",
      "The UserObject UserObject to get values from.  Note that the UserObject "
      "_must_ implement the spatialValue() virtual function!");
  return params;
}

SpatialUserObjectAux::SpatialUserObjectAux(const InputParameters & parameters)
  : AuxKernel(parameters), _user_object(getUserObjectBase("user_object"))
{
}

Real
SpatialUserObjectAux::computeValue()
{
  if (isNodal())
    return _user_object.spatialValue(*_current_node);
  else
    return _user_object.spatialValue(_current_elem->vertex_average());
}
