//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "MooseError.h"
#include "InputParameters.h"
#include "Restartable.h"

#include <string>

class MooseApp;
class MeshGenerator;

/**
 * The Interface used to retrieve mesh meta data (attributes) set by the MeshGenerator system.
 * MOOSE objects should avoid retrieving and casting MeshGenerator objects since they are not
 * re-created during a recover operation. This data is read from files early during the simulation
 * setup an d can be used to make decisions about how to setup the rest of the problem.
 */
class MeshMetaDataInterface : private Restartable
{
public:
  /// The suffix appended when writing the restartable data file.
  static constexpr auto FILE_SUFFIX = "_mesh";

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
   * Interface for MeshGenerators: This interface initializes the Restartable interface with the
   * generator name for prefixing purposes.
   */
  MeshMetaDataInterface(const MeshGenerator * mesh_gen_object);

  /**
   * This class constructor is used for non-Moose-based objects like interfaces. A name for the
   * storage as well as a system name must be passed in along with the thread ID explicitly.
   */
  MeshMetaDataInterface(MooseApp & moose_app);

  const MeshGeneratorName & getMeshGeneratorPrefix(const std::string & name);

  /**
   * Method for retrieving a property with the given type and name exists in the mesh meta-data
   * store. This method will throw an error if the property does not exist.
   */
  template <typename T>
  const T & getMeshProperty(const std::string & data_name, const std::string & prefix)
  {
    return declareRestartableDataWithPrefixOverrideAndContext<T>(data_name, prefix, nullptr, true);
  }

private:
  /**
   * MeshGenerators are the only objects that may write to the mesh meta-data store. We use a friend
   * declaration to grant access to the MeshGenerators to access the meta-data.
   */
  friend class MeshGenerator;
  template <typename T>
  T & declareMeshPropertyInternal(const std::string & data_name)
  {
    return declareRestartableDataWithContext<T>(data_name, nullptr, false);
  }

  template <typename T>
  T & declareMeshPropertyInternal(const std::string & data_name, const T & value)
  {
    return declareRestartableDataWithContext<T>(data_name, value, nullptr);
  }
};
