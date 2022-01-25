#include "ScaledGradientVector.h"

registerMooseObject("isopodApp", ScaledGradientVector);

InputParameters
ScaledGradientVector::validParams()
{
  InputParameters params = VectorAuxKernel::validParams();
  params.addRequiredCoupledVar("gradient_variable",
                               "The variable from which to compute the gradient");
  params.addParam<MaterialPropertyName>(
      "material_scaling", 1, "Material property to scale gradient variable");
  return params;
}

ScaledGradientVector::ScaledGradientVector(const InputParameters & parameters)
  : VectorAuxKernel(parameters),
    _var_grad(coupledGradient("gradient_variable")),
    _mat_scaling(getMaterialProperty<Real>("material_scaling"))
{
}

RealVectorValue
ScaledGradientVector::computeValue()
{
  return _mat_scaling[_qp] * _var_grad[_qp];
}
