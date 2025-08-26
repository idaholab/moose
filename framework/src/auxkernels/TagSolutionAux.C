//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TagSolutionAux.h"

registerMooseObject("MooseApp", TagSolutionAux);

InputParameters
TagSolutionAux::validParams()
{
  InputParameters params = TagResidualAux::validParams();
  params.addClassDescription("Couple a tag solution vector, and return its dof value");

  // see #31357 and #20482
  params.set<bool>("scaled") = true;
  params.suppressParameter<bool>("scaled");

  return params;
}

TagSolutionAux::TagSolutionAux(const InputParameters & parameters) : TagResidualAux(parameters) {}

void
TagSolutionAux::initialSetup()
{
  TagAuxBase<AuxKernel>::initialSetup();

  const auto vector_tag_id = _subproblem.getVectorTagID(getParam<TagName>("vector_tag"));
  const auto vector_tag_type = _subproblem.vectorTagType(vector_tag_id);
  if (vector_tag_type != Moose::VECTOR_TAG_SOLUTION)
    paramError("vector_tag",
               "The provided vector tag does not correspond to a solution vector tag.");
}
