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
   * @return The mesh property \p data_name from the generator \p generator_name of type T.
   */
  template <typename T>
  const T & getMeshProperty(const std::string & data_name, const std::string & generator_name);
  /**
   * @return The mesh property \p data_name from the default generator.
   *
   * If this object is a generator, the default is this generator. Otherwise, it is
   * the final mesh generator.
   */
  template <typename T>
  const T & getMeshProperty(const std::string & data_name)
  {
    return getMeshProperty<T>(data_name, meshPropertyPrefix(data_name));
  }

  /**
   * @returns Whether or not a mesh meta-data exists with the name \p data_name
   * of type \p type from generator \p generator_name of
   */
  bool hasMeshProperty(const std::string & data_name,
                       const std::string & generator_name,
                       const std::type_info & type) const;
  /**
   * @returns Whether or not a mesh meta-data with name \p data_name of type T from
   * generator \p generator_name exists
   */
  template <typename T>
  bool hasMeshProperty(const std::string & data_name, const std::string & generator_name) const
  {
    return hasMeshProperty(data_name, generator_name, typeid(T));
  }

  /**
   * @returns Whether or not a mesh meta-data with the name \p data_name of type \p type
   * exists from the default generator.
   *
   * If this object is a generator, the default is this generator. Otherwise, it is
   * the final mesh generator.
   */
  bool hasMeshProperty(const std::string & data_name, const std::type_info & type) const
  {
    return hasMeshProperty(data_name, meshPropertyPrefix(data_name), type);
  }
  /**
   * @returns Whether or not a mesh meta-data with the name \p data_name of type
   * T exists from the default generator.
   *
   * If this object is a generator, the default is this generator. Otherwise, it is
   * the final mesh generator.
   */
  template <typename T>
  bool hasMeshProperty(const std::string & data_name) const
  {
    return hasMeshProperty(data_name, typeid(T));
  }

  /**
   * @returns The full name for mesh property data with name \p data_name
   * and from generator \p generator_name.
   */
  static std::string meshPropertyName(const std::string & data_name,
                                      const std::string & generator_name);

  /**
   * @returns The default mesh property name for mesh property data
   */
  std::string meshPropertyName(const std::string & data_name) const
  {
    return meshPropertyName(data_name, meshPropertyPrefix(data_name));
  }

private:
  /**
   * The default prefix to use for getting/seeing if mesh properties exist.
   *
   * For now, this is not supported except in MeshGenerators. In the future, we will
   * automate looking for mesh properties.
   */
  virtual std::string meshPropertyPrefix(const std::string & data_name) const;

  /// Helper for getting a mesh property
  const RestartableDataValue & getMeshPropertyInternal(const std::string & data_name,
                                                       const std::string & prefix) const;

  /**
   * Internal enum for whether or not a mesh property is had.
   *
   * Distinguishes between a property that is had and is loaded and one that is not.
   */
  enum HasMeshProperty
  {
    HAS_LOADED,
    HAS_NOT_LOADED,
    DOES_NOT_HAVE
  };

  MeshMetaDataInterface::HasMeshProperty hasMeshPropertyInternal(const std::string & data_name,
                                                                 const std::string & prefix,
                                                                 const std::type_info & type,
                                                                 const bool error_on_type) const;

  /// Reference to the application
  MooseApp & _meta_data_app;

  /// The MooseObject (if any); used for better error handling
  const MooseObject * const _meta_data_object;

  /**
   * Helper for forwarding a mooseError to an object's mooseError if it is available (said error
   * will provide more context: object name and type)
   */
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
  const auto has_property = hasMeshPropertyInternal(data_name, prefix, typeid(T), true);
  if (has_property == DOES_NOT_HAVE)
    mooseErrorInternal("Failed to get mesh property '", prefix, "/", data_name, "'");

  auto value = &getMeshPropertyInternal(data_name, prefix);
  mooseAssert(value->declared(), "Value has not been declared");
  const RestartableData<T> * T_value = dynamic_cast<const RestartableData<T> *>(value);
  mooseAssert(T_value, "Bad cast");
  return T_value->get();
}
