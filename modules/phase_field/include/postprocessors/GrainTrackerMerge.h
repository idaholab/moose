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

  // establish the vector of adjacent grains based on the topological relationship
  virtual void createAdjacentIDVector();

  const EulerAngleProvider & _euler;
  MisorientationAngleData _s_misorientation_angle;  
};