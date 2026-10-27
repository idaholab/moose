//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoefVectorTimeDerivative.h"

/**
 * This makes the kernel available in input files as:  type = CoefVectorTimeDerivative
 */
registerMooseObject("MooseApp", CoefVectorTimeDerivative);

InputParameters
CoefVectorTimeDerivative::validParams()
{
  InputParameters params = VectorTimeDerivative::validParams();
  params += NonADFunctorInterface::validParams();
  params.addClassDescription(
      "The time derivative operator for a vector variable multiplied by a scalar functor "
      "coefficient: $coef \\frac{\\partial \\vec{A}}{\\partial t}$.");
  params.addRequiredParam<MooseFunctorName>(
      "coef",
      "Scalar coefficient multiplying the vector time derivative (e.g. mu0 * sigma for the "
      "magnetoquasistatic EM equation). May be a constant, a Function, a functor material "
      "property, or a variable.");
  return params;
}

CoefVectorTimeDerivative::CoefVectorTimeDerivative(const InputParameters & parameters)
  : VectorTimeDerivative(parameters), NonADFunctorInterface(this), _coef(getFunctor<Real>("coef"))
{
}

Real
CoefVectorTimeDerivative::computeQpResidual()
{
  const Moose::ElemQpArg qp_arg = {_current_elem, _qp, _qrule, _q_point[_qp]};
  return _coef(qp_arg, determineState()) * VectorTimeDerivative::computeQpResidual();
}

Real
CoefVectorTimeDerivative::computeQpJacobian()
{
  const Moose::ElemQpArg qp_arg = {_current_elem, _qp, _qrule, _q_point[_qp]};
  return _coef(qp_arg, determineState()) * VectorTimeDerivative::computeQpJacobian();
}
