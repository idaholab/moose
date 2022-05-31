//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EMRobinBC.h"
#include "ElectromagneticEnums.h"
#include "ElectromagneticConstants.h"
#include "Function.h"
#include <complex>

registerMooseObject("ElectromagneticsApp", EMRobinBC);

InputParameters
EMRobinBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
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
  MooseEnum sign("positive=1 negative=-1", "positive");
  params.addParam<MooseEnum>("sign", sign, "Sign of boundary term in weak form.");
  MooseEnum mode("absorbing port", "port");
  params.addParam<MooseEnum>("mode",
                             mode,
                             "Mode of operation for EMRobinBC. Can be set to 'absorbing' or 'port' "
                             "(default: 'port').");
  return params;
}

EMRobinBC::EMRobinBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _field_real(adCoupledValue("field_real")),
    _field_imag(adCoupledValue("field_imaginary")),
    _component(getParam<MooseEnum>("component")),
    _func_real(getFunction("func_real")),
    _func_imag(getFunction("func_imag")),
    _profile_func_real(getFunction("profile_func_real")),
    _profile_func_imag(getFunction("profile_func_imag")),
    _coeff_real(getParam<Real>("coeff_real")),
    _coeff_imag(getParam<Real>("coeff_imag")),
    _sign(getParam<MooseEnum>("sign")),
    _mode(getParam<MooseEnum>("mode"))
{
  bool profile_func_real_was_set = parameters.isParamSetByUser("profile_func_real");
  bool profile_func_imag_was_set = parameters.isParamSetByUser("profile_func_imag");

  if (_mode == EM::ABSORBING && (profile_func_real_was_set || profile_func_imag_was_set))
    mooseError(
        "In ",
        _name,
        ", mode was set to Absorbing, while an incoming profile function (used for Port BCs) was "
        "defined. Either remove the profile function parameters, or set your BC to Port mode!");
}

ADReal
EMRobinBC::computeQpResidual()
{
  std::complex<double> func(_func_real.value(_t, _q_point[_qp]),
                            _func_imag.value(_t, _q_point[_qp]));
  std::complex<double> profile_func(_profile_func_real.value(_t, _q_point[_qp]),
                                    _profile_func_imag.value(_t, _q_point[_qp]));
  std::complex<double> coeff(_coeff_real, _coeff_imag);

  std::complex<double> common = EM::j * coeff * func;
  ADReal lhs_real = common.real() * _field_real[_qp] - common.imag() * _field_imag[_qp];
  ADReal lhs_imag = common.real() * _field_imag[_qp] + common.imag() * _field_real[_qp];

  std::complex<double> rhs = 0.0;
  switch (_mode)
  {
    case EM::PORT:
      rhs = 2.0 * common * profile_func * std::exp(common * _q_point[_qp](0));
      break;
    case EM::ABSORBING:
      break;
  }

  ADReal diff_real = rhs.real() - lhs_real;
  ADReal diff_imag = rhs.imag() - lhs_imag;

  ADReal res = 0.0;
  switch (_component)
  {
    case EM::REAL:
      res = _sign * _test[_i][_qp] * diff_real;
      break;
    case EM::IMAGINARY:
      res = _sign * _test[_i][_qp] * diff_imag;
      break;
  }
  return res;
}
