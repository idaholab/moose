// Added function: When misorientation is below a certain threshold, grains merge

#pragma once

#include "GrainTracker.h"

class GrainTrackerMerge : public GrainTracker
{
public:
  static InputParameters validParams();

  GrainTrackerMerge(const InputParameters & parameters);
};