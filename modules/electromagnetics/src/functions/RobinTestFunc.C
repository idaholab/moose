#include "RobinTestFunc.h"
#include <complex>

template <>
InputParameters
validParams<RobinTestFunc>()
{
  InputParameters params = validParams<Function>();
  params.addClassDescription("Function of analytical solution for use in convergence testing with "
                             "RobinTest test file.");
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
  std::complex<double> val(0, 0);

  std::complex<double> _g0(_g0_real, _g0_imag);
  std::complex<double> _c(_a, _b);

  std::complex<double> _j(0, 1);

  Real _x = p(0);

  _C1 = _g0;
  _C2 = (_func.value(t, _L) * _g0 * std::cos(_c * _L) -
         2 * _func.value(t, _L) * std::exp(_j * _c * _L * _func.value(t, _L)) +
         _j * _g0 * std::sin(_c * _L)) /
        (-_func.value(t, _L) * std::sin(_c * _L) + _j * std::cos(_c * _L));

  val = _C1 * std::cos(_c * _x) + _C2 * std::sin(_c * _x);

  if (_component == "real")
  {
    return val.real();
  }
  else
  {
    return val.imag();
  }
}
