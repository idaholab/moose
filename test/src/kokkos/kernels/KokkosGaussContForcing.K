//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosGaussContForcing.h"

registerKokkosResidualObject("MooseTestApp", KokkosGaussContForcing);

InputParameters
KokkosGaussContForcing::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addParam<Real>("amplitude", 1.0, "Aplitude of the bell curve");
  params.addParam<Real>("x_center", 4.0, "Center of the hump in the X direction");
  params.addParam<Real>("y_center", 6.0, "Center of the hump in the Y direction");
  params.addParam<Real>("z_center", 0.0, "Center of the hump in the Z direction");
  params.addParam<Real>("x_spread", 1.0, "Spread of the curve in the x direction (sigma_x)");
  params.addParam<Real>("y_spread", 1.0, "Spread of the curve in the y direction (sigma_y)");
  params.addParam<Real>("z_spread", 1.0, "Spread of the curve in the z direction (sigma_z)");
  return params;
}

KokkosGaussContForcing::KokkosGaussContForcing(const InputParameters & parameters)
  : Kernel(parameters),
    _amplitude(getParam<Real>("amplitude")),
    _x_center(getParam<Real>("x_center")),
    _y_center(getParam<Real>("y_center")),
    _z_center(getParam<Real>("z_center")),
    _x_spread(getParam<Real>("x_spread")),
    _y_spread(getParam<Real>("y_spread")),
    _z_spread(getParam<Real>("z_spread")),
    _x_min(_x_center - (3.0 * _x_spread)),
    _x_max(_x_center + (3.0 * _x_spread)),
    _y_min(_y_center - (3.0 * _y_spread)),
    _y_max(_y_center + (3.0 * _y_spread)),
    _z_min(_z_center - (3.0 * _z_spread)),
    _z_max(_z_center + (3.0 * _z_spread))
{
}
