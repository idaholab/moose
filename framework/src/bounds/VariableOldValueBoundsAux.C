//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariableOldValueBoundsAux.h"

registerMooseObject("MooseApp", VariableOldValueBoundsAux);

InputParameters
VariableOldValueBoundsAux::validParams()
{
  InputParameters params = BoundsAuxBase::validParams();
  params.addClassDescription("Uses the old variable values as the bounds for the new solve.");
  return params;
}

VariableOldValueBoundsAux::VariableOldValueBoundsAux(const InputParameters & parameters)
  : BoundsAuxBase(parameters)
{
}

Real
VariableOldValueBoundsAux::getBound()
{
  if (_fe_var && isNodal())
    return _fe_var->getNodalValueOld(*_current_node);
  else
    mooseError("This variable type is not supported yet");
}
