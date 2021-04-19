//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurrogateTrainer.h"
#include "Sampler.h"
#include "RestartableDataIO.h"
#include "StochasticToolsApp.h"

InputParameters
SurrogateTrainerBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.registerBase("SurrogateTrainer");
  return params;
}

SurrogateTrainerBase::SurrogateTrainerBase(const InputParameters & parameters)
  : GeneralUserObject(parameters), _model_meta_data_name(_type + "_" + name())
{
  _app.registerRestartableDataMapName(_model_meta_data_name, name());
}

InputParameters
SurrogateTrainer::validParams()
{
  InputParameters params = SurrogateTrainerBase::validParams();
  params.addRequiredParam<SamplerName>("sampler",
                                       "Sampler used to create predictor and response data.");
  return params;
}

SurrogateTrainer::SurrogateTrainer(const InputParameters & parameters)
  : SurrogateTrainerBase(parameters),
    _sampler(getSampler("sampler")),
    _row_data(_sampler.getNumberOfCols())
{
}

void
SurrogateTrainer::initialize()
{
  // Figure out if data is distributed
  for (auto & pair : _training_data)
  {
    const ReporterName & name = pair.first;
    TrainingDataBase & data = *pair.second;

    const auto & mode = _fe_problem.getReporterData().getReporterMode(name);
    if (mode == REPORTER_MODE_DISTRIBUTED || (mode == REPORTER_MODE_ROOT && processor_id() != 0))
      data.isDistributed() = true;
    else if (mode == REPORTER_MODE_REPLICATED ||
             (mode == REPORTER_MODE_ROOT && processor_id() == 0))
      data.isDistributed() = false;
    else
      mooseError("Predictor reporter value ", name, " is not of supported mode.");
  }
}

void
SurrogateTrainer::execute()
{
  checkIntegrity();

  _row = _sampler.getLocalRowBegin();
  _local_row = 0;

  preTrain();

  for (_row = _sampler.getLocalRowBegin(); _row < _sampler.getLocalRowEnd(); ++_row)
  {
    // Need to do this manually in order to keep the iterators valid
    const std::vector<Real> data = _sampler.getNextLocalRow();
    for (unsigned int i = 0; i < _row_data.size(); ++i)
      _row_data[i] = data[i];

    // Set training data
    for (auto & pair : _training_data)
      pair.second->setCurrentIndex((pair.second->isDistributed() ? _local_row : _row));

    train();

    _local_row++;
  }

  postTrain();
}

void
SurrogateTrainer::checkIntegrity() const
{
  // Check that the number of sampler columns hasn't changed
  if (_row_data.size() != _sampler.getNumberOfCols())
    mooseError("Number of sampler columns has changed.");

  // Check that training data is correctly sized
  for (auto & pair : _training_data)
  {
    dof_id_type rsize = pair.second->size();
    dof_id_type nrow =
        pair.second->isDistributed() ? _sampler.getNumberOfLocalRows() : _sampler.getNumberOfRows();
    if (rsize != nrow)
      mooseError("Reporter value ",
                 pair.first,
                 " of size ",
                 rsize,
                 " does not match sampler size (",
                 nrow,
                 ").");
  }
}
