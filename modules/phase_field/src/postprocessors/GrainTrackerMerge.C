#include "GrainTrackerMerge.h"

registerMooseObject("PhaseFieldApp", GrainTrackerMerge);

InputParameters
GrainTrackerMerge::validParams()
{
  InputParameters params = GrainTrackerMerge::validParams();
  params.addClassDescription("Grain Tracker derived object for merging of grains based on misorientation angle.");

  return params;
}

GrainTrackerMerge::GrainTrackerMerge(const InputParameters & parameters)
  : GrainTracker(parameters)
{
}