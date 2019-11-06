//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FrontSource.h"

registerMooseObject("MooseTestApp", FrontSource);

InputParameters
FrontSource::validParams()
{
  InputParameters params = DiracKernel::validParams();

  params.addParam<Real>("value", 1.0, "The value of the strength of the point source.");
  params.addRequiredParam<UserObjectName>(
      "front_uo", "A TrackDiracFront UserObject that will be supplying the positions");

  return params;
}

FrontSource::FrontSource(const InputParameters & parameters)
  : DiracKernel(parameters),
    _value(getParam<Real>("value")),
    _front_tracker(getUserObject<TrackDiracFront>("front_uo"))
{
}

void
FrontSource::addPoints()
{
  const std::vector<std::pair<Elem *, Point>> & points = _front_tracker.getDiracPoints();

  std::vector<std::pair<Elem *, Point>>::const_iterator i = points.begin();
  std::vector<std::pair<Elem *, Point>>::const_iterator end = points.end();

  // Add all of the points the front tracker found
  for (; i != end; ++i)
    addPoint(i->first, i->second);
}

Real
FrontSource::computeQpResidual()
{
  // This is negative because it's a forcing function that has been brought over to the left side.
  return -_test[_i][_qp] * _value;
}
