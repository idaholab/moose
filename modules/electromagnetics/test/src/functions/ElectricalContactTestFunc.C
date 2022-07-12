//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElectricalContactTestFunc.h"

registerMooseObject("ElectromagneticsTestApp", ElectricalContactTestFunc);

InputParameters
ElectricalContactTestFunc::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription(
      "Function used in ElectrostaticContactCondition analytic solution testing.");
  params.addRangeCheckedParam<Real>(
      "mechanical_pressure",
      3000.,
      "mechanical_pressure>0",
      "Mechanical pressure uniformly applied at the contact surface area "
      "(Pressure = Force / Surface Area).");
  params.addRangeCheckedParam<Real>(
      "mean_hardness",
      2.4797e9,
      "mean_hardness>0",
      "Geometric mean of the hardness of graphite and stainless steel.");
  params.addRangeCheckedParam<Real>("graphite_conductivity",
                                    73069.2,
                                    "graphite_conductivity>0",
                                    "Conductivity in graphite (default at 300 K).");
  params.addRangeCheckedParam<Real>("stainless_steel_conductivity",
                                    1.41867e6,
                                    "stainless_steel_conductivity>0",
                                    "Conductivity in stainless steel (default at 300 K).");
  params.addRangeCheckedParam<Real>(
      "contact_conductance",
      75524.,
      "contact_conductance >0",
      "Electrical contact conductance at the interface (default is at 300 K with "
      "3 kN/m^2 applied pressure).");
  MooseEnum domain("stainless_steel graphite");
  params.addParam<MooseEnum>(
      "domain", domain, "Material domain / block of interest (stainless_steel, graphite).");
  params.addParam<bool>("three_block", false, "Is this a three block test case? Default = false.");
  MooseEnum side("left right");
  params.addParam<MooseEnum>("three_block_side",
                             side,
                             "If a three block test case, side / block of interest (left, right).");
  return params;
}

ElectricalContactTestFunc::ElectricalContactTestFunc(const InputParameters & parameters)
  : Function(parameters),
    _electrical_conductivity_graphite(getParam<Real>("graphite_conductivity")),
    _electrical_conductivity_stainless_steel(getParam<Real>("stainless_steel_conductivity")),
    _mean_hardness(getParam<Real>("mean_hardness")),
    _mechanical_pressure(getParam<Real>("mechanical_pressure")),
    _electrical_contact_conductance(getParam<Real>("contact_conductance")),
    _domain(getParam<MooseEnum>("domain")),
    _is_three_block(getParam<bool>("three_block")),
    _side(getParam<MooseEnum>("three_block_side"))
{
}

Real
ElectricalContactTestFunc::value(Real t, const Point & p) const
{
  if (_is_three_block)
  {
    return threeBlockFunction(t, p);
  }
  else
  {
    return twoBlockFunction(t, p);
  }
}

Real
ElectricalContactTestFunc::twoBlockFunction(Real /*t*/, const Point & p) const
{
  Real denominator = _electrical_contact_conductance * _electrical_conductivity_stainless_steel +
                     _electrical_conductivity_graphite * _electrical_conductivity_stainless_steel +
                     _electrical_conductivity_graphite * _electrical_contact_conductance;

  Real graphite_coefficient =
      -_electrical_contact_conductance * _electrical_conductivity_stainless_steel / denominator;

  Real stainless_steel_coefficient =
      -_electrical_contact_conductance * _electrical_conductivity_graphite / denominator;

  Real graphite_func = graphite_coefficient * (p(0) - 2);

  Real stainless_steel_func = stainless_steel_coefficient * p(0) + 1;

  if (_domain == STAINLESS_STEEL)
  {
    return stainless_steel_func;
  }
  else if (_domain == GRAPHITE)
  {
    return graphite_func;
  }
  else
  {
    mooseError(_name + ": Error in selecting proper domain in ElectricalContactTestFunc.");
  }
}

Real
ElectricalContactTestFunc::threeBlockFunction(Real /*t*/, const Point & p) const
{
  Real denominator =
      2.0 * _electrical_conductivity_graphite * _electrical_contact_conductance +
      2.0 * _electrical_conductivity_graphite * _electrical_conductivity_stainless_steel +
      _electrical_conductivity_stainless_steel * _electrical_contact_conductance;

  Real graphite_coefficient =
      -_electrical_conductivity_stainless_steel * _electrical_contact_conductance / denominator;

  Real graphite_constant =
      (_electrical_conductivity_graphite * _electrical_contact_conductance +
       _electrical_conductivity_graphite * _electrical_conductivity_stainless_steel +
       2.0 * _electrical_conductivity_stainless_steel * _electrical_contact_conductance) /
      denominator;

  Real stainless_steel_coefficient =
      -_electrical_conductivity_graphite * _electrical_contact_conductance / denominator;

  Real graphite_func = graphite_coefficient * p(0) + graphite_constant;

  Real stainless_steel_func_left = stainless_steel_coefficient * p(0) + 1;

  Real stainless_steel_func_right = stainless_steel_coefficient * (p(0) - 3);

  /**
   * Enum used in comparisons with _side. Enum-to-enum comparisons are a bit
   * more lightweight, so we should create another enum with the possible choices.
   */
  enum SideEnum
  {
    LEFT,
    RIGHT
  };

  if (_domain == STAINLESS_STEEL && _side == LEFT)
  {
    return stainless_steel_func_left;
  }
  else if (_domain == STAINLESS_STEEL && _side == RIGHT)
  {
    return stainless_steel_func_right;
  }
  else if (_domain == GRAPHITE)
  {
    return graphite_func;
  }
  else
  {
    mooseError(_name + ": Error in selecting proper domain in ElectricalContactTestFunc.");
  }
}
