/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "RandomEulerAngleProvider.h"

template<>
InputParameters validParams<RandomEulerAngleProvider>()
{
  InputParameters params = validParams<EulerAngleProvider>();
  params.addClassDescription("Assign random euler angles for each grain.");
  params.addRequiredParam<UserObjectName>("grain_tracker_object", "The FeatureFloodCount UserObject to get values from.");
  return params;
}

RandomEulerAngleProvider::RandomEulerAngleProvider(const InputParameters & params) :
    EulerAngleProvider(params),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_tracker_object")),
    _angles(0)
{
}

void
RandomEulerAngleProvider::initialize()
{
  EulerAngles angle;
  auto grain_num = _grain_tracker.getTotalFeatureCount();
  for (unsigned int i = _angles.size(); i < grain_num; ++i)
  {
    angle.random();
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
