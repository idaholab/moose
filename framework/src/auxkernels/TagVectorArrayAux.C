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
  InputParameters params = AuxKernel::validParams();

  params.addRequiredParam<TagName>("vector_tag", "Tag Name this Aux works on");
  params.addRequiredCoupledVar("v",
                               "The coupled variable whose components are coupled to AuxVariable");
  params.set<ExecFlagEnum>("execute_on", true) = {EXEC_TIMESTEP_END};
  params.addRequiredParam<unsigned int>(
      "component",
      "What component to index the variable at. This is a relevant "
      "parameter in the context of array variables.");

  params.addClassDescription("Couple a tag vector, and return its nodal value");
  return params;
}

TagVectorArrayAux::TagVectorArrayAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _v(coupledVectorTagArrayValue("v", "vector_tag")),
    _component(getParam<unsigned int>("component"))
{
  if (getArrayVar("v", 0)->feType() != _var.feType())
    paramError("variable",
               "The AuxVariable this AuxKernel is acting on has to have the same order and family "
               "as the variable 'v'");
}

Real
TagVectorArrayAux::computeValue()
{
  return _v[_qp](_component);
}
