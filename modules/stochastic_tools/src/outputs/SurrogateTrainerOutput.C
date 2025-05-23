//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "SurrogateTrainerOutput.h"
#include "SurrogateTrainer.h"
#include "FEProblem.h"
#include "RestartableDataWriter.h"

registerMooseObject("StochasticToolsApp", SurrogateTrainerOutput);

InputParameters
SurrogateTrainerOutput::validParams()
{
  InputParameters params = FileOutput::validParams();
  params.addClassDescription("Output for trained surrogate model data.");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "trainers", "A list of SurrogateTrainer objects to output.");
  return params;
}

SurrogateTrainerOutput::SurrogateTrainerOutput(const InputParameters & parameters)
  : FileOutput(parameters),
    SurrogateModelInterface(this),
    _trainers(getParam<std::vector<UserObjectName>>("trainers"))
{
}

void
SurrogateTrainerOutput::output()
{
  if (processor_id() == 0)
    for (const auto & surrogate_name : _trainers)
    {
      const SurrogateTrainerBase & trainer = getSurrogateTrainerByName(surrogate_name);
      const auto filename =
          RestartableDataIO::restartableDataFolder(this->filename() + "_" + surrogate_name);
      RestartableDataMap & meta_data = _app.getRestartableDataMap(trainer.modelMetaDataName());

      RestartableDataWriter writer(_app, meta_data);
      writer.write(filename);
    }
}
