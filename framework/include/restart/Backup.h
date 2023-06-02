//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

#include <sstream>
#include <vector>
#include <unordered_map>

class RestartableDataIO;

/**
 * Helper class to hold streams for Backup and Restore operations.
 */
class Backup
{
public:
  Backup();

  /**
   * Class used as a key to protect write operations to this object.
   */
  class WriteKey
  {
    friend class RestartableDataIO;
    friend void dataStore(std::ostream & stream, Backup *& backup, void * context);
    friend void dataLoad(std::istream & stream, Backup *& backup, void * context);
    WriteKey() {}
    WriteKey(const WriteKey &) {}
  };

  /**
   * Helper struct for a restartable data entry.
   */
  struct DataEntry
  {
    /// The position in the stream at which this data is
    std::streampos position;
    /// The size of this data
    std::size_t size;
    /// The hash code for this data (typeid(T).hash_code())
    std::size_t type_hash_code;
    /// Whether or not this data has been loaded yet
    bool loaded = false;
  };

  /**
   * @returns Read-only access to the binary restartable data for thread \p tid
   */
  const std::stringstream & restartableData(const THREAD_ID tid) const
  {
    return _restartable_data[tid];
  }
  /**
   * @returns Read-only access to the restartable data mapping for thread \p tid
   */
  const std::unordered_map<std::string, DataEntry> & restartableDataMap(const THREAD_ID tid) const
  {
    return _restartable_data_map[tid];
  }

  /**
   * @returns Write access to the binary restartable data for thread \p tid
   *
   * This invalidates the map in _restartable_data_map
   */
  std::stringstream & restartableData(const THREAD_ID tid, const WriteKey);
  /**
   * @returns Write-only access to the restartable data mapping for thread \p tid
   */
  std::unordered_map<std::string, DataEntry> & restartableDataMap(const THREAD_ID tid,
                                                                  const WriteKey)
  {
    return _restartable_data_map[tid];
  }

private:
  /// Vector of streams for holding individual thread data for the simulation.
  std::vector<std::stringstream> _restartable_data;
  /// Restarable data mapping; stores metadata for each data entry
  std::vector<std::unordered_map<std::string, DataEntry>> _restartable_data_map;
};

void dataStore(std::ostream & stream, Backup *& backup, void * context);
void dataLoad(std::istream & stream, Backup *& backup, void * context);
