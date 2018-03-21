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

template <>
InputParameters
validParams<TagVectorAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addParam<std::string>("vector_tag", "TagName", "Tag Name this Aux works on");
  params.addRequiredCoupledVar("v",
                               "The coupled variable whose components are coupled to AuxVariable");

  params.addClassDescription("Couple a tag vector, and return its nodal value");
  return params;
}

TagVectorAux::TagVectorAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _tag_id(_subproblem.getVectorTagID(getParam<std::string>("vector_tag"))),
    _v(coupledVectorTagValue("v", _tag_id))
{
}

Real
TagVectorAux::computeValue()
{
  if (_var.isNodal())
  {
    return _v[_qp];
  }
  else
    mooseError("Require a nodal variable");
}
