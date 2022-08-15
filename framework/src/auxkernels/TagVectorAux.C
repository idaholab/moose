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
  if (_v_var.feType() != _var.feType())
    paramError("variable",
               "The AuxVariable this AuxKernel is acting on has to have the same order and family "
               "as the variable 'v'");
}

Real
TagVectorAux::computeValue()
{
  return _scaled ? _v[_qp] : _v[_qp] / _v_var.scalingFactor();
}
