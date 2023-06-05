//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "DataIO.h"
#include "RestartableData.h"
#include "PerfGraphInterface.h"
#include "Backup.h"

// C++ includes
#include <sstream>
#include <string>
#include <list>
#include <typeinfo>

// Forward declarations
class RestartableDataMap;

/**
 * Class for doing restart.
 *
 * It takes care of writing and reading the restart files.
 */
class RestartableDataIO : public PerfGraphInterface
{
public:
  RestartableDataIO(MooseApp & moose_app);

  /**
   * Sets the backup for use in restoreBackup and restoreData.
   */
  void setBackup(std::shared_ptr<Backup> backup);
  /**
   * @returns Whether or not a backup has been set via setBackup.
   */
  bool hasBackup() const { return _backup != nullptr; }
  /**
   * Clears the backup that has been set by setBackup.
   */
  void clearBackup();

  /**
   * Create a Backup for the current system.
   */
  std::shared_ptr<Backup> createBackup();

  static const std::string & getRestartableDataExt() { return RESTARTABLE_DATA_EXT; }

  /**
   * Restore a Backup for the current system.
   *
   * Requires that a backup has been set via setBackup.
   */
  void restoreBackup(bool for_restart = false);

  /**
   * Restores the data with the name \p name on thread \p tid.
   *
   * Requires that a backup has been set via setBackup.
   */
  void restoreData(const std::string & name, const THREAD_ID tid);

  /**
   * Writes restartable data into a binary file.
   *
   * This should only be used for the arbitrary restartable data maps (such as mesh meta data)
   * The normal restart data is encaapsulated in the backup
   *
   * @param file_name The name of the file to read from
   * @restartable_data The data to be written
   */
  void writeRestartableData(const std::string & file_name,
                            const RestartableDataMap & restartable_data);

  /**
   * Reads restartable data from a binary file.
   *
   * This should only be used for the arbitrary restartable data maps (such as mesh meta data)
   * The normal restart data is encaapsulated in the backup
   *
   * @param file_name The file name to read from
   * @param restartable_data The data structure to read into
   * @param filter_names The names of data to NOT read (used to filter out recover only variables
   * during restart)
   */
  void readRestartableData(const std::string & file_name,
                           RestartableDataMap & restartable_data,
                           const DataNames & filter_names = {});

  ///@{
  /*
   * Enable/Disable errors to allow meta data to be created/loaded on different number or
   * processors
   *
   * See LoadSurrogateModelAction for use case
   */
  void setErrorOnLoadWithDifferentNumberOfProcessors(bool value)
  {
    _error_on_different_number_of_processors = value;
  }

  void setErrorOnLoadWithDifferentNumberOfThreads(bool value)
  {
    _error_on_different_number_of_threads = value;
  }
  ///@}

private:
  /**
   * Reads the backup headers for the Backup on thread \p tid, and
   * stores them into the data maps within the Backup for future use.
   */
  void readBackup(const THREAD_ID tid);

  /**
   * @returns The restartable data headers from the stream \p stream.
   */
  std::unordered_map<std::string, Backup::DataInfo>
  readRestartableData(std::istream & stream, const std::string & filename = "") const;

  /**
   * Serializes the data into the stream object.
   */
  void serializeRestartableData(const RestartableDataMap & restartable_data, std::ostream & stream);

  /**
   * Deserializes the data from the stream object.
   */
  void deserializeRestartableData(RestartableDataMap & restartable_data,
                                  std::istream & stream,
                                  std::unordered_map<std::string, Backup::DataInfo> & data_map,
                                  const DataNames & filter_names);
  void deserializeRestartableDataValue(RestartableDataValue & value,
                                       Backup::DataInfo & data_entry,
                                       std::istream & stream);

  /// A reference to the MooseApp object for retrieving restartable data stores and filters
  MooseApp & _moose_app;

  /// The Backup object for use in restoration
  std::shared_ptr<Backup> _backup;

  static const std::string RESTARTABLE_DATA_EXT;
  static const unsigned int CURRENT_BACKUP_FILE_VERSION;
  typedef int COMPARE_HASH_CODE_TYPE;

  /// Error check controls
  bool _error_on_different_number_of_processors = true;
  bool _error_on_different_number_of_threads = true;
};
