//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TagResidualAux.h"

registerMooseObject("MooseApp", TagResidualAux);
registerMooseObjectRenamed("MooseApp", TagVectorAux, "10/01/2025 00:00", TagResidualAux);

InputParameters
TagResidualAux::validParams()
{
  InputParameters params = TagAuxBase<AuxKernel>::validParams();
  params.addClassDescription("Couple a tag residual vector, and return its dof value");
  params.addDeprecatedParam<TagName>("vector_tag",
                                     "Name of the vector tag this AuxKernel works on",
                                     "This parameter has been renamed to 'residual_tag'");
  params.addRequiredParam<TagName>("residual_tag",
                                   "Name of the residual tag this AuxKernel works on");
  return params;
}

TagResidualAux::TagResidualAux(const InputParameters & parameters)
  : TagAuxBase<AuxKernel>(parameters),
    _v(coupledVectorTagValue("v", "residual_tag")),
    _v_var(*getFieldVar("v", 0))
{
  checkCoupledVariable(&_v_var, &_var);
}

void
TagResidualAux::initialSetup()
{
  TagAuxBase<AuxKernel>::initialSetup();

  const auto tag_param_name = isParamValid("vector_tag") ? "vector_tag" : "residual_tag";
  const auto tag_id = _subproblem.getVectorTagID(getParam<TagName>(tag_param_name));
  const auto tag_type = _subproblem.vectorTagType(tag_id);
  if (tag_type == Moose::VECTOR_TAG_SOLUTION)
    paramError(tag_param_name,
               "The provided tag corresponds to a solution vector. Please use "
               "'TagSolutionAux' instead.");
  if (tag_type != Moose::VECTOR_TAG_RESIDUAL)
    paramError(tag_param_name, "The provided tag does not correspond to a residual vector.");
}

Real
TagResidualAux::computeValue()
{
  return _scaled ? _v[_qp] : _v[_qp] / _v_var.scalingFactor();
}
