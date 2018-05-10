#include "VectorCurrentSource.h"

registerMooseObject("ElkApp", VectorCurrentSource);

template <>
InputParameters
validParams<VectorCurrentSource>()
{
  InputParameters params = validParams<VectorKernel>();
  params.addClassDescription(
      "Kernel to calculate the RHS current source term in the helmholtz wave equation.");
  params.addParam<Real>("coeff", 1.0, "Coefficient multiplier for field.");
  params.addParam<FunctionName>("func", 1.0, "Function multiplier for field.");
  params.addParam<RealVectorValue>(
      "source_real", RealVectorValue(), "Current Source vector, real component.");
  params.addParam<RealVectorValue>(
      "source_imaginary", RealVectorValue(), "Current Source vector, imaginary component.");
  MooseEnum component("real imaginary");
  params.addParam<MooseEnum>("component", component, "Component of field (real or imaginary).");
  return params;
}

VectorCurrentSource::VectorCurrentSource(const InputParameters & parameters)
  : VectorKernel(parameters),

    _coefficient(getParam<Real>("coeff")),

    _func(getFunction("func")),

    _source_real(getParam<RealVectorValue>("source_real")),
    _source_imaginary(getParam<RealVectorValue>("source_imaginary")),

    _component(getParam<MooseEnum>("component"))
{
}

Real
VectorCurrentSource::computeQpResidual()
{
  if (_component == "real")
  {
    return -_coefficient * _func.value(_t, _q_point[_qp]) * _source_imaginary * _test[_i][_qp];
  }
  else
  {
    return _coefficient * _func.value(_t, _q_point[_qp]) * _source_real * _test[_i][_qp];
  }
}

Real
VectorCurrentSource::computeQpJacobian()
{
  return 0.0;
}
