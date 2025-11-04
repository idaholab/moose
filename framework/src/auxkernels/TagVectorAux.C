//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TagVectorAux.h"

registerMooseObject("MooseApp", TagVectorAux);

InputParameters
TagVectorAux::validParams()
{
  InputParameters params = TagAuxBase<AuxKernel>::validParams();
  params.addClassDescription("Extract DOF values from a tagged vector into an AuxVariable");
  params.addRequiredParam<TagName>("vector_tag", "Name of the vector tag to extract values from");
  params.addParam<bool>(
      "remove_variable_scaling",
      false,
      "Whether to remove variable scaling from DOF value. If false, values are directly extracted "
      "from the tag vector, and potentially with scaling applied. If true, any scaling of "
      "variables is undone in the reported values.");
  return params;
}

TagVectorAux::TagVectorAux(const InputParameters & parameters)
  : TagAuxBase<AuxKernel>(parameters),
    _remove_variable_scaling(isParamSetByUser("scaled")
                                 ? !getParam<bool>("scaled")
                                 : getParam<bool>("remove_variable_scaling")),
    _v(coupledVectorTagValue("v", "vector_tag")),
    _v_var(*getFieldVar("v", 0))
{
  checkCoupledVariable(&_v_var, &_var);

  if (isParamSetByUser("scaled"))
  {
    mooseDeprecated("The 'scaled' parameter has been deprecated. Please use the "
                    "'remove_variable_scaling' parameter instead.");
    if (isParamSetByUser("remove_variable_scaling"))
      paramError("You cannot set both the 'scaled' and 'remove_variable_scaling' parameters. "
                 "Please use only the 'remove_variable_scaling' parameter.");
  }

  if (_remove_variable_scaling)
  {
    const auto vector_tag_id = _subproblem.getVectorTagID(getParam<TagName>("vector_tag"));
    const auto vector_tag_type = _subproblem.vectorTagType(vector_tag_id);
    if (vector_tag_type != Moose::VECTOR_TAG_RESIDUAL)
      paramError("vector_tag",
                 "The provided vector tag does not correspond to a tagged residual vector, which "
                 "is the only kind of vector tag type for which scaling is applicable, yet "
                 "variable scaling is requested to be removed.");
  }
}

Real
TagVectorAux::computeValue()
{
  return _remove_variable_scaling ? _v[_qp] / _v_var.scalingFactor() : _v[_qp];
}
