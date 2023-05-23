//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "MappingOutput.h"
#include "RestartableDataIO.h"
#include "RestartableData.h"

registerMooseObject("StochasticToolsApp", MappingOutput);

InputParameters
MappingOutput::validParams()
{
  InputParameters params = FileOutput::validParams();
  params.addClassDescription("Output for mapping model data.");
  params.addRequiredParam<std::vector<std::string>>("mappings",
                                                    "A list of Mapping objects to output.");
  return params;
}

MappingOutput::MappingOutput(const InputParameters & parameters)
  : FileOutput(parameters),
    MappingInterface(this),
    _mappings(getParam<std::vector<std::string>>("mappings"))
{
}

void
MappingOutput::output(const ExecFlagType & /*type*/)
{
  if (processor_id() == 0)
  {
    RestartableDataIO restartable_data_io(_app);
    for (const auto & map_name : _mappings)
    {
      const VariableMappingBase & map = getMappingByName(map_name);
      const std::string filename =
          this->filename() + "_" + map_name + restartable_data_io.getRestartableDataExt();

      const RestartableDataMap & meta_data = _app.getRestartableDataMap(map.modelMetaDataName());
      restartable_data_io.writeRestartableData(filename, meta_data);
    }
  }
}
