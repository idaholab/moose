#include "RobinTestFunc.h"
#include "ElkEnums.h"
#include <complex>

registerMooseObject("ElkApp", RobinTestFunc);

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
  MooseEnum component("real imaginary");
  params.addParam<MooseEnum>("component", component, "Real or Imaginary wave component.");
  return params;
}

RobinTestFunc::RobinTestFunc(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),

    _length(getParam<Real>("L")),
    _a(getParam<Real>("a")),
    _b(getParam<Real>("b")),
    _g_0_real(getParam<Real>("g0_real")),
    _g_0_imag(getParam<Real>("g0_imag")),
    _func(getFunction("func")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
RobinTestFunc::value(Real t, const Point & p) const
{
  std::complex<double> c_1(0, 0);
  std::complex<double> c_2(0, 0);
  std::complex<double> val(0, 0);

  std::complex<double> g_0(_g_0_real, _g_0_imag);
  std::complex<double> c(_a, _b);

  std::complex<double> jay(0, 1);

  Real x = p(0);

  c_1 = g_0;
  c_2 = (_func.value(t, _length) * g_0 * std::cos(c * _length) -
         2 * _func.value(t, _length) * std::exp(jay * c * _length * _func.value(t, _length)) +
         jay * g_0 * std::sin(c * _length)) /
        (-_func.value(t, _length) * std::sin(c * _length) + jay * std::cos(c * _length));

  val = c_1 * std::cos(c * x) + c_2 * std::sin(c * x);

  if (_component == elk::REAL)
  {
    return val.real();
  }
  else
  {
    return val.imag();
  }
}
