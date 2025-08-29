//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReactionForceAux.h"

registerMooseObject("SolidMechanicsApp", ReactionForceAux);

InputParameters
ReactionForceAux::validParams()
{
  InputParameters params = TagResidualAux::validParams();
  params.addClassDescription(
      "Couple a tag residual vector, and return its dof value. Variable scaling is "
      "removed from the dof value.");

  // see #31357 and #20482
  params.set<bool>("scaled") = false;
  params.suppressParameter<bool>("scaled");

  return params;
}

ReactionForceAux::ReactionForceAux(const InputParameters & parameters) : TagResidualAux(parameters)
{
}
