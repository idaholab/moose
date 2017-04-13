/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SmoothSuperellipsoidIC.h"

template <>
InputParameters
validParams<SmoothSuperellipsoidIC>()
{
  InputParameters params = validParams<SmoothSuperellipsoidBaseIC>();
  params.addClassDescription("Superellipsoid with a smooth interface");
  params.addRequiredParam<Real>("x1", "The x coordinate of the superellipsoid center");
  params.addRequiredParam<Real>("y1", "The y coordinate of the superellipsoid center");
  params.addParam<Real>("z1", 0.0, "The z coordinate of the superellipsoid center");
  params.addRequiredParam<Real>("a", "Semiaxis a of the superellipsoid");
  params.addRequiredParam<Real>("b", "Semiaxis b of the superellipsoid");
  params.addParam<Real>("c", 1.0, "Semiaxis c of the superellipsoid");
  params.addRequiredParam<Real>("n", "Exponent n of the superellipsoid");
  return params;
}

SmoothSuperellipsoidIC::SmoothSuperellipsoidIC(const InputParameters & parameters)
  : SmoothSuperellipsoidBaseIC(parameters),
    _x1(parameters.get<Real>("x1")),
    _y1(parameters.get<Real>("y1")),
    _z1(parameters.get<Real>("z1")),
    _a(parameters.get<Real>("a")),
    _b(parameters.get<Real>("b")),
    _c(parameters.get<Real>("c")),
    _n(parameters.get<Real>("n")),
    _center(_x1, _y1, _z1)
{
}

void
SmoothSuperellipsoidIC::computeSuperellipsoidCenters()
{
  _centers = {_center};
}

void
SmoothSuperellipsoidIC::computeSuperellipsoidSemiaxes()
{
  _as = {_a};
  _bs = {_b};
  _cs = {_c};
}

void
SmoothSuperellipsoidIC::computeSuperellipsoidExponents()
{
  _ns = {_n};
}
