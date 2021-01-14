//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GetElementalValueAux.h"
#include "AuxiliarySystem.h"

registerMooseObject("MooseTestApp", GetElementalValueAux);

InputParameters
GetElementalValueAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addClassDescription("Tests the getElementalValue() function of MooseVariableFE.");

  params.addRequiredCoupledVar("copied_variable", "Variable to be copied");

  MooseEnum time_level("current old older", "current", false);
  params.addRequiredParam<MooseEnum>("time_level", time_level, "Time level of the copied value");

  return params;
}

GetElementalValueAux::GetElementalValueAux(const InputParameters & params)
  : AuxKernel(params),
    _copied_var(*getVar("copied_variable", 0)),
    _time_level(getParam<MooseEnum>("time_level"))
{
  if (isNodal())
    mooseError("GetElementalValueAux cannot be used to compute a nodal variable");

  // These values may not be available by default, and if we ask for them within
  // an elemental loop, it will be far too late. Therefore - request them here.
  // This is not ideal... welcome to better suggestions!
  if (_time_level == "old")
    _copied_var.slnOld();
  else if (_time_level == "older")
    _copied_var.slnOlder();
}

Real
GetElementalValueAux::computeValue()
{
  if (_time_level == "current")
    return _copied_var.getElementalValue(_current_elem);
  else if (_time_level == "old")
    return _copied_var.getElementalValueOld(_current_elem);
  else if (_time_level == "older")
    return _copied_var.getElementalValueOlder(_current_elem);
  else
    mooseError("Invalid time level: ", _time_level);
}
