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
  params.addRequiredParam<FunctionName>("x_real", "x component");
  params.addRequiredParam<FunctionName>("y_real", "y component");
  params.addRequiredParam<FunctionName>("z_real", "z component");
  params.addRequiredParam<FunctionName>("x_imag", "x component");
  params.addRequiredParam<FunctionName>("y_imag", "y component");
  params.addRequiredParam<FunctionName>("z_imag", "z component");
  // params.addParam<RealVectorValue>(
  //     "source_real", RealVectorValue(), "Current Source vector, real component.");
  // params.addParam<RealVectorValue>(
  //     "source_imaginary", RealVectorValue(), "Current Source vector, imaginary component.");
  MooseEnum component("real imaginary");
  params.addParam<MooseEnum>("component", component, "Component of field (real or imaginary).");
  return params;
}

VectorCurrentSource::VectorCurrentSource(const InputParameters & parameters)
  : VectorKernel(parameters),

    _coefficient(getParam<Real>("coeff")),

    _func(getFunction("func")),

    _x_real(getFunction("x_real")),
    _y_real(getFunction("y_real")),
    _z_real(getFunction("z_real")),

    _x_imag(getFunction("x_imag")),
    _y_imag(getFunction("y_imag")),
    _z_imag(getFunction("z_imag")),

    // _source_real(getParam<RealVectorValue>("source_real")),
    // _source_imaginary(getParam<RealVectorValue>("source_imaginary")),

    _component(getParam<MooseEnum>("component"))
{
}

Real
VectorCurrentSource::computeQpResidual()
{
  RealVectorValue _source_real(_x_real.value(_t, _q_point[_qp]),
                               _y_real.value(_t, _q_point[_qp]),
                               _z_real.value(_t, _q_point[_qp]));
  RealVectorValue _source_imaginary(_x_imag.value(_t, _q_point[_qp]),
                                    _y_imag.value(_t, _q_point[_qp]),
                                    _z_imag.value(_t, _q_point[_qp]));

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
