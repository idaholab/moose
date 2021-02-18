#include "JinSlabCoeffFunc.h"
#include "ElkEnums.h"
#include <complex>

registerMooseObject("ElkApp", JinSlabCoeffFunc);

template <>
InputParameters
validParams<JinSlabCoeffFunc>()
{
  InputParameters params = validParams<Function>();
  params.addClassDescription("Function describing a wave incident on a surface at a given angle "
                             "and wave number, for use in reflection and transmission problems.");
  params.addRequiredParam<Real>("k", "Wave Number");
  params.addRequiredParam<Real>("theta", "Wave Incidence angle, in degrees");
  MooseEnum component("real imaginary");
  params.addParam<MooseEnum>("component", component, "Real or Imaginary wave component");
  return params;
}

JinSlabCoeffFunc::JinSlabCoeffFunc(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),

    _k(getParam<Real>("k")),
    _theta(getParam<Real>("theta")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
JinSlabCoeffFunc::value(Real /*t*/, const Point & p) const
{

  std::complex<double> jay(0, 1);
  std::complex<double> eps_r = 4.0 + (2.0 - jay * 0.1) * std::pow((1 - p(0) * _k / 5), 2);
  std::complex<double> mu_r(2, -0.1);

  std::complex<double> val =
      _k * std::sqrt(eps_r * mu_r - std::pow(std::sin(_theta * libMesh::pi / 180.), 2));

  if (_component == elk::REAL)
  {
    return val.real();
  }
  else
  {
    return val.imag();
  }
}
