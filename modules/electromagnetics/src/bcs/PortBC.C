#include "PortBC.h"
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
  MooseEnum component("imaginary real", "real");
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

  std::complex<double> _field(_field_real[_qp], _field_imag[_qp]);
  std::complex<double> _func(_func_real.value(_t, _q_point[_qp]),
                             _func_imag.value(_t, _q_point[_qp]));
  std::complex<double> _profile_func(_profile_func_real.value(_t, _q_point[_qp]),
                                     _profile_func_imag.value(_t, _q_point[_qp]));
  std::complex<double> _coeff(_coeff_real, _coeff_imag);
  std::complex<double> _j(0, 1);

  std::complex<double> _common = _j * _coeff * _func;
  std::complex<double> _RHS = 2.0 * _common * _profile_func * std::exp(_common * _q_point[_qp](0));
  std::complex<double> _LHS = _common * _field;
  std::complex<double> _diff = _RHS - _LHS;

  if (_component == "real")
  {
    return _sign * _test[_i][_qp] * _diff.real();
  }
  else
  {
    return _sign * _test[_i][_qp] * _diff.imag();
  }
}
