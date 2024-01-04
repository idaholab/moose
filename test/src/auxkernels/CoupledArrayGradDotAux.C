//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledArrayGradDotAux.h"
#include "MooseApp.h"

registerMooseObject("MooseTestApp", CoupledArrayGradDotAux);

InputParameters
CoupledArrayGradDotAux::validParams()
{
  InputParameters params = ArrayAuxKernel::validParams();
  params.addRequiredCoupledVar(
      "v", "The coupled array variable whose components are coupled to AuxArrayVariable");
  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "grad_component", component, "Gradient component of array variable's time derivative");
  params.addClassDescription(
      "Couple an array variable, and return a component of its gradient of time derivative");
  return params;
}

CoupledArrayGradDotAux::CoupledArrayGradDotAux(const InputParameters & parameters)
  : ArrayAuxKernel(parameters),
    _v(coupledArrayValue("v")),
    _v_grad_dot(coupledArrayGradientDot("v")),
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
CoupledArrayGradDotAux::computeValue()
{
  return _v_grad_dot[_qp].col(_grad_component);
}
