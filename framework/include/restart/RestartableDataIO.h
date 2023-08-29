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
#include <filesystem>

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
   * @return The common extension for restartable data folders
   */
  static const std::string & getRestartableExt();
  /**
   * @return The filename for the restartable data
   */
  static const std::string & restartableDataFile();
  /**
   * @return The filename for the restartable header
   */
  static const std::string & restartableHeaderFile();

  /**
   *  @return The path to the restartable data folder with base \p folder_base
   *
   * This just appends .rd
   */
  static std::filesystem::path restartableDataFolder(const std::filesystem::path & folder_base);
  /**
   * @return The path to the restartable data file with base \p folder_base
   *
   * Does not append .rd to the folder base
   */
  static std::filesystem::path restartableDataFile(const std::filesystem::path & folder_base);
  /**
   * @return The path to the restartable header file with base \p folder_base
   *
   * Does not append .rd to the folder base
   */
  static std::filesystem::path restartableHeaderFile(const std::filesystem::path & folder_base);

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

  /// The current version for the backup file
  static const unsigned int CURRENT_BACKUP_FILE_VERSION;
  /// The type to used for comparing hash codes (sanity checking)
  typedef int COMPARE_HASH_CODE_TYPE;
};
