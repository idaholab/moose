//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NearestPointTrainer.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", NearestPointTrainer);

InputParameters
NearestPointTrainer::validParams()
{
  InputParameters params = SurrogateTrainer::validParams();
  params.addClassDescription("Loops over and saves sample values for [NearestPointSurrogate.md].");

  return params;
}

NearestPointTrainer::NearestPointTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _sample_points(declareModelData<std::vector<std::vector<Real>>>("_sample_points")),
    _sample_results(declareModelData<std::vector<std::vector<Real>>>("_sample_results")),
    _predictor_row(getPredictorData())
{
  _sample_points.resize(_n_dims);
  _sample_results.resize(1);
}

void
NearestPointTrainer::preTrain()
{
  for (auto & it : _sample_points)
  {
    it.clear();
    it.reserve(getLocalSampleSize());
  }

  for (auto & it : _sample_results)
  {
    it.clear();
    it.reserve(getLocalSampleSize());
  }
}

void
NearestPointTrainer::train()
{
  if (_rvecval && (_sample_results.size() != _rvecval->size()))
    _sample_results.resize(_rvecval->size());

  // Get predictors from reporter values
  for (auto d : make_range(_n_dims))
    _sample_points[d].push_back(_predictor_row[d]);

  // Get responses
  if (_rval)
    _sample_results[0].push_back(*_rval);
  else if (_rvecval)
    for (auto r : make_range(_rvecval->size()))
      _sample_results[r].push_back((*_rvecval)[r]);
}

void
NearestPointTrainer::postTrain()
{
  for (auto & it : _sample_points)
    _communicator.allgather(it);

  for (auto & it : _sample_results)
    _communicator.allgather(it);
}
