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
registerMooseObjectDeprecated("MooseApp", TagVectorAux, "10/01/2025 00:00");

InputParameters
TagResidualAux::validParams()
{
  InputParameters params = TagAuxBase<AuxKernel>::validParams();
  params.addClassDescription("Couple a tag residual vector, and return its dof value");
  params.addRequiredParam<TagName>("vector_tag", "Name of the vector tag this AuxKernel works on");
  return params;
}

TagResidualAux::TagResidualAux(const InputParameters & parameters)
  : TagAuxBase<AuxKernel>(parameters),
    _v(coupledVectorTagValue("v", "vector_tag")),
    _v_var(*getFieldVar("v", 0))
{
  checkCoupledVariable(&_v_var, &_var);
}

void
TagResidualAux::initialSetup()
{
  TagAuxBase<AuxKernel>::initialSetup();

  const auto vector_tag_id = _subproblem.getVectorTagID(getParam<TagName>("vector_tag"));
  const auto vector_tag_type = _subproblem.vectorTagType(vector_tag_id);
  if (vector_tag_type != Moose::VECTOR_TAG_RESIDUAL)
    paramError("vector_tag",
               "The provided vector tag does not correspond to a residual vector tag.");
}

Real
TagResidualAux::computeValue()
{
  return _scaled ? _v[_qp] : _v[_qp] / _v_var.scalingFactor();
}
