//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JinSlabCoeffFunc.h"
#include "ElectromagneticEnums.h"
#include <complex>

registerMooseObject("ElectromagneticsTestApp", JinSlabCoeffFunc);

InputParameters
JinSlabCoeffFunc::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription(
      "Function describing a wave incident on a surface at a given angle, wavenumber, and domain "
      "length, for use in the slab reflection benchmark.");
  params.addRequiredParam<Real>("k", "Wavenumber");
  params.addRequiredParam<Real>("theta", "Wave Incidence angle, in degrees");
  params.addRequiredParam<Real>("length", "Length of slab domain");
  MooseEnum component("real imaginary");
  params.addParam<MooseEnum>("component", component, "Real or Imaginary wave component");
  return params;
}

JinSlabCoeffFunc::JinSlabCoeffFunc(const InputParameters & parameters)
  : Function(parameters),

    _k(getParam<Real>("k")),
    _theta(getParam<Real>("theta")),
    _length(getParam<Real>("length")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
JinSlabCoeffFunc::value(Real /*t*/, const Point & p) const
{
  std::complex<double> jay(0, 1);
  std::complex<double> eps_r = 4.0 + (2.0 - jay * 0.1) * std::pow((1 - p(0) / _length), 2);
  std::complex<double> mu_r(2, -0.1);

  std::complex<double> val =
      mu_r * std::pow(_k, 2) * (eps_r - (1.0 / mu_r) * std::pow(sin(_theta * libMesh::pi / 180.), 2));

  if (_component == electromagnetics::REAL)
  {
    return val.real();
  }
  else
  {
    return val.imag();
  }
}
