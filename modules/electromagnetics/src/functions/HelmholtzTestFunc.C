#include "HelmholtzTestFunc.h"
#include "ElectromagneticEnums.h"
#include <complex>

registerMooseObject("ElectromagneticsApp", HelmholtzTestFunc);

InputParameters
HelmholtzTestFunc::validParams()
{
  InputParameters params = Function::validParams();
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
  MooseEnum component("real imaginary");
  params.addParam<MooseEnum>("component", component, "Real or Imaginary wave component.");
  return params;
}

HelmholtzTestFunc::HelmholtzTestFunc(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),

    _length(getParam<Real>("L")),
    _a(getFunction("a")),
    _b(getFunction("b")),
    _d(getParam<Real>("d")),
    _h(getParam<Real>("h")),
    _g_0_real(getParam<Real>("g0_real")),
    _g_0_imag(getParam<Real>("g0_imag")),
    _g_l_real(getParam<Real>("gL_real")),
    _g_l_imag(getParam<Real>("gL_imag")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
HelmholtzTestFunc::value(Real t, const Point & p) const
{
  std::complex<double> c_1(0, 0);
  std::complex<double> c_2(0, 0);
  std::complex<double> lambda(0, 0);
  std::complex<double> lambda_l(0, 0);
  std::complex<double> val(0, 0);

  std::complex<double> g_0(_g_0_real, _g_0_imag);
  std::complex<double> g_l(_g_l_real, _g_l_imag);
  std::complex<double> c(_a.value(t, p), _b.value(t, p));
  std::complex<double> c_l(_a.value(t, _length), _b.value(t, _length));
  std::complex<double> k(_d, _h);

  lambda = c / std::sqrt(k);
  lambda_l = c_l / std::sqrt(k);

  c_1 = g_0;
  c_2 = (g_l - g_0 * std::cos(lambda_l * _length)) / std::sin(lambda_l * _length);

  val = c_1 * std::cos(lambda * p(0)) + c_2 * std::sin(lambda * p(0));

  if (_component == electromagnetics::REAL)
  {
    return val.real();
  }
  else
  {
    return val.imag();
  }
}
