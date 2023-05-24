//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "Attributes.h"
#include "RestartableDataIO.h"
#include "RestartableModelInterface.h"

/**
 * Action for loading the model data for the mapping objects
 * @tparam The type of the object which needs to be loaded. This needs to be
 *         a derived class of `RestartableModelInterface` at the moment.
 */
template <typename T>
class LoadModelDataAction : public Action
{
public:
  static InputParameters validParams();

  LoadModelDataAction(const InputParameters & params) : Action(params) {}

  virtual void act() override;

private:
  /**
   * Load the necessary information for the given model
   * @param object Reference to the object whose data shall be loaded
   */
  void load(const T & object);
};

template <typename T>
InputParameters
LoadModelDataAction<T>::validParams()
{
  return Action::validParams();
}

template <typename T>
void
LoadModelDataAction<T>::act()
{
  static_assert(std::is_base_of<RestartableModelInterface, T>::value,
                "You must derive from RestartableModelInterface to use this action");

  // We fetch the mapping objects and then load the necessary data
  std::vector<T *> objects;
  static const auto attribute_name = T::validParams().getSystemAttributeName();

  _app.theWarehouse().query().template condition<AttribSystem>(attribute_name).queryInto(objects);
  for (auto object_ptr : objects)
    if (object_ptr->hasModelData())
      load(*object_ptr);
}

template <typename T>
void
LoadModelDataAction<T>::load(const T & object)
{
  // File to load
  const FileName & filename = object.getModelDataFileName();

  // Create the object that will load in data
  RestartableDataIO data_io(_app);
  data_io.setErrorOnLoadWithDifferentNumberOfProcessors(false);
  data_io.setErrorOnLoadWithDifferentNumberOfThreads(false);

  // Read header
  bool pass = data_io.readRestartableDataHeaderFromFile(filename, false);
  if (!pass)
    object.paramError("filename", "The supplied file '", filename, "' failed to load.");

  // Get the data object that the loaded data will be applied
  const RestartableDataMap & meta_data = _app.getRestartableDataMap(object.modelMetaDataName());

  // Read the supplied file
  std::unordered_set<std::string> filter_names;
  data_io.readRestartableData(meta_data, filter_names);
}
