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
      std::vector<ReporterName>(),
      "Reporter values used as the independent random variables, If 'predictors' and "
      "'predictor_cols' are both empty, all sampler columns are used.");
  params.addParam<std::vector<unsigned int>>(
      "predictor_cols",
      std::vector<unsigned int>(),
      "Sampler columns used as the independent random variables, If 'predictors' and "
      "'predictor_cols' are both empty, all sampler columns are used.");

  return params;
}

NearestPointTrainer::NearestPointTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _sample_points(declareModelData<std::vector<std::vector<Real>>>("_sample_points")),
    _sampler_row(getSamplerData()),
    _response(getTrainingData<Real>(getParam<ReporterName>("response"))),
    _predictor_cols(getParam<std::vector<unsigned int>>("predictor_cols"))
{
  for (const ReporterName & rname : getParam<std::vector<ReporterName>>("predictors"))
    _predictors.push_back(&getTrainingData<Real>(rname));

  // If predictors and predictor_cols are empty, use all sampler columns
  if (_predictors.empty() && _predictor_cols.empty())
  {
    _predictor_cols.resize(_sampler.getNumberOfCols());
    std::iota(_predictor_cols.begin(), _predictor_cols.end(), 0);
  }

  // Resize sample points to number of predictors
  _sample_points.resize(_predictors.size() + _predictor_cols.size() + 1);
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
  unsigned int d = 0;
  // Get predictors from reporter values
  for (const auto & val : _predictors)
    _sample_points[d++][_local_row] = *val;
  // Get predictors from sampler
  for (const auto & col : _predictor_cols)
    _sample_points[d++][_local_row] = _sampler_row[col];

  _sample_points.back()[_local_row] = _response;
}

void
NearestPointTrainer::postTrain()
{
  for (auto & it : _sample_points)
    _communicator.allgather(it);
}
