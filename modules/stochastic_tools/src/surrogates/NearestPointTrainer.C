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
  params.addRequiredParam<ReporterName>(
      "response",
      "Reporter value of response results, can be vpp with <vpp_name>/<vector_name> or sampler "
      "column with 'sampler/col_<index>'.");
  params.addParam<std::vector<ReporterName>>(
      "predictors",
      "Reporter values used as the independent random variables, sampler columns can be used with "
      "'sampler/col_<index>' syntax. Default is to use all sampler columns.");

  return params;
}

NearestPointTrainer::NearestPointTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _sample_points(declareModelData<std::vector<std::vector<Real>>>("_sample_points")),
    _response(getTrainingData<Real>("response")),
    _predictors(getTrainingDataVector<Real>("predictors", true))
{
  _sample_points.resize(_predictors.size() + 1);
}

void
NearestPointTrainer::preTrain()
{
  // Resize to number of sample points
  for (auto & it : _sample_points)
    it.resize(_sampler.getNumberOfLocalRows());
}

void
NearestPointTrainer::train()
{
  for (unsigned int i = 0; i < _predictors.size(); ++i)
    _sample_points[i][_local_row] = *_predictors[i];
  _sample_points.back()[_local_row] = _response;
}

void
NearestPointTrainer::postTrain()
{
  for (auto & it : _sample_points)
    _communicator.allgather(it);
}
