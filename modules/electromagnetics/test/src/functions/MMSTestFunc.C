#include "MMSTestFunc.h"
#include <complex>

registerMooseObject("ElkApp", MMSTestFunc);

template <>
InputParameters
validParams<MMSTestFunc>()
{
  InputParameters params = validParams<Function>();
  params.addClassDescription(
      "Function of RHS for manufactured solution in spatial_constant_helmholtz test.");
  params.addRequiredParam<Real>("L", "Length of 1D test domain, where 0 < x < L");
  params.addRequiredParam<FunctionName>("a",
                                        "Real component constant of function of squared "
                                        "coefficient c in the PDE, -u'' - c^2 * u = -F");
  params.addRequiredParam<FunctionName>(
      "b", "Imaginary component function of squared coefficient c in the PDE, -u'' - c^2 * u = -F");
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

MMSTestFunc::MMSTestFunc(const InputParameters & parameters)
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
MMSTestFunc::value(Real t, const Point & p)
{

  Real _val = 0;
  std::complex<double> _F(0, 0);

  std::complex<double> _g0(_g0_real, _g0_imag);
  std::complex<double> _gL(_gL_real, _gL_imag);

  Real _a_grad = _a.gradient(t, p)(0);
  Real _b_grad = _b.gradient(t, p)(0);
  // assuming a constant * (1 + x / _L)^2 structure for functions
  Real _a_second = 2 * _a.value(t, 0) / (_L * _L);
  Real _b_second = 2 * _b.value(t, 0) / (_L * _L);

  std::complex<double> _c(_a.value(t, p), _b.value(t, p));
  std::complex<double> _c_L(_a.value(t, _L), _b.value(t, _L));

  std::complex<double> _c_grad(_a_grad, _b_grad);
  std::complex<double> _c_second(_a_second, _b_second);

  std::complex<double> _C1 = _g0;

  std::complex<double> _C2 = (_gL - _g0 * std::cos(_c_L * _L)) / std::sin(_c_L * _L);

  _F = -_C1 * ((_c_second * p(0) + 2.0 * _c_grad) * std::sin(_c * p(0)) +
               (std::pow(_c_grad * p(0), 2) + 2.0 * _c_grad * _c * p(0)) * std::cos(_c * p(0))) +
       _C2 * ((_c_second * p(0) + 2.0 * _c_grad) * std::cos(_c * p(0)) -
              (std::pow(_c_grad * p(0), 2) + 2.0 * _c_grad * _c * p(0)) * std::sin(_c * p(0)));

  if (_component == "real")
  {
    _val = _F.real();
  }
  else
  {
    _val = _F.imag();
  }

  // sign flip because being used in -u'' - c^2 * u = -F(x) strong form
  return -_val;
}
