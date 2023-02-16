//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseError.h"
#include "RestartableData.h"
#include "MooseObject.h"

#include <string>

class MeshGenerator;

/**
 * The Interface used to retrieve mesh meta data (attributes) set by the MeshGenerator system.
 * MOOSE objects should avoid retrieving and casting MeshGenerator objects since they are not
 * re-created during a recover operation. This data is read from files early during the simulation
 * setup an d can be used to make decisions about how to setup the rest of the problem.
 */
class MeshMetaDataInterface
{
public:
  /// The system name used when initializing the Restartable interface
  static constexpr auto SYSTEM = "MeshMetaData";

  /// The data name used when initializing the Restartable interface for non-MeshGenerator objects.
  static constexpr auto NAME = "<empty>";

protected:
  /**
   * Interface for all objects reading MeshMetaData. The name of the object gets a bogus prefix,
   * which is not intended to be used for storing data. Instead this value is overridden during
   * retrieval.
   */
  MeshMetaDataInterface(const MooseObject * moose_object);

  /**
   * This class constructor is used for non-Moose-based objects like interfaces. A name for the
   * storage as well as a system name must be passed in along with the thread ID explicitly.
   */
  MeshMetaDataInterface(MooseApp & moose_app);

  /**
   * Method for retrieving a property with the given type and name exists in the mesh meta-data
   * store. This method will throw an error if the property does not exist.
   */
  template <typename T>
  const T & getMeshProperty(const std::string & data_name, const std::string & prefix);
  template <typename T>
  const T & getMeshProperty(const std::string & data_name)
  {
    return getMeshProperty<T>(data_name, defaultMeshPropertyPrefix());
  }

  /**
   * @returns Whether or not a mesh meta-data exists.
   */
  bool hasMeshProperty(const std::string & data_name, const std::string & prefix) const;
  /**
   * @returns Whether or not a mesh meta-data exists with the given type.
   */
  template <typename T>
  bool hasMeshProperty(const std::string & data_name, const std::string & prefix) const;

  bool hasMeshProperty(const std::string & data_name) const
  {
    return hasMeshProperty(data_name, defaultMeshPropertyPrefix());
  }
  template <typename T>
  bool hasMeshProperty(const std::string & data_name) const
  {
    return hasMeshProperty<T>(data_name, defaultMeshPropertyPrefix());
  }

  /**
   * @returns The full name for mesh property data.
   */
  static std::string meshPropertyName(const std::string & data_name, const std::string & prefix);

  /**
   * @returns The default mesh property name for mesh property data
   */
  std::string meshPropertyName(const std::string & data_name) const
  {
    return meshPropertyName(data_name, defaultMeshPropertyPrefix());
  }

private:
  /**
   * The default prefix to use for meshPropertyName(), the name
   * of the final MeshGenerator
   */
  virtual std::string defaultMeshPropertyPrefix() const;

  /// Helper function for actually registering the restartable data.
  RestartableDataValue & registerMetaDataOnApp(const std::string & name,
                                               std::unique_ptr<RestartableDataValue> data);

  /// Helper for getting a mesh property
  const RestartableDataValue & getMeshPropertyInternal(const std::string & data_name,
                                                       const std::string & prefix) const;

  /// Reference to the application
  MooseApp & _meta_data_app;

  /// The MooseObject (if any); used for better error handling
  const MooseObject * const _meta_data_object;

  template <typename... Args>
  [[noreturn]] void mooseErrorInternal(Args &&... args) const
  {
    if (_meta_data_object)
      _meta_data_object->mooseError(std::forward<Args>(args)...);
    mooseError(std::forward<Args>(args)...);
  }
};

template <typename T>
const T &
MeshMetaDataInterface::getMeshProperty(const std::string & data_name, const std::string & prefix)

{
  const auto full_name = meshPropertyName(prefix, data_name);
  auto data_ptr = std::make_unique<RestartableData<T>>(full_name, nullptr);

  // Here we will create the RestartableData even though we may not use this instance.
  // If it's already in use, the App will return a reference to the existing instance and we'll
  // return that one instead. We might refactor this to have the app create the RestartableData
  // at a later date.
  auto & restartable_data_ref =
      static_cast<RestartableData<T> &>(registerMetaDataOnApp(full_name, std::move(data_ptr)));

template <typename T>
bool
MeshMetaDataInterface::hasMeshProperty(const std::string & data_name,
                                       const std::string & prefix) const
{
  if (!hasMeshProperty(data_name, prefix))
    return false;
  const auto & value = getMeshPropertyInternal(data_name, prefix);
  return dynamic_cast<const RestartableData<T> *>(&value) != nullptr;
}
