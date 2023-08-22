//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PerfGraphInterface.h"

#include "libmesh/parallel_object.h"

#include <vector>
#include <variant>

class RestartableDataMap;

/**
 * Class for doing restart.
 *
 * It takes care of writing and reading the restart files.
 */
class RestartableDataIO : public PerfGraphInterface, public libMesh::ParallelObject
{
public:
  RestartableDataIO(MooseApp & app, RestartableDataMap & data);
  RestartableDataIO(MooseApp & app, std::vector<RestartableDataMap> & data);

  /**
   * @return The common extension for restartable data
   */
  static const std::string & getRestartableDataExt() { return RESTARTABLE_DATA_EXT; }

protected:
  /**
   * @return The restartable data for thread \p tid
   *
   * This exists so that we can support threaded and non-threaded data in _data
   */
  RestartableDataMap & currentData(const THREAD_ID tid);
  /**
   * @return The size of _data
   */
  std::size_t dataSize() const;

  /// The data we wish to act on
  /// This is a variant so that we can act on threaded and non-threaded data
  const std::variant<RestartableDataMap *, std::vector<RestartableDataMap> *> _data;

  /// The common extension for restartable data
  static const std::string RESTARTABLE_DATA_EXT;
  /// The current version for the backup file
  static const unsigned int CURRENT_BACKUP_FILE_VERSION;
  /// The type to used for comparing hash codes (sanity checking)
  typedef int COMPARE_HASH_CODE_TYPE;
};
