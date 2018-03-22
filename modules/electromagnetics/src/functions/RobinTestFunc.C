#include "RobinTestFunc.h"
#include <complex>

template <>
InputParameters
validParams<RobinTestFunc>()
{
  InputParameters params = validParams<Function>();
  params.addClassDescription("Function of analytical solution for use in convergence testing with "
                             "coupled_helmholtz test file.");
  params.addRequiredParam<Real>("L", "Length of 1D test domain, where 0 < x < L");
  params.addRequiredParam<Real>(
      "a", "Real component of squared coefficient c in the PDE, u'' + c^2 * u = 0");
  params.addRequiredParam<Real>(
      "b", "Imaginary component of squared coefficient c in the PDE, u'' + c^2 * u = 0");
  params.addRequiredParam<Real>("g0_real", "Real component of DirichletBC where x = 0.");
  params.addRequiredParam<Real>("g0_imag", "Imaginary component of DirichletBC where x = 0.");
  params.addParam<FunctionName>("func", "Function coefficient.");
  MooseEnum component("imaginary real", "real");
  params.addParam<MooseEnum>("component", component, "Real or Imaginary wave component.");
  return params;
}

RobinTestFunc::RobinTestFunc(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),

    _L(getParam<Real>("L")),
    _a(getParam<Real>("a")),
    _b(getParam<Real>("b")),
    _g0_real(getParam<Real>("g0_real")),
    _g0_imag(getParam<Real>("g0_imag")),
    _func(getFunction("func")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
RobinTestFunc::value(Real t, const Point & p)
{
  std::complex<double> _C1(0, 0);
  std::complex<double> _C2(0, 0);
  std::complex<double> _lambda(0, 0);
  std::complex<double> _lambda_L(0, 0);
  std::complex<double> val(0, 0);

  std::complex<double> _g0(_g0_real, _g0_imag);
  std::complex<double> _c(_a, _b);

  std::complex<double> _j(0, 1);

  _C1 = _g0;
  _C2 = ((2.0 * _j * _c * _func.value(t, p) * std::exp(_j * _c * _func.value(t, p) * _L)) -
         (-_g0 * _c * std::sin(_c * _L) + _j * _c * _func.value(t, p) * _g0 * std::cos(_c * _L))) /
        (_c * std::cos(_c * _L) + _j * _c * _func.value(t, p) * std::sin(_c * _L));

  val = _C1 * std::cos(_c * p(0)) + _C2 * std::sin(_c * p(0));

  if (_component == "real")
  {
    return val.real();
  }
  else
  {
    return val.imag();
  }
}
