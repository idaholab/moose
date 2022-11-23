//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TagVectorArrayVariableValueAux.h"

registerMooseObject("MooseApp", TagVectorArrayVariableValueAux);

InputParameters
TagVectorArrayVariableValueAux::validParams()
{
  InputParameters params = TagAuxBase<ArrayAuxKernel>::validParams();

  params.addRequiredParam<TagName>("vector_tag", "Tag Name this Aux works on");
  params.addClassDescription("Couple a tagged vector, and return its array value.");
  return params;
}

TagVectorArrayVariableValueAux::TagVectorArrayVariableValueAux(const InputParameters & parameters)
  : TagAuxBase<ArrayAuxKernel>(parameters), _v(coupledVectorTagArrayValue("v", "vector_tag"))
{
  checkCoupledVariable(getArrayVar("v", 0), &_var);
}

RealEigenVector
TagVectorArrayVariableValueAux::computeValue()
{
  return _v[_qp];
}
