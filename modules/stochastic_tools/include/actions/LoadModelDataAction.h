//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "Attributes.h"
#include "RestartableDataReader.h"
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
  // Create the object that will load in data
  RestartableDataReader reader(
      _app, _app.getRestartableDataMap(object.modelMetaDataName()), _app.forceRestart());
  reader.setErrorOnLoadWithDifferentNumberOfProcessors(false);

  // Read the supplied file
  const std::string filename = object.getModelDataFileName();
  try
  {
    reader.setInput(filename);
    reader.restore();
  }
  catch (...)
  {
    paramError("filename", "The supplied file '", filename, "' failed to load.");
  }
}
