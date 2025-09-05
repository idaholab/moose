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
  params.addClassDescription("Couple a tagged vector, and return its DOF value");
  params.addRequiredParam<TagName>("vector_tag", "Name of the vector tag");
  params.addParam<bool>("remove_variable_scaling", false, "Remove variable scaling from DOF value");
  return params;
}

TagVectorAux::TagVectorAux(const InputParameters & parameters)
  : TagAuxBase<AuxKernel>(parameters),
    _unscaled(isParamSetByUser("scaled") ? !getParam<bool>("scaled")
                                         : getParam<bool>("remove_variable_scaling")),
    _v(coupledVectorTagValue("v", "vector_tag")),
    _v_var(*getFieldVar("v", 0))
{
  checkCoupledVariable(&_v_var, &_var);

  if (isParamSetByUser("scaled"))
    mooseDeprecated("The 'scaled' parameter has been deprecated. Please use the "
                    "'remove_variable_scaling' parameter instead.");
}

Real
TagVectorAux::computeValue()
{
  return _unscaled ? _v[_qp] / _v_var.scalingFactor() : _v[_qp];
}
