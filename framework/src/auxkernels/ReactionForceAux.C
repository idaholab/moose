//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReactionForceAux.h"

registerMooseObject("MooseApp", ReactionForceAux);

InputParameters
ReactionForceAux::validParams()
{
  InputParameters params = TagVectorAux::validParams();
  params.addClassDescription("Couple a tag vector, and return its dof value. Variable scaling is removed from the dof value.");
  params.addRequiredParam<TagName>("vector_tag", "Tag Name this AuxKernel works on");
  params.suppressParameter<bool>("scaled");
  return params;
}

ReactionForceAux::ReactionForceAux(const InputParameters & parameters) : TagVectorAux(parameters) {}

Real
ReactionForceAux::computeValue()
{
  return _v[_qp] / _v_var.scalingFactor();
}
