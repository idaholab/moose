#include "RobinReflectionBC.h"
#include "Function.h"
#include <complex>

template <>
InputParameters
validParams<RobinReflectionBC>()
{
  InputParameters params = validParams<IntegratedBC>();

  params.addRequiredCoupledVar("coupled_variable", "Coupled variable.");
  MooseEnum component("imaginary real", "real");
  params.addParam<MooseEnum>("component", component, "Real or Imaginary wave component.");
  params.addRequiredParam<Real>("length", "Length of domain.");
  params.addParam<FunctionName>("func_real", 1.0, "Function coefficient, real component.");
  params.addParam<FunctionName>("func_imag", 0.0, "Function coefficient, imaginary component.");
  params.addParam<Real>("coeff_real", 1.0, "Constant coefficient, real component.");
  params.addParam<Real>("coeff_imag", 0.0, "Constant coefficient, real component.");
  return params;
}

RobinReflectionBC::RobinReflectionBC(const InputParameters & parameters)
  : IntegratedBC(parameters),

    _coupled_val(coupledValue("coupled_variable")),
    _component(getParam<MooseEnum>("component")),
    _L(getParam<Real>("length")),
    _func_real(getFunction("func_real")),
    _func_imag(getFunction("func_imag")),
    _coeff_real(getParam<Real>("coeff_real")),
    _coeff_imag(getParam<Real>("coeff_imag"))
{
}

Real
RobinReflectionBC::computeQpResidual()
{

  std::complex<double> _func(_func_real.value(_t, _q_point[_qp]),
                             _func_imag.value(_t, _q_point[_qp]));
  std::complex<double> _coeff(_coeff_real, _coeff_imag);
  std::complex<double> _j(0, 1);

  std::complex<double> _common = _j * _coeff * _func;
  std::complex<double> _RHS = 2.0 * _common * std::exp(_common * _L);

  if (_component == "real")
  {
    return -_test[_i][_qp] *
           (_RHS.real() - (_common.real() * _u[_qp] - _common.imag() * _coupled_val[_qp]));
  }
  else
  {
    return -_test[_i][_qp] *
           (_RHS.imag() - (_common.real() * _u[_qp] + _common.imag() * _coupled_val[_qp]));
  }
}
