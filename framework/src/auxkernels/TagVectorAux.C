//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  params.addRequiredParam<TagName>("vector_tag", "Tag Name this Aux works on");
  params.addClassDescription("Couple a tag vector, and return its nodal value");
  return params;
}

TagVectorAux::TagVectorAux(const InputParameters & parameters)
  : TagAuxBase<AuxKernel>(parameters),
    _v(coupledVectorTagValue("v", "vector_tag")),
    _v_var(*getFieldVar("v", 0))
{
  checkCoupledVariable(&_v_var, &_var);
}

Real
TagVectorAux::computeValue()
{
  return _scaled ? _v[_qp] : _v[_qp] / _v_var.scalingFactor();
}
