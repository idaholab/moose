//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TagVectorArrayAux.h"

registerMooseObject("MooseApp", TagVectorArrayAux);

InputParameters
TagVectorArrayAux::validParams()
{
  InputParameters params = TagAuxBase<ArrayAuxKernel>::validParams();

  params.addRequiredParam<TagName>("vector_tag", "Tag Name this Aux works on");
  params.addClassDescription(
      "Couple a tagged vector, and return its evaluations at degree of freedom "
      "indices corresponding to the coupled array variable.");
  return params;
}

TagVectorArrayAux::TagVectorArrayAux(const InputParameters & parameters)
  : TagAuxBase<ArrayAuxKernel>(parameters), _v(coupledVectorTagArrayValue("v", "vector_tag"))
{
  if (getArrayVar("v", 0)->feType() != _var.feType())
    paramError("variable",
               "The AuxVariable this AuxKernel is acting on has to have the same order and family "
               "as the variable 'v'");
}

RealEigenVector
TagVectorArrayAux::computeValue()
{
  return _v[_qp];
}
