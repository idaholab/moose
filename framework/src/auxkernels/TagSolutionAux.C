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
  InputParameters params = TagAuxBase<AuxKernel>::validParams();
  params.addClassDescription("Couple a tag solution vector, and return its dof value");
  params.addRequiredParam<TagName>("solution_tag",
                                   "Name of the solution tag this AuxKernel works on");
  // see #31357 and #20482
  params.suppressParameter<bool>("scaled");
  params.suppressParameter<bool>("remove_variable_scaling");
  return params;
}

TagSolutionAux::TagSolutionAux(const InputParameters & parameters)
  : TagAuxBase<AuxKernel>(parameters),
    _v(coupledVectorTagValue("v", "solution_tag")),
    _v_var(*getFieldVar("v", 0))
{
  checkCoupledVariable(&_v_var, &_var);
}

void
TagSolutionAux::initialSetup()
{
  TagAuxBase<AuxKernel>::initialSetup();

  const auto tag_id = _subproblem.getVectorTagID(getParam<TagName>("solution_tag"));
  const auto tag_type = _subproblem.vectorTagType(tag_id);
  if (tag_type == Moose::VECTOR_TAG_RESIDUAL)
    paramError("solution_tag",
               "The provided tag corresponds to a residual vector. Please use "
               "'TagResidualAux' instead.");
  if (tag_type != Moose::VECTOR_TAG_SOLUTION)
    paramError("solution_tag", "The provided tag does not correspond to a solution vector.");
}

Real
TagSolutionAux::computeValue()
{
  return _scaled ? _v[_qp] : _v[_qp] / _v_var.scalingFactor();
}
