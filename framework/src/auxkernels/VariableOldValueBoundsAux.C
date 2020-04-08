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

defineLegacyParams(VariableOldValueBoundsAux);

InputParameters
VariableOldValueBoundsAux::validParams()
{
  InputParameters params = BoundsAuxBase::validParams();
  params.addClassDescription("Provides the upper and lower bound of the phase field fracture "
                             "variable to PETSc's SNES variational inequalities solver.");
  return params;
}

VariableOldValueBoundsAux::VariableOldValueBoundsAux(const InputParameters & parameters)
  : BoundsAuxBase(parameters)
{
}

Real
VariableOldValueBoundsAux::getBound()
{
  return _var.getNodalValueOld(*_current_node);
}
