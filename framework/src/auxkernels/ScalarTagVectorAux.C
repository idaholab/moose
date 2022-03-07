//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarTagVectorAux.h"

registerMooseObject("MooseApp", ScalarTagVectorAux);

InputParameters
ScalarTagVectorAux::validParams()
{
  InputParameters params = TagAuxBase<AuxScalarKernel>::validParams();
  params.addParam<std::string>("vector_tag", "TagName", "Tag Name this Aux works on");
  params.addClassDescription("Couple a tag vector, and return its value");
  return params;
}

ScalarTagVectorAux::ScalarTagVectorAux(const InputParameters & parameters)
  : TagAuxBase<AuxScalarKernel>(parameters),
    _tag_id(_subproblem.getVectorTagID(getParam<std::string>("vector_tag"))),
    _v(coupledVectorTagScalarValue("v", _tag_id)),
    _v_var(*getScalarVar("v", 0))
{
}

Real
ScalarTagVectorAux::computeValue()
{
  return _scaled ? _v[0] : _v[0] / _v_var.scalingFactor();
}
