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

  Real Re11 = 0;
  Real Re12 = 0;
  Real Im11 = 0;
  Real Im12 = 0;
  Real Re23 = 0;
  Real Re24 = 0;
  Real Im23 = 0;
  Real Im24 = 0;

  Real sinSinh = std::sin(_a_func * p(0)) * std::sinh(_b_func * p(0));
  Real cosCosh = std::cos(_a_func * p(0)) * std::cosh(_b_func * p(0));
  Real sinCosh = std::sin(_a_func * p(0)) * std::cosh(_b_func * p(0));
  Real cosSinh = std::cos(_a_func * p(0)) * std::sinh(_b_func * p(0));

  // Real _a_second_x = _a_secondDeriv * p(0);
  // Real _b_second_x = _b_secondDeriv * p(0);
  // Real _two_aPrime = 2 * _a_grad;
  // Real _two_bPrime = 2 * _b_grad;
  // Real _aPrimeSq = _a_grad * _a_grad * p(0) * p(0);
  // Real _bPrimeSq = _b_grad * _b_grad * p(0) * p(0);
  // Real _aPrime_a = 2 * _a_grad * _a_func * p(0);
  // Real _aPrime_b = 2 * _a_grad * _b_func * p(0);
  // Real _bPrime_a = 2 * _b_grad * _a_func * p(0);
  // Real _bPrime_b = 2 * _b_grad * _b_func * p(0);

  Real _a_second_plus_aPrime = _a_secondDeriv * p(0) + 2 * _a_grad;
  Real _b_second_plus_bPrime = _b_secondDeriv * p(0) + 2 * _b_grad;

  Real _abSq = _a_grad * _a_grad * p(0) * p(0) - _b_grad * _b_grad * p(0) * p(0) +
               2 * _a_grad * _a_func * p(0) - 2 * _b_grad * _b_func * p(0);
  Real _aPrime_bPrime = 2 * _a_grad * _b_grad * p(0) * p(0) + 2 * _a_grad * _b_func * p(0) +
                        2 * _b_grad * _a_func * p(0);

  if (_component == "real")
  {

    Re11 = _C1_real * (-sinCosh * _a_second_plus_aPrime + cosSinh * _b_second_plus_bPrime);

    Re12 = _C1_real * (-cosCosh * _abSq - sinSinh * _aPrime_bPrime);

    Im11 = _C1_imag * (cosSinh * _a_second_plus_aPrime + sinCosh * _b_second_plus_bPrime);

    Im12 = _C1_imag * (-sinSinh * _abSq + cosCosh * _aPrime_bPrime);

    Re23 = _C2_real * (cosCosh * _a_second_plus_aPrime + sinSinh * _b_second_plus_bPrime);

    Re24 = _C2_real * (-sinCosh * _abSq + cosSinh * _aPrime_bPrime);

    Im23 = _C2_imag * (sinSinh * _a_second_plus_aPrime - cosCosh * _a_second_plus_aPrime);

    Im24 = _C2_imag * (cosSinh * _abSq + sinCosh * _aPrime_bPrime);

    _F = Re11 + Re12 + Im11 + Im12 + Re23 + Re24 + Im23 + Im24;
  }
  else
  {

    Re11 = _C1_real * (-cosSinh * _a_second_plus_aPrime - sinCosh * _b_second_plus_bPrime);

    Re12 = _C1_real * (sinSinh * _abSq - cosCosh * _aPrime_bPrime);

    Im11 = _C1_imag * (-sinCosh * _a_second_plus_aPrime + cosSinh * _b_second_plus_bPrime);

    Im12 = _C1_imag * (-cosCosh * _abSq - sinSinh * _aPrime_bPrime);

    Re23 = _C2_real * (-sinSinh * _a_second_plus_aPrime + cosCosh * _b_second_plus_bPrime);

    Re24 = _C2_real * (-cosSinh * _abSq - sinCosh * _aPrime_bPrime);

    Im23 = _C2_imag * (cosCosh * _a_second_plus_aPrime + sinSinh * _b_second_plus_bPrime);

    Im24 = _C2_imag * (-sinCosh * _abSq + cosSinh * _aPrime_bPrime);

    _F = Re11 + Re12 + Im11 + Im12 + Re23 + Re24 + Im23 + Im24;
  }

  return _F;
}
