#include "HelmholtzTestFunc.h"
#include <complex>

registerMooseObject("ElkApp", HelmholtzTestFunc);

template <>
InputParameters
validParams<HelmholtzTestFunc>()
{
  InputParameters params = validParams<Function>();
  params.addClassDescription("Function of analytical solution for use in convergence testing with "
                             "coupled_helmholtz test file.");
  params.addRequiredParam<Real>("L", "Length of 1D test domain, where 0 < x < L");
  params.addRequiredParam<FunctionName>(
      "a", "Real component of squared coefficient c in the PDE, u'' + c^2 * u = 0");
  params.addRequiredParam<FunctionName>(
      "b", "Imaginary component of squared coefficient c in the PDE, u'' + c^2 * u = 0");
  params.addParam<Real>("d", 1.0, "Real component of optional laplacian coefficient");
  params.addParam<Real>("h", 0.0, "Imaginary component of optional laplacian coefficient");
  params.addRequiredParam<Real>("g0_real", "Real component of DirichletBC where x = 0.");
  params.addRequiredParam<Real>("g0_imag", "Imaginary component of DirichletBC where x = 0.");
  params.addRequiredParam<Real>("gL_real", "Real component of DirichletBC where x = L.");
  params.addRequiredParam<Real>("gL_imag", "Imaginary component of DirichletBC where x = L.");
  MooseEnum component("imaginary real", "real");
  params.addParam<MooseEnum>("component", component, "Real or Imaginary wave component.");
  return params;
}

HelmholtzTestFunc::HelmholtzTestFunc(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),

    _L(getParam<Real>("L")),
    _a(getFunction("a")),
    _b(getFunction("b")),
    _d(getParam<Real>("d")),
    _h(getParam<Real>("h")),
    _g0_real(getParam<Real>("g0_real")),
    _g0_imag(getParam<Real>("g0_imag")),
    _gL_real(getParam<Real>("gL_real")),
    _gL_imag(getParam<Real>("gL_imag")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
HelmholtzTestFunc::value(Real t, const Point & p)
{
  std::complex<double> _C1(0, 0);
  std::complex<double> _C2(0, 0);
  std::complex<double> _lambda(0, 0);
  std::complex<double> _lambda_L(0, 0);
  std::complex<double> val(0, 0);

  std::complex<double> _g0(_g0_real, _g0_imag);
  std::complex<double> _gL(_gL_real, _gL_imag);
  std::complex<double> _c(_a.value(t, p), _b.value(t, p));
  std::complex<double> _c_L(_a.value(t, _L), _b.value(t, _L));
  std::complex<double> _k(_d, _h);

  _lambda = _c / std::sqrt(_k);
  _lambda_L = _c_L / std::sqrt(_k);

  _C1 = _g0;
  _C2 = (_gL - _g0 * std::cos(_lambda_L * _L)) / std::sin(_lambda_L * _L);

  val = _C1 * std::cos(_lambda * p(0)) + _C2 * std::sin(_lambda * p(0));

  if (_component == "real")
  {
    return val.real();
  }
  else
  {
    return val.imag();
  }
}
