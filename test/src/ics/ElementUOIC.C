//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementUOIC.h"
#include "ElementUOProvider.h"

registerMooseObject("MooseTestApp", ElementUOIC);

InputParameters
ElementUOIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredParam<UserObjectName>("element_user_object",
                                          "The ElementUOProvider to be coupled into this IC");

  params.addParam<std::string>("field_name",
                               "The field name to retrieve from the ElementUOProvider");

  MooseEnum field_type("long Real", "long");
  params.addParam<MooseEnum>("field_type", field_type, "The type of field to retrieve");

  return params;
}

ElementUOIC::ElementUOIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _mesh(_fe_problem.mesh()),
    _elem_uo(getUserObject<ElementUOProvider>("element_user_object")),
    _field_name(isParamValid("field_name") ? getParam<std::string>("field_name") : "default"),
    _field_type(getParam<MooseEnum>("field_type"))
{
}

Real
ElementUOIC::value(const Point & /*p*/)
{
  mooseAssert(_current_elem, "Current Elem is nullptr");

  if (_field_type == "long")
    return _elem_uo.getElementalValueLong(_current_elem->id(), _field_name);
  else
    return _elem_uo.getElementalValueReal(_current_elem->id(), _field_name);
}
