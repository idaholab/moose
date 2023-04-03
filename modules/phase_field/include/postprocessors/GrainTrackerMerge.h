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

  // remap grain with the same Grain ID
  virtual void remapMisorientedGrains() override;

  const EulerAngleProvider & _euler;
  MisorientationAngleData _s_misorientation_angle;  

  // This data structure is used to store the mapping from Grain ID to index in feature sets
  std::map<unsigned int, std::size_t> _feature_id_to_index_maps;
};