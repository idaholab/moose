#include "PortBC.h"
#include "ElkEnums.h"
#include "Function.h"
#include <complex>

registerMooseObject("ElkApp", PortBC);

template <>
InputParameters
validParams<PortBC>()
{
  InputParameters params = validParams<IntegratedBC>();

  params.addRequiredCoupledVar("field_real", "Real component of field.");
  params.addRequiredCoupledVar("field_imaginary", "Imaginary component of field.");
  MooseEnum component("real imaginary");
  params.addParam<MooseEnum>("component", component, "Real or Imaginary wave component.");
  params.addParam<FunctionName>("func_real", 1.0, "Function coefficient, real component.");
  params.addParam<FunctionName>("func_imag", 0.0, "Function coefficient, imaginary component.");
  params.addParam<FunctionName>("profile_func_real", 1.0, "Function coefficient, real component.");
  params.addParam<FunctionName>(
      "profile_func_imag", 0.0, "Function coefficient, imaginary component.");
  params.addParam<Real>("coeff_real", 1.0, "Constant coefficient, real component.");
  params.addParam<Real>("coeff_imag", 0.0, "Constant coefficient, real component.");
  params.addParam<Real>("sign", 1.0, "Sign of term in weak form.");
  return params;
}

PortBC::PortBC(const InputParameters & parameters)
  : IntegratedBC(parameters),

    _field_real(coupledValue("field_real")),
    _field_imag(coupledValue("field_imaginary")),
    _component(getParam<MooseEnum>("component")),
    _func_real(getFunction("func_real")),
    _func_imag(getFunction("func_imag")),
    _profile_func_real(getFunction("profile_func_real")),
    _profile_func_imag(getFunction("profile_func_imag")),
    _coeff_real(getParam<Real>("coeff_real")),
    _coeff_imag(getParam<Real>("coeff_imag")),
    _sign(getParam<Real>("sign"))
{
}

Real
PortBC::computeQpResidual()
{

  std::complex<double> field(_field_real[_qp], _field_imag[_qp]);
  std::complex<double> func(_func_real.value(_t, _q_point[_qp]),
                             _func_imag.value(_t, _q_point[_qp]));
  std::complex<double> profile_func(_profile_func_real.value(_t, _q_point[_qp]),
                                     _profile_func_imag.value(_t, _q_point[_qp]));
  std::complex<double> coeff(_coeff_real, _coeff_imag);
  std::complex<double> jay(0, 1);

  std::complex<double> common = jay * coeff * func;
  std::complex<double> rhs = 2.0 * common * profile_func * std::exp(common * _q_point[_qp](0));
  std::complex<double> lhs = common * field;
  std::complex<double> diff = rhs - lhs;

  if (_component == elk::REAL)
  {
    return _sign * _test[_i][_qp] * diff.real();
  }
  else
  {
    return _sign * _test[_i][_qp] * diff.imag();
  }
}
