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
  params.addRequiredParam<SamplerName>("sampler", "Training set defined by a sampler object.");
  params.addRequiredParam<VectorPostprocessorName>(
      "results_vpp", "Vectorpostprocessor with results of samples created by trainer.");
  params.addRequiredParam<std::string>(
      "results_vector",
      "Name of vector from vectorpostprocessor with results of samples created by trainer");

  return params;
}

NearestPointTrainer::NearestPointTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _sample_points(declareModelData<std::vector<std::vector<Real>>>("_sample_points"))
{
}

void
NearestPointTrainer::initialSetup()
{
  // Results VPP
  _values_distributed = isVectorPostprocessorDistributed("results_vpp");
  _values_ptr = &getVectorPostprocessorValue(
      "results_vpp", getParam<std::string>("results_vector"), !_values_distributed);

  // Sampler
  _sampler = &getSamplerByName(getParam<SamplerName>("sampler"));
}

void
NearestPointTrainer::initialize()
{
  // Check if results of samples matches number of samples
  __attribute__((unused)) dof_id_type num_rows =
      _values_distributed ? _sampler->getNumberOfLocalRows() : _sampler->getNumberOfRows();
  mooseAssert(num_rows == _values_ptr->size(),
              "Sampler number of rows does not match number of results from vector postprocessor.");

  // Resize to number of sample points
  _sample_points.resize(_sampler->getNumberOfCols() + 1);
  for (auto & it : _sample_points)
    it.resize(_sampler->getNumberOfLocalRows());
}

void
NearestPointTrainer::execute()
{
  // Offset for replicated/distributed result data
  dof_id_type offset = _values_distributed ? _sampler->getLocalRowBegin() : 0;

  // Loop over samples
  for (dof_id_type p = _sampler->getLocalRowBegin(); p < _sampler->getLocalRowEnd(); ++p)
  {
    std::vector<Real> data = _sampler->getNextLocalRow();

    dof_id_type ind = p - _sampler->getLocalRowBegin();
    for (unsigned int i = 0; i < data.size(); ++i)
      _sample_points[i][ind] = data[i];
    _sample_points[data.size()][ind] = (*_values_ptr)[p - offset];
  }
}

void
NearestPointTrainer::finalize()
{
  for (auto & it : _sample_points)
    _communicator.allgather(it);
}
