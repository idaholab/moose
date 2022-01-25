#include "ScaledGradientVector.h"

registerMooseObject("isopodApp", ScaledGradientVector);

InputParameters
ScaledGradientVector::validParams()
{
  InputParameters params = VectorAuxKernel::validParams();
  params.addRequiredCoupledVar("gradient_variable",
                               "The variable from which to compute the gradient");
  // MURHTY is scale something else, like a material property or anothe auxvariable?  This is easy
  // to change
  params.addParam<Real>("scale", 1.0, "scale factor for each gradient term");
  return params;
}

ScaledGradientVector::ScaledGradientVector(const InputParameters & parameters)
  : VectorAuxKernel(parameters),
    _var_grad(coupledGradient("gradient_variable")),
    _scale(getParam<Real>("scale"))
{
}

RealVectorValue
ScaledGradientVector::computeValue()
{
  return _scale * _var_grad[_qp];
}
