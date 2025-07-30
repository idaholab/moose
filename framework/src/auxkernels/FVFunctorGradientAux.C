//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFunctorGradientAux.h"
#include "Assembly.h"

registerMooseObject("SubChannelApp", FVFunctorGradientAux);

InputParameters
FVFunctorGradientAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  MooseEnum component("x y z normal");
  params.addClassDescription("Compute components of a finite volume variable "
                             "$(c \\nabla \phi)$.");
  params.addRequiredParam<MooseEnum>("component", component, "The desired component of flux.");
  params.addRequiredCoupledVar("gradient_variable", "The name of the variable which is the operand for the gradient operator which is the operand for the gradient operator.");
  params.addParam<Real>("scaling_factor", 1.0, "The scaling factor");
  return params;
}

FVFunctorGradientAux::FVFunctorGradientAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _use_normal(getParam<MooseEnum>("component") == "normal"),
    _component(getParam<MooseEnum>("component")),
    _gradient_var(dynamic_cast<const MooseVariableFVReal *>(getFieldVar("gradient_variable", 0))),
    _scaling_factor(getParam<Real>("scaling_factor")),
    _normals(_assembly.normals())
{
  if (_use_normal && !isParamValid("boundary"))
    paramError("boundary", "A boundary must be provided if using the normal component!");
}

Real
FVFunctorGradientAux::computeValue()
{
  const auto _vec_grad = raw_value(_gradient_var->gradSln(_current_elem, determineState()));
  const Real gradient = _use_normal ? _vec_grad * _normals[_qp] : _vec_grad(_component);
  return -scaling_factor * gradient;
}
