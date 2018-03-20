#include "MMSTestFunc.h"

template <>
InputParameters
validParams<MMSTestFunc>()
{
  InputParameters params = validParams<Function>();
  params.addClassDescription(
      "Function of RHS for manufactured solution in spatial_constant_helmholtz test.");
  params.addRequiredParam<Real>("L", "Length of 1D test domain, where 0 < x < L");
  params.addRequiredParam<FunctionName>(
      "a",
      "Real component constant of function of squared coefficient c in the PDE, u'' + c^2 * u = 0");
  params.addRequiredParam<FunctionName>(
      "b", "Imaginary component function of squared coefficient c in the PDE, u'' + c^2 * u = 0");
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

  Real _a_func = _a.value(t, p);
  Real _a_func_L = _a.value(t, _L);
  Real _a_grad = _a.gradient(t, p)(0);
  Real _a_secondDeriv = 0; // 2 * _a_func / (_L * _L);

  Real _b_func = _b.value(t, p);
  Real _b_func_L = _b.value(t, _L);
  Real _b_grad = _b.gradient(t, p)(0);
  Real _b_secondDeriv = 0; // 2 * _b_func / (_L * _L);

  Real _coeff = std::pow((std::sin(_a_func_L * _L) * std::cosh(_b_func_L * _L)), 2) +
                std::pow((std::cos(_a_func_L * _L) * std::sinh(_b_func_L * _L)), 2);

  _C1_real = _g0_real;
  _C2_real = (_gL_real * std::sin(_a_func_L * _L) * std::cosh(_b_func_L * _L) +
              _gL_imag * std::cos(_a_func_L * _L) * std::sinh(_b_func_L * _L) -
              _g0_real * std::cos(_a_func_L * _L) * std::sin(_a_func_L * _L) *
                  std::pow(std::cosh(_b_func_L * _L), 2) +
              _g0_real * std::cos(_a_func_L * _L) * std::sin(_a_func_L * _L) *
                  std::pow(std::sinh(_b_func_L * _L), 2) -
              _g0_imag * std::pow(std::cos(_a_func_L * _L), 2) * std::cosh(_b_func_L * _L) *
                  std::sinh(_b_func_L * _L) -
              _g0_imag * std::pow(std::sin(_a_func_L * _L), 2) * std::sinh(_b_func_L * _L) *
                  std::cosh(_b_func_L * _L)) /
             _coeff;

  _C1_imag = _g0_imag;
  _C2_imag = (-_gL_real * std::cos(_a_func_L * _L) * std::sinh(_b_func_L * _L) +
              _gL_imag * std::sin(_a_func_L * _L) * std::cosh(_b_func_L * _L) +
              _g0_real * std::pow(std::cos(_a_func_L * _L), 2) * std::cosh(_b_func_L * _L) *
                  std::sinh(_b_func_L * _L) +
              _g0_real * std::pow(std::sin(_a_func_L * _L), 2) * std::sinh(_b_func_L * _L) *
                  std::cosh(_b_func_L * _L) -
              _g0_imag * std::cos(_a_func_L * _L) * std::sin(_a_func_L * _L) *
                  std::pow(std::cosh(_b_func_L * _L), 2) +
              _g0_imag * std::sin(_a_func_L * _L) * std::cos(_a_func_L * _L) *
                  std::pow(std::sinh(_b_func_L * _L), 2)) /
             _coeff;

  Real _F = 0;

  if (_component == "real")
  {
    _F = _C1_real * (std::sin(_a_func * p(0)) * std::cosh(_b_func * p(0)) *
                         (-_a_secondDeriv * p(0) - 2 * _a_grad) +
                     std::cos(_a_func * p(0)) * std::sinh(_b_func * p(0)) *
                         (_b_secondDeriv * p(0) + 2 * _b_grad) +
                     std::cos(_a_func * p(0)) * std::cosh(_b_func * p(0)) *
                         (-_a_grad * _a_grad * p(0) * p(0) + _b_grad * _b_grad * p(0) * p(0) -
                          2 * _a_grad * _a_func * p(0) + 2 * _b_grad * _b_func * p(0)) +
                     std::sin(_a_func * p(0)) * std::sinh(_b_func * p(0)) *
                         (-2 * _a_grad * _b_grad * p(0) * p(0) - 2 * _a_grad * _b_func * p(0) -
                          2 * _b_grad * _a_func * p(0))) +
         _C1_imag * (std::cos(_a_func * p(0)) * std::sinh(_b_func * p(0)) *
                         (_a_secondDeriv * p(0) + 2 * _a_grad) +
                     std::sin(_a_func * p(0)) * std::cosh(_b_func * p(0)) *
                         (_b_secondDeriv * p(0) + 2 * _b_grad) +
                     std::sin(_a_func * p(0)) * std::sinh(_b_func * p(0)) *
                         (-_a_grad * _a_grad * p(0) * p(0) + _b_grad * _b_grad * p(0) * p(0) -
                          2 * _a_grad * _a_func * p(0) + 2 * _b_grad * _b_func * p(0)) +
                     std::cos(_a_func * p(0)) * std::cosh(_b_func * p(0)) *
                         (2 * _a_grad * _b_grad * p(0) * p(0) + 2 * _a_grad * _b_func * p(0) +
                          2 * _b_grad * _a_func * p(0))) +
         _C2_real * (std::cos(_a_func * p(0)) * std::cosh(_b_func * p(0)) *
                         (_a_secondDeriv * p(0) + 2 * _a_grad) +
                     std::sin(_a_func * p(0)) * std::sinh(_b_func * p(0)) *
                         (_b_secondDeriv * p(0) + 2 * _b_grad) +
                     std::sin(_a_func * p(0)) * std::cosh(_b_func * p(0)) *
                         (-_a_grad * _a_grad * p(0) * p(0) + _b_grad * _b_grad * p(0) * p(0) -
                          2 * _a_grad * _a_func * p(0) + 2 * _b_grad * _b_func * p(0)) +
                     std::cos(_a_func * p(0)) * std::sinh(_b_func * p(0)) *
                         (2 * _a_grad * _b_grad * p(0) * p(0) + 2 * _a_grad * _b_func * p(0) +
                          2 * _b_grad * _a_func * p(0))) +
         _C2_imag * (std::sin(_a_func * p(0)) * std::sinh(_b_func * p(0)) *
                         (_a_secondDeriv * p(0) + 2 * _a_grad) +
                     std::cos(_a_func * p(0)) * std::cosh(_b_func * p(0)) *
                         (-_b_secondDeriv * p(0) - 2 * _b_grad) +
                     std::cos(_a_func * p(0)) * std::sinh(_b_func * p(0)) *
                         (_a_grad * _a_grad * p(0) * p(0) - _b_grad * _b_grad * p(0) * p(0) +
                          2 * _a_grad * _a_func * p(0) - 2 * _b_grad * _b_func * p(0)) +
                     std::sin(_a_func * p(0)) * std::cosh(_b_func * p(0)) *
                         (2 * _a_grad * _b_grad * p(0) * p(0) + 2 * _a_grad * _b_func * p(0) +
                          2 * _b_grad * _a_func * p(0)));
  }
  else
  {
    _F = _C1_real * (std::cos(_a_func * p(0)) * std::sinh(_b_func * p(0)) *
                         (-_a_secondDeriv * p(0) - 2 * _a_grad) +
                     std::sin(_a_func * p(0)) * std::cosh(_b_func * p(0)) *
                         (-_b_secondDeriv * p(0) - 2 * _b_grad) +
                     std::sin(_a_func * p(0)) * std::sinh(_b_func * p(0)) *
                         (_a_grad * _a_grad * p(0) * p(0) - _b_grad * _b_grad * p(0) * p(0) +
                          2 * _a_grad * _a_func * p(0) - 2 * _b_grad * _b_func * p(0)) +
                     std::cos(_a_func * p(0)) * std::cosh(_b_func * p(0)) *
                         (-2 * _a_grad * _b_grad * p(0) * p(0) - 2 * _a_grad * _b_func * p(0) -
                          2 * _b_grad * _a_func * p(0))) +
         _C1_imag * (std::sin(_a_func * p(0)) * std::cosh(_b_func * p(0)) *
                         (-_a_secondDeriv * p(0) - 2 * _a_grad) +
                     std::cos(_a_func * p(0)) * std::sinh(_b_func * p(0)) *
                         (_b_secondDeriv * p(0) + 2 * _b_grad) +
                     std::cos(_a_func * p(0)) * std::cosh(_b_func * p(0)) *
                         (-_a_grad * _a_grad * p(0) * p(0) + _b_grad * _b_grad * p(0) * p(0) -
                          2 * _a_grad * _a_func * p(0) + 2 * _b_grad * _b_func * p(0)) +
                     std::sin(_a_func * p(0)) * std::sinh(_b_func * p(0)) *
                         (-2 * _a_grad * _b_grad * p(0) * p(0) - 2 * _a_grad * _b_func * p(0) -
                          2 * _b_grad * _a_func * p(0))) +
         _C2_real * (std::sin(_a_func * p(0)) * std::sinh(_b_func * p(0)) *
                         (-_a_secondDeriv * p(0) - 2 * _a_grad) +
                     std::cos(_a_func * p(0)) * std::cosh(_b_func * p(0)) *
                         (_b_secondDeriv * p(0) + 2 * _b_grad) +
                     std::cos(_a_func * p(0)) * std::sinh(_b_func * p(0)) *
                         (-_a_grad * _a_grad * p(0) * p(0) + _b_grad * _b_grad * p(0) * p(0) -
                          2 * _a_grad * _a_func * p(0) + 2 * _b_grad * _b_func * p(0)) +
                     std::sin(_a_func * p(0)) * std::cosh(_b_func * p(0)) *
                         (-2 * _a_grad * _b_grad * p(0) * p(0) - 2 * _a_grad * _b_func * p(0) -
                          2 * _b_grad * _a_func * p(0))) +
         _C2_imag * (std::cos(_a_func * p(0)) * std::cosh(_b_func * p(0)) *
                         (_a_secondDeriv * p(0) + 2 * _a_grad) +
                     std::sin(_a_func * p(0)) * std::sinh(_b_func * p(0)) *
                         (_b_secondDeriv * p(0) + 2 * _b_grad) +
                     std::sin(_a_func * p(0)) * std::cosh(_b_func * p(0)) *
                         (-_a_grad * _a_grad * p(0) * p(0) + _b_grad * _b_grad * p(0) * p(0) -
                          2 * _a_grad * _a_func * p(0) + 2 * _b_grad * _b_func * p(0)) +
                     std::cos(_a_func * p(0)) * std::sinh(_b_func * p(0)) *
                         (2 * _a_grad * _b_grad * p(0) * p(0) + 2 * _a_grad * _b_func * p(0) +
                          2 * _b_grad * _a_func * p(0)));
  }

  return _F;
}
