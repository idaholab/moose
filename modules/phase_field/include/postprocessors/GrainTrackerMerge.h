// Added function: When misorientation is below a certain threshold, grains merge

#pragma once

#include "GrainTracker.h"
#include "EulerAngleProvider.h"
#include "MisorientationAngleCalculator.h"

class GrainTrackerMerge : public GrainTracker
{
public:
  static InputParameters validParams();

  GrainTrackerMerge(const InputParameters & parameters);

protected:
  // re-merge grains due to misorientation angle from euler angles calculation
  virtual void mergeGrainsBasedMisorientation() override;

  const EulerAngleProvider & _euler;
  misoriAngle_isTwining _s_misoriTwin;  
};