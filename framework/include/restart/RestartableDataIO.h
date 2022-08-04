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

// C++ includes
#include <sstream>
#include <string>
#include <list>

// Forward declarations
class Backup;
class FEProblemBase;

/**
 * Class for doing restart.
 *
 * It takes care of writing and reading the restart files.
 */
class RestartableDataIO : public PerfGraphInterface
{
public:
  RestartableDataIO(FEProblemBase & fe_problem);

  RestartableDataIO(MooseApp & moose_app, FEProblemBase * fe_problem_ptr = nullptr);

  virtual ~RestartableDataIO() = default;

  /**
   * Create a Backup for the current system.
   */
  std::shared_ptr<Backup> createBackup();

  /**
   * Restore a Backup for the current system.
   */
  void restoreBackup(std::shared_ptr<Backup> backup, bool for_restart = false);

  ///@{
  /*
   * Enable/Disable errors to allow meta data to be created/loaded on different number or processors
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
   * Serializes the data into the stream object.
   */
  void serializeRestartableData(const RestartableDataMap & restartable_data, std::ostream & stream);

  /**
   * Deserializes the data from the stream object.
   */
  void deserializeRestartableData(const RestartableDataMap & restartable_data,
                                  std::istream & stream,
                                  const DataNames & filter_names);

  /**
   * Serializes the data for the Systems in FEProblemBase
   */
  void serializeSystems(std::ostream & stream);

  /**
   * Deserializes the data for the Systems in FEProblemBase
   */
  void deserializeSystems(std::istream & stream);

  /// A reference to the MooseApp object for retrieving restartable data stores and filters
  MooseApp & _moose_app;

  /// Pointer to the FEProblemBase when serializing/deserializing system data
  FEProblemBase * _fe_problem_ptr;

  static constexpr auto RESTARTABLE_DATA_EXT = ".rd";
  static constexpr auto CURRENT_BACKUP_FILE_VERSION = 3;

  /// Error check controls
  bool _error_on_different_number_of_processors = true;
  bool _error_on_different_number_of_threads = true;
};
