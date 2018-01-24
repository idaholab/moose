/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SpecifiedSmoothSuperellipsoidIC.h"
#include "MooseRandom.h"

template <>
InputParameters
validParams<SpecifiedSmoothSuperellipsoidIC>()
{
  InputParameters params = validParams<SmoothSuperellipsoidBaseIC>();
  params.addClassDescription("Multiple smooth superellipsoids with manually specified center "
                             "points; semiaxes a,b,c; and exponents n");
  params.addRequiredParam<std::vector<Real>>("x_positions",
                                             "The x-coordinate for each superellipsoid center");
  params.addRequiredParam<std::vector<Real>>("y_positions",
                                             "The y-coordinate for each superellipsoid center");
  params.addRequiredParam<std::vector<Real>>("z_positions",
                                             "The z-coordinate for each superellipsoid center");
  params.addRequiredParam<std::vector<Real>>("as", "Semiaxis a for each superellipsoid");
  params.addRequiredParam<std::vector<Real>>("bs", "Semiaxis b for each superellipsoid");
  params.addRequiredParam<std::vector<Real>>("cs", "Semiaxis c for each superellipsoid");
  params.addRequiredParam<std::vector<Real>>("ns", "Exponent n for each superellipsoid");

  return params;
}

SpecifiedSmoothSuperellipsoidIC::SpecifiedSmoothSuperellipsoidIC(const InputParameters & parameters)
  : SmoothSuperellipsoidBaseIC(parameters),
    _x_positions(getParam<std::vector<Real>>("x_positions")),
    _y_positions(getParam<std::vector<Real>>("y_positions")),
    _z_positions(getParam<std::vector<Real>>("z_positions")),
    _input_as(getParam<std::vector<Real>>("as")),
    _input_bs(getParam<std::vector<Real>>("bs")),
    _input_cs(getParam<std::vector<Real>>("cs")),
    _input_ns(getParam<std::vector<Real>>("ns"))
{
}

void
SpecifiedSmoothSuperellipsoidIC::computeSuperellipsoidCenters()
{
  _centers.resize(_x_positions.size());

  for (unsigned int circ = 0; circ < _x_positions.size(); ++circ)
  {
    _centers[circ](0) = _x_positions[circ];
    _centers[circ](1) = _y_positions[circ];
    _centers[circ](2) = _z_positions[circ];
  }
}

void
SpecifiedSmoothSuperellipsoidIC::computeSuperellipsoidSemiaxes()
{
  _as.resize(_input_as.size());
  _bs.resize(_input_bs.size());
  _cs.resize(_input_cs.size());

  for (unsigned int circ = 0; circ < _input_as.size(); ++circ)
  {
    _as[circ] = _input_as[circ];
    _bs[circ] = _input_bs[circ];
    _cs[circ] = _input_cs[circ];
  }
}

void
SpecifiedSmoothSuperellipsoidIC::computeSuperellipsoidExponents()
{
  _ns.resize(_input_ns.size());

  for (unsigned int circ = 0; circ < _input_ns.size(); ++circ)
    _ns[circ] = _input_ns[circ];
}
