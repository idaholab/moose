//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LoadSurrogateDataAction.h"
#include "SurrogateModel.h"
#include "FEProblem.h"
#include "RestartableDataIO.h"
#include "StochasticToolsApp.h"

registerMooseAction("StochasticToolsApp", LoadSurrogateDataAction, "load_surrogate_data");

InputParameters
LoadSurrogateDataAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Calls load method on SurrogateModel objects contained within the "
                             "`[Surrogates]` input block, if a filename is given.");
  return params;
}

LoadSurrogateDataAction::LoadSurrogateDataAction(InputParameters params) : Action(params) {}

void
LoadSurrogateDataAction::act()
{
  std::vector<SurrogateModel *> models;
  _app.theWarehouse().query().queryInto(models);
  for (auto model_ptr : models)
    if (model_ptr->isParamValid("filename"))
      load(*model_ptr);
}

void
LoadSurrogateDataAction::load(SurrogateModel & model)
{
  // File to load
  const FileName & filename = model.getParamTempl<FileName>("filename");

  // Create the object that will load in data
  RestartableDataIO data_io(_app);
  bool pass = data_io.readRestartableDataHeaderFromFile(filename, false);
  if (!pass)
    paramError("filename", "The supplied file '", filename, "' failed to load.");

  // Get the data object that the loaded data will be applied
  const RestartableDataMap & meta_data = _app.getRestartableDataMap(model.name());

  // Check the class name being loaded matches the class name doing the loading
  auto data_ptr = static_cast<RestartableData<std::string> *>(meta_data.at("type").value.get());
  const std::string & meta_name = data_ptr->get();
  const std::string & class_name = model.getParamTempl<std::string>("_type");
  if (meta_name != class_name)
  {
    model.paramError("filename",
                     "The supplied file '",
                     filename,
                     "' contains model data for type '",
                     meta_name,
                     "' but the object is of type '",
                     class_name,
                     "'.");
  }

  // Read the supplied file
  std::unordered_set<std::string> filter_names;
  data_io.readRestartableData(meta_data, filter_names);
}
