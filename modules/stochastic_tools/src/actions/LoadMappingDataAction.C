//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LoadMappingDataAction.h"
#include "MappingBase.h"
#include "FEProblem.h"
#include "RestartableDataIO.h"
#include "StochasticToolsApp.h"

registerMooseAction("StochasticToolsApp", LoadMappingDataAction, "load_mapping_data");

InputParameters
LoadMappingDataAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Blabla.");
  return params;
}

LoadMappingDataAction::LoadMappingDataAction(const InputParameters & params) : Action(params) {}

void
LoadMappingDataAction::act()
{
  std::vector<MappingBase *> objects;
  _app.theWarehouse().query().condition<AttribSystem>("MappingBase").queryInto(objects);
  for (auto mapping_ptr : objects)
  {
    if (mapping_ptr && mapping_ptr->isParamValid("filename"))
      load(*mapping_ptr);
  }
}

void
LoadMappingDataAction::load(const MappingBase & model)
{
  std::cerr << model.isParamValid("filename") << std::endl;
  std::cerr << model.getParam<FileName>("filename") << std::endl;
  // File to load
  const FileName & filename = model.getParam<FileName>("filename");

  // Create the object that will load in data
  RestartableDataIO data_io(_app);
  data_io.setErrorOnLoadWithDifferentNumberOfProcessors(false);
  data_io.setErrorOnLoadWithDifferentNumberOfThreads(false);

  // Read header
  bool pass = data_io.readRestartableDataHeaderFromFile(filename, false);
  if (!pass)
    model.paramError("filename", "The supplied file '", filename, "' failed to load.");

  // Get the data object that the loaded data will be applied
  const RestartableDataMap & meta_data = _app.getRestartableDataMap(model.modelMetaDataName());

  // Read the supplied file
  std::unordered_set<std::string> filter_names;
  data_io.readRestartableData(meta_data, filter_names);
}
