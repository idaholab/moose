/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "FeatureBoundingBoxVectorPostprocessor.h"
#include "FeatureFloodCount.h"
#include "GrainTrackerInterface.h"
#include "MooseMesh.h"
#include "Assembly.h"

#include "libmesh/quadrature.h"

template<>
InputParameters validParams<FeatureBoundingBoxVectorPostprocessor>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();

  params.addRequiredParam<UserObjectName>("flood_counter", "The FeatureFloodCount UserObject to get values from.");
  return params;
}

FeatureBoundingBoxVectorPostprocessor::FeatureBoundingBoxVectorPostprocessor(const InputParameters & parameters) :
    GeneralVectorPostprocessor(parameters),
    MooseVariableDependencyInterface(),
    _feature_counter(getUserObject<FeatureFloodCount>("flood_counter")),
    _feature_id(declareVector("feature_id")),
    _min_x(declareVector("min_x")),
    _min_y(declareVector("min_y")),
    _min_z(declareVector("min_z")),
    _max_x(declareVector("max_x")),
    _max_y(declareVector("max_y")),
    _max_z(declareVector("max_z")),
    _vars(_feature_counter.getCoupledVars())
{
  addMooseVariableDependency(_vars);
}

void
FeatureBoundingBoxVectorPostprocessor::initialize()
{
  _feature_id.clear();
  _min_x.clear();
  _min_y.clear();
  _min_z.clear();
  _max_x.clear();
  _max_y.clear();
  _max_z.clear();
}

void
FeatureBoundingBoxVectorPostprocessor::execute()
{
  // We only need to run this method on the master
  if (processor_id() > 0)
    return;

  auto num_features = _feature_counter.getTotalFeatureCount();

  // We'll need at least as much memory as the number of features, but possibly more
  _min_x.reserve(num_features);
  _min_y.reserve(num_features);
  _min_z.reserve(num_features);
  _max_x.reserve(num_features);
  _max_y.reserve(num_features);
  _max_z.reserve(num_features);

  for (decltype(num_features) feature_num = 0; feature_num < num_features; ++feature_num)
  {
    const auto & feature = _feature_counter.getFeature(feature_num);
    if (feature._id == FeatureFloodCount::invalid_id || feature._status == FeatureFloodCount::Status::INACTIVE)
      continue;

    for (const auto & bbox : feature._bboxes)
    {
      const auto & min = bbox.min();
      const auto & max = bbox.max();

      _feature_id.push_back(feature._id);
      _min_x.push_back(min(0));
      _min_y.push_back(min(1));
      _min_z.push_back(min(2));
      _max_x.push_back(max(0));
      _max_y.push_back(max(1));
      _max_z.push_back(max(2));
    }
  }
}

void
FeatureBoundingBoxVectorPostprocessor::finalize()
{
}
