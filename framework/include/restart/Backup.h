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
  /**
   * Constructor.
   *
   * The filename is optional and is only used for error reporting.
   */
  Backup(const std::string & filename = "");

  /**
   * Class used as a key to protect write operations to this object
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
   * Struct that describes data in a stream
   */
  struct DataInfo
  {
    /// The position in the stream at which this data is
    std::streampos position;
    /// The size of this data
    std::size_t size;
    /// The hash code for this data (typeid(T).hash_code())
    std::size_t type_hash_code;
    /// The type for this data
    std::string type;
  };

  /**
   * @returns Read-only access to the binary data for thread \p tid
   */
  const std::stringstream & data(const THREAD_ID tid) const { return _data[tid]; }
  /**
   * @returns Read-only access to the data info mapping for thread \p tid
   */
  const std::unordered_map<std::string, DataInfo> & dataInfo(const THREAD_ID tid) const
  {
    return _data_info_map[tid];
  }

  /**
   * @returns Write access to the binary data for thread \p tid
   *
   * This invalidates _data_info_map
   */
  std::stringstream & data(const THREAD_ID tid, const WriteKey);
  /**
   * @returns Write-only access to the data info mapping for thread \p tid
   */
  std::unordered_map<std::string, DataInfo> & dataInfo(const THREAD_ID tid, const WriteKey)
  {
    return _data_info_map[tid];
  }

  /**
   * @returns The filename associated with this backup (if any)
   */
  const std::string & filename() const { return _filename; }

private:
  /// Vector of streams for holding individual thread data for the simulation
  std::vector<std::stringstream> _data;
  /// Data mapping; stores metadata info for each data entry
  std::vector<std::unordered_map<std::string, DataInfo>> _data_info_map;
  /// The filename used to produce this backup (if any); used for error context
  const std::string _filename;
};

void dataStore(std::ostream & stream, Backup *& backup, void * context);
void dataLoad(std::istream & stream, Backup *& backup, void * context);
