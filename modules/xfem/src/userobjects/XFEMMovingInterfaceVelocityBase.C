//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMMovingInterfaceVelocityBase.h"

InputParameters
XFEMMovingInterfaceVelocityBase::validParams()
{
  InputParameters params = DiscreteElementUserObject::validParams();
  params.addRequiredParam<UserObjectName>(
      "value_at_interface_uo",
      "The name of the userobject that obtains the value and gradient at the interface.");
  return params;
}

XFEMMovingInterfaceVelocityBase::XFEMMovingInterfaceVelocityBase(const InputParameters & parameters)
  : DiscreteElementUserObject(parameters)
{
}

void
XFEMMovingInterfaceVelocityBase::initialize()
{
  const UserObject * uo =
      &(_fe_problem.getUserObjectBase(getParam<UserObjectName>("value_at_interface_uo")));

  if (dynamic_cast<const NodeValueAtXFEMInterface *>(uo) == nullptr)
    mooseError("UserObject casting to NodeValueAtXFEMInterface in XFEMMovingInterfaceVelocity");

  _value_at_interface_uo = dynamic_cast<const NodeValueAtXFEMInterface *>(uo);
}
