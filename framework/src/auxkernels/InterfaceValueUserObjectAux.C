//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceValueUserObjectAux.h"

registerMooseObject("MooseApp", InterfaceValueUserObjectAux);

InputParameters
InterfaceValueUserObjectAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<UserObjectName>("interface_uo_name",
                                          "The name of the interface user object to use");
  params.addParam<bool>("return_side_average",
                        false,
                        "If true returns the elment side average rather than a single qp value");
  params.addClassDescription("Get stored value from the specified InterfaceQpUserObjectBase.");

  return params;
}

InterfaceValueUserObjectAux::InterfaceValueUserObjectAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _interface_uo(getUserObject<InterfaceQpUserObjectBase>("interface_uo_name")),
    _return_side_average(getParam<bool>("return_side_average"))
{
  if (isNodal())
    paramError("variable", "This AuxKernel only supports Elemental fields");
}

Real
InterfaceValueUserObjectAux::computeValue()
{
  if (_return_side_average)
    return _interface_uo.getSideAverageValue(_current_elem->id(), _current_side);
  else
    return _interface_uo.getQpValue(_current_elem->id(), _current_side, _qp);
}
