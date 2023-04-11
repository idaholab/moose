//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LoadMappingDataAction.h"
#include "VariableMappingBase.h"
#include "FEProblem.h"
#include "RestartableDataIO.h"
#include "StochasticToolsApp.h"

registerMooseAction("StochasticToolsApp", LoadMappingDataAction, "load_mapping_data");

InputParameters
LoadMappingDataAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Load the model data for the objects defined in the [VariableMappings] block.");
  return params;
}

LoadMappingDataAction::LoadMappingDataAction(const InputParameters & params) : Action(params) {}

void
LoadMappingDataAction::act()
{
  // We fetch the mapping objects and then load the necessary data
  std::vector<VariableMappingBase *> objects;
  _app.theWarehouse().query().condition<AttribSystem>("VariableMappingBase").queryInto(objects);
  for (auto mapping_ptr : objects)
  {
    if (mapping_ptr && mapping_ptr->isParamValid("filename"))
      load(*mapping_ptr);
  }
}

void
LoadMappingDataAction::load(const VariableMappingBase & mapping)
{
  // File to load
  const FileName & filename = mapping.getParam<FileName>("filename");

  // Create the object that will load in data
  RestartableDataIO data_io(_app);
  data_io.setErrorOnLoadWithDifferentNumberOfProcessors(false);
  data_io.setErrorOnLoadWithDifferentNumberOfThreads(false);

  // Read header
  bool pass = data_io.readRestartableDataHeaderFromFile(filename, false);
  if (!pass)
    mapping.paramError("filename", "The supplied file '", filename, "' failed to load.");

  // Get the data object that the loaded data will be applied
  const RestartableDataMap & meta_data = _app.getRestartableDataMap(mapping.modelMetaDataName());

  // Read the supplied file
  std::unordered_set<std::string> filter_names;
  data_io.readRestartableData(meta_data, filter_names);
}
