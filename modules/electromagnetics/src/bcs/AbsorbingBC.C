#include "AbsorbingBC.h"
#include "ElkEnums.h"
#include "Function.h"
#include <complex>

registerMooseObject("ElkApp", AbsorbingBC);

InputParameters
AbsorbingBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription("First order Absorbing BC based on 'Theory and Computation of "
                             "Electromagnetic Fields' by JM Jin for scalar variables.");
  params.addRequiredCoupledVar("field_real", "Real component of field.");
  params.addRequiredCoupledVar("field_imaginary", "Imaginary component of field.");
  MooseEnum component("real imaginary");
  params.addParam<MooseEnum>("component", component, "Real or Imaginary wave component.");
  params.addParam<FunctionName>("func_real", 1.0, "Function coefficient, real component.");
  params.addParam<FunctionName>("func_imag", 0.0, "Function coefficient, imaginary component.");
  params.addParam<Real>("coeff_real", 1.0, "Constant coefficient, real component.");
  params.addParam<Real>("coeff_imag", 0.0, "Constant coefficient, real component.");
  params.addParam<Real>("sign", 1.0, "Sign of term in weak form.");
  return params;
}

AbsorbingBC::AbsorbingBC(const InputParameters & parameters)
  : IntegratedBC(parameters),

    _field_real(coupledValue("field_real")),
    _field_imag(coupledValue("field_imaginary")),
    _component(getParam<MooseEnum>("component")),
    _func_real(getFunction("func_real")),
    _func_imag(getFunction("func_imag")),
    _coeff_real(getParam<Real>("coeff_real")),
    _coeff_imag(getParam<Real>("coeff_imag")),
    _sign(getParam<Real>("sign"))
{
}

Real
AbsorbingBC::computeQpResidual()
{

  std::complex<double> field(_field_real[_qp], _field_imag[_qp]);
  std::complex<double> func(_func_real.value(_t, _q_point[_qp]),
                             _func_imag.value(_t, _q_point[_qp]));
  std::complex<double> coeff(_coeff_real, _coeff_imag);
  std::complex<double> jay(0, 1);

  std::complex<double> val = -jay * coeff * func * field;

  if (_component == elk::REAL)
  {
    return _sign * _test[_i][_qp] * val.real();
  }
  else
  {
    return _sign * _test[_i][_qp] * val.imag();
  }
}
