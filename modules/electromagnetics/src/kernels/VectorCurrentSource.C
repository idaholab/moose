#include "VectorCurrentSource.h"
#include "ElkEnums.h"
#include <complex>

registerMooseObject("ElkApp", VectorCurrentSource);

template <>
InputParameters
validParams<VectorCurrentSource>()
{
  InputParameters params = validParams<VectorKernel>();
  params.addClassDescription(
      "Kernel to calculate the RHS current source term in the helmholtz wave equation.");
  params.addParam<FunctionName>("function_coefficient", 1.0, "Function coefficient multiplier for current source (normally $\\omega$ or $\\omega \\cdot \\mu$).");
  params.addRequiredParam<FunctionName>("source_real", "Current Source vector, real component");
  params.addRequiredParam<FunctionName>("source_imag", "Current Source vector, imaginary component");
  MooseEnum component("real imaginary");
  params.addParam<MooseEnum>("component", component, "Component of field (real or imaginary).");
  return params;
}

VectorCurrentSource::VectorCurrentSource(const InputParameters & parameters)
  : VectorKernel(parameters),

    _func(getFunction("function_coefficient")),
    _source_real(getFunction("source_real")),
    _source_imag(getFunction("source_imag")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
VectorCurrentSource::computeQpResidual()
{
  std::complex<double> source_0(_source_real.vectorValue(_t, _q_point[_qp])(0), _source_imag.vectorValue(_t, _q_point[_qp])(0));
  std::complex<double> source_1(_source_real.vectorValue(_t, _q_point[_qp])(1), _source_imag.vectorValue(_t, _q_point[_qp])(1));
  std::complex<double> source_2(_source_real.vectorValue(_t, _q_point[_qp])(2), _source_imag.vectorValue(_t, _q_point[_qp])(2));
  VectorValue<std::complex<double>> source(source_0, source_1, source_2);

  std::complex<double> jay(0, 1);

  std::complex<double> res = jay * _func.value(_t, _q_point[_qp]) * source * _test[_i][_qp];

  if (_component == elk::REAL)
  {
    return res.real();
  }
  else
  {
    return res.imag();
  }
}

Real
VectorCurrentSource::computeQpJacobian()
{
  return 0.0;
}
