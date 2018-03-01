#include "ReflectionCoefficient.h"
#include "Function.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ReflectionCoefficient>()
{
  InputParameters params = validParams<SidePostprocessor>();
  params.addClassDescription(
      "CURRENTLY ONLY FOR 1D SOLVES. Calculate reflection coefficient for impinging wave on a "
      "surface. Assumes that wave of form F = F_incoming + R*F_reflected");
  params.addRequiredCoupledVar(
      "variable", "The name of the real field variable this postprocessor operates on.");
  params.addRequiredParam<FunctionName>("incoming_real", "Incoming wave function, real component.");
  params.addRequiredParam<FunctionName>("incoming_imag",
                                        "Incoming wave function, imaginary component.");
  params.addRequiredParam<FunctionName>("reflected_real",
                                        "Reflected general waveform, real component.");
  params.addRequiredParam<FunctionName>("reflected_imag",
                                        "Reflected general waveform, imaginary component.");
  params.addRequiredCoupledVar("coupled_imag", "Coupled imaginary field variable.");
  return params;
}

ReflectionCoefficient::ReflectionCoefficient(const InputParameters & parameters)
  : SidePostprocessor(parameters),

    MooseVariableInterface(this, false),
    _u(coupledValue("variable")),

    _qp(0),

    _incoming_real(getFunction("incoming_real")),
    _incoming_imag(getFunction("incoming_imag")),

    _reflected_real(getFunction("reflected_real")),
    _reflected_imag(getFunction("reflected_imag")),

    _coupled_imag(coupledValue("coupled_imag"))
{
  addMooseVariableDependency(mooseVariable());
}

void
ReflectionCoefficient::initialize()
{
  _R = 0;
}

void
ReflectionCoefficient::execute()
{
  _R = computeReflection();
}

PostprocessorValue
ReflectionCoefficient::getValue()
{
  return _R;
}

void
ReflectionCoefficient::threadJoin(const UserObject & y)
{
  const ReflectionCoefficient & pps = static_cast<const ReflectionCoefficient &>(y);
  _R = pps._R;
}

Real
ReflectionCoefficient::computeReflection()
{
  Real _ref = 0;
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {

    Real _real_diff = _u[_qp] - _incoming_real.value(_t, _q_point[_qp]);
    Real _imag_diff = _coupled_imag[_qp] - _incoming_imag.value(_t, _q_point[_qp]);

    Real _real_numerator = _reflected_real.value(_t, _q_point[_qp]) * _real_diff +
                           _reflected_imag.value(_t, _q_point[_qp]) * _imag_diff;
    Real _imag_numerator = _reflected_real.value(_t, _q_point[_qp]) * _imag_diff -
                           _reflected_imag.value(_t, _q_point[_qp]) * _real_diff;
    Real _denominator = std::pow(_reflected_real.value(_t, _q_point[_qp]), 2) +
                        std::pow(_reflected_imag.value(_t, _q_point[_qp]), 2);

    _ref = std::sqrt(std::pow(_real_numerator / _denominator, 2) +
                     std::pow(_imag_numerator / _denominator, 2));
  }
  return _ref;
}
