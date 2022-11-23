//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TagVectorArrayVariableGradientAux.h"

registerMooseObject("MooseTestApp", TagVectorArrayVariableGradientAux);

InputParameters
TagVectorArrayVariableGradientAux::validParams()
{
  InputParameters params = TagAuxBase<ArrayAuxKernel>::validParams();

  params.addRequiredParam<TagName>("vector_tag", "Tag Name this Aux works on");
  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "grad_component", component, "Gradient component of tagged vector.");
  params.addClassDescription("Couple a tagged vector, and return its array gradient component.");
  return params;
}

TagVectorArrayVariableGradientAux::TagVectorArrayVariableGradientAux(
    const InputParameters & parameters)
  : TagAuxBase<ArrayAuxKernel>(parameters),
    _v(coupledVectorTagArrayValue("v", "vector_tag")),
    _v_grad(coupledVectorTagArrayGradient("v", "vector_tag")),
    _grad_component(getParam<MooseEnum>("grad_component"))
{
  if (getArrayVar("v", 0)->feType() != _var.feType())
    paramError("variable",
               "The AuxVariable this AuxKernel is acting on has to have the same order and family "
               "as the variable 'v'");
  if (getArrayVar("v", 0)->count() != _var.count())
    paramError("variable",
               "The AuxVariable this AuxKernel is acting on has to have the same number of "
               "components as the variable 'v'");
}

RealEigenVector
TagVectorArrayVariableGradientAux::computeValue()
{
  return _v_grad[_qp].col(_grad_component);
}
