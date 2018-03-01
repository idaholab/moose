#include "ZPolarizedWave.h"

template <>
InputParameters
validParams<ZPolarizedWave>()
{
  InputParameters params = validParams<Function>();
  params.addClassDescription("Z-polarized wave function in 1D, at a particular angle of incidence "
                             "relative to the x-axis, on a plane parallel to the y-axis.");
  params.addRequiredParam<Real>("theta", "Angle of incidence, in degrees");
  params.addRequiredParam<Real>("k", "Wave Number");
  MooseEnum component("imaginary real", "real");
  params.addParam<MooseEnum>("component", component, "Real or Imaginary wave component.");
  params.addParam<bool>(
      "sign_flip",
      false,
      "Flag to flip sign/direction of function/wave (default = false -- into slab).");
  return params;
}

ZPolarizedWave::ZPolarizedWave(const InputParameters & parameters)
  : Function(parameters),

    _theta(getParam<Real>("theta")),

    _k(getParam<Real>("k")),

    _component(getParam<MooseEnum>("component")),

    _sign_flip(getParam<bool>("sign_flip"))
{
  if (_sign_flip)
  {
    _sign = -1;
  }
  else
  {
    _sign = 1;
  }
}

Real
ZPolarizedWave::value(Real t, const Point & p)
{
  if (_component == "real")
  {
    return std::cos(_sign * _k * p(0) * std::cos(2 * libMesh::pi * _theta / 360));
  }
  else
  {
    return std::sin(_sign * _k * p(0) * std::cos(2 * libMesh::pi * _theta / 360));
  }
}
