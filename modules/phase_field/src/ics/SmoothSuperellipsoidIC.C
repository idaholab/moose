//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmoothSuperellipsoidIC.h"

registerMooseObject("PhaseFieldApp", SmoothSuperellipsoidIC);

InputParameters
SmoothSuperellipsoidIC::validParams()
{
  InputParameters params = SmoothSuperellipsoidBaseIC::validParams();
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
