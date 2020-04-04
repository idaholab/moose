//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RandomEulerAngleProvider.h"
#include "GrainTrackerInterface.h"

registerMooseObject("PhaseFieldApp", RandomEulerAngleProvider);

InputParameters
RandomEulerAngleProvider::validParams()
{
  InputParameters params = EulerAngleProvider::validParams();
  params.addClassDescription("Assign random Euler angles for each grain.");
  params.addRequiredParam<UserObjectName>("grain_tracker_object",
                                          "The FeatureFloodCount UserObject to get values from.");
  params.addParam<unsigned int>("seed", 0, "Seed value for the random number generator");
  return params;
}

RandomEulerAngleProvider::RandomEulerAngleProvider(const InputParameters & params)
  : EulerAngleProvider(params),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_tracker_object")),
    _angles(0)
{
  _random.seed(0, getParam<unsigned int>("seed"));
}

void
RandomEulerAngleProvider::initialize()
{
  EulerAngles angle;
  auto grain_num = _grain_tracker.getTotalFeatureCount();
  for (auto i = _angles.size(); i < grain_num; ++i)
  {
    angle.random(_random);
    _angles.push_back(angle);
  }
}

unsigned int
RandomEulerAngleProvider::getGrainNum() const
{
  return _angles.size();
}

const EulerAngles &
RandomEulerAngleProvider::getEulerAngles(unsigned int i) const
{
  mooseAssert(i < getGrainNum(), "Requesting Euler angles for an invalid grain id");
  return _angles[i];
}
