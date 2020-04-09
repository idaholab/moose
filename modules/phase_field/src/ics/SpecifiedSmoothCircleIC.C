//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SpecifiedSmoothCircleIC.h"
#include "MooseRandom.h"

registerMooseObject("PhaseFieldApp", SpecifiedSmoothCircleIC);

InputParameters
SpecifiedSmoothCircleIC::validParams()
{
  InputParameters params = SmoothCircleBaseIC::validParams();
  params.addClassDescription(
      "Multiple smooth circles with manually specified radii and center points");
  params.addRequiredParam<std::vector<Real>>("x_positions",
                                             "The x-coordinate for each circle center");
  params.addRequiredParam<std::vector<Real>>("y_positions",
                                             "The y-coordinate for each circle center");
  params.addRequiredParam<std::vector<Real>>("z_positions",
                                             "The z-coordinate for each circle center");
  params.addRequiredParam<std::vector<Real>>("radii", "The radius for each circle");

  return params;
}

SpecifiedSmoothCircleIC::SpecifiedSmoothCircleIC(const InputParameters & parameters)
  : SmoothCircleBaseIC(parameters),
    _x_positions(getParam<std::vector<Real>>("x_positions")),
    _y_positions(getParam<std::vector<Real>>("y_positions")),
    _z_positions(getParam<std::vector<Real>>("z_positions")),
    _input_radii(getParam<std::vector<Real>>("radii"))
{
}

void
SpecifiedSmoothCircleIC::computeCircleRadii()
{
  _radii.resize(_input_radii.size());

  for (unsigned int circ = 0; circ < _input_radii.size(); ++circ)
    _radii[circ] = _input_radii[circ];
}

void
SpecifiedSmoothCircleIC::computeCircleCenters()
{
  _centers.resize(_x_positions.size());

  for (unsigned int circ = 0; circ < _x_positions.size(); ++circ)
  {
    _centers[circ](0) = _x_positions[circ];
    _centers[circ](1) = _y_positions[circ];
    _centers[circ](2) = _z_positions[circ];
  }
}
