//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementUOAux.h"
#include "ElementUOProvider.h"

registerMooseObject("MooseApp", ElementUOAux);

InputParameters
ElementUOAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<UserObjectName>("element_user_object",
                                          "The ElementUOProvider where this Aux pulls values from");

  params.addParam<std::string>("field_name",
                               "The field name to retrieve from the ElementUOProvider");

  MooseEnum field_type("long Real", "long");
  params.addParam<MooseEnum>("field_type", field_type, "The type of field to retrieve");

  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;
  params.addClassDescription("Aux Kernel to display generic spatial (elemental) information from a "
                             "UserObject that satisfies the underlying ElementUOProvider "
                             "interface.");
  return params;
}

ElementUOAux::ElementUOAux(const InputParameters & params)
  : AuxKernel(params),
    // doco-get-user-object-begin
    _elem_uo(getUserObject<ElementUOProvider>("element_user_object")),
    // doco-get-user-object-end
    _field_name(isParamValid("field_name") ? getParam<std::string>("field_name") : "default"),
    _field_type(getParam<MooseEnum>("field_type"))
{
  if (isNodal())
    mooseError("This AuxKernel only supports Elemental fields");
}

Real
ElementUOAux::computeValue()
{
  if (_field_type == "long")
  {
    auto value = _elem_uo.getElementalValueLong(_current_elem->id(), _field_name);
    if (value == std::numeric_limits<unsigned long>::max())
      return -1.0;
    else
      return value;
  }
  else
    return _elem_uo.getElementalValueReal(_current_elem->id(), _field_name);
}
