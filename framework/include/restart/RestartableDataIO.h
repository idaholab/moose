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
class RestartableDataIO
{
public:
  RestartableDataIO(FEProblemBase & fe_problem);

  RestartableDataIO(MooseApp & moose_app);

  virtual ~RestartableDataIO() = default;

  /**
   * Write out the restartable data.
   */
  void writeRestartableData(const std::string & base_file_name,
                            const RestartableDataMaps & restartable_datas,
                            const DataNames & _recoverable_data_names);

  /**
   * Read restartable data header to verify that we are restarting on the correct number of
   * processors and threads.
   */
  void readRestartableDataHeader(const std::string & base_file_name);

  /**
   * Read the restartable data.
   */
  void readRestartableData(const RestartableDataMaps & restartable_datas,
                           const DataNames & _recoverable_data_names);

  /**
   * Create a Backup for the current system.
   */
  std::shared_ptr<Backup> createBackup();

  /**
   * Restore a Backup for the current system.
   */
  void restoreBackup(std::shared_ptr<Backup> backup, bool for_restart = false);

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
                                  const DataNames & filter_names,
                                  bool exclude = true);

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
  FEProblemBase * _fe_problem;

  /// A vector of file handles, one per thread
  std::vector<std::shared_ptr<std::ifstream>> _in_file_handles;
};
