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
  InputParameters params = TagVectorAux::validParams();
  params.addClassDescription("Extract the value of the residual from an appropriately formed tag "
                             "vector and save those values as reaction forces in an AuxVariable");

  params.set<bool>("remove_variable_scaling") = true;
  params.suppressParameter<bool>("remove_variable_scaling");
  params.suppressParameter<bool>("scaled");
  return params;
}

ReactionForceAux::ReactionForceAux(const InputParameters & parameters) : TagVectorAux(parameters)
{
  const auto tag_id = _subproblem.getVectorTagID(getParam<TagName>("vector_tag"));
  const auto tag_type = _subproblem.vectorTagType(tag_id);
  if (tag_type != Moose::VECTOR_TAG_RESIDUAL)
    paramError("vector_tag", "The provided tag does not correspond to a residual vector.");
}
