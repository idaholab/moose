#include "RobinBC.h"
#include "ElectromagneticEnums.h"
#include "Function.h"
#include <complex>

registerMooseObject("ElectromagneticsApp", RobinBC);

InputParameters
RobinBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription(
      "First order Robin-style Absorbing/Port BC for scalar variables, assuming plane waves.");
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
  MooseEnum mode("absorbing port", "port");
  params.addParam<MooseEnum>("mode",
                             mode,
                             "Mode of operation for RobinBC. Can be set to 'absorbing' or 'port' "
                             "(default: 'port').");
  return params;
}

RobinBC::RobinBC(const InputParameters & parameters)
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
    _sign(getParam<Real>("sign")),
    _mode(getParam<MooseEnum>("mode"))
{
  bool profile_func_real_was_set = parameters.isParamSetByUser("profile_func_real");
  bool profile_func_imag_was_set = parameters.isParamSetByUser("profile_func_imag");

  if (_mode == electromagnetics::ABSORBING && (profile_func_real_was_set || profile_func_imag_was_set))
  {
    mooseError(
        "In ",
        _name,
        ", mode was set to Absorbing, while an incoming profile function (used for Port BCs) was "
        "defined. Either remove the profile function parameters, or set your BC to Port mode!");
  }
}

Real
RobinBC::computeQpResidual()
{

  std::complex<double> field(_field_real[_qp], _field_imag[_qp]);
  std::complex<double> func(_func_real.value(_t, _q_point[_qp]),
                            _func_imag.value(_t, _q_point[_qp]));
  std::complex<double> profile_func(_profile_func_real.value(_t, _q_point[_qp]),
                                    _profile_func_imag.value(_t, _q_point[_qp]));
  std::complex<double> coeff(_coeff_real, _coeff_imag);
  std::complex<double> jay(0, 1);

  std::complex<double> common = jay * coeff * func;
  std::complex<double> lhs = common * field;

  std::complex<double> rhs = 0.0;
  switch (_mode)
  {
    case electromagnetics::PORT:
      rhs = 2.0 * common * profile_func * std::exp(common * _q_point[_qp](0));
      break;
    case electromagnetics::ABSORBING:
      break;
  }

  std::complex<double> diff = rhs - lhs;
  Real res = 0.0;
  switch (_component)
  {
    case electromagnetics::REAL:
      res = _sign * _test[_i][_qp] * diff.real();
      break;
    case electromagnetics::IMAGINARY:
      res = _sign * _test[_i][_qp] * diff.imag();
      break;
  }
  return res;
}
