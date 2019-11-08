//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatPropUserObjectAux.h"
#include "MaterialPropertyUserObject.h"

registerMooseObject("MooseTestApp", MatPropUserObjectAux);

InputParameters
MatPropUserObjectAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<UserObjectName>(
      "material_user_object", "The MaterialPropertyUserObject to retrieve values from.");
  return params;
}

MatPropUserObjectAux::MatPropUserObjectAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _mat_uo(getUserObject<MaterialPropertyUserObject>("material_user_object"))
{
}

Real
MatPropUserObjectAux::computeValue()
{
  return _mat_uo.getElementalValue(_current_elem->id());
}
