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
   * Tell the Resurrector to use Ascii formatted data instead of the default binary format.
   */
  void useAsciiExtension();

  /**
   * Perform a restart of the libMesh Equation Systems from a file.
   */
  void restartEquationSystemsObject();

  /**
   * Write out the restartable data.
   */
  void writeRestartableData(const std::string & base_file_name,
                            const RestartableDataMap & restartable_datas);

  void writeRestartableDataPerProc(const std::string & base_file_name,
                                   const RestartableDataMaps & restartable_data);

  /**
   * Read restartable data header to verify that we are restarting on the correct number of
   * processors and threads.
   */
  bool readRestartableDataHeader(bool per_proc_id, const std::string & suffix = "");
  bool readRestartableDataHeaderFromFile(const std::string & recover_file, bool per_proc_id);

  /**
   * Read the restartable data.
   */
  void readRestartableData(const RestartableDataMaps & restartable_datas,
                           const DataNames & _recoverable_data_names);
  void readRestartableData(const RestartableDataMap & restartable_data,
                           const DataNames & _recoverable_data_names,
                           unsigned int tid = 0);
  /**
   * Create a Backup for the current system.
   */
  std::shared_ptr<Backup> createBackup();

  /**
   * Restore a Backup for the current system.
   */
  void restoreBackup(std::shared_ptr<Backup> backup, bool for_restart = false);

  std::string getESFileExtension(bool is_binary) const
  {
    return is_binary ? ES_BINARY_EXT : ES_ASCII_EXT;
  }

  std::string getRestartableDataExt() const { return RESTARTABLE_DATA_EXT; }

  ///@{
  /*
   * Enable/Disable errors to allow meta data to be created/loaded on different number or processors
   *
   * See LoadSurrogateModelAction for use case
   */
  void setErrorOnLoadWithDifferentNumberOfProcessors(bool value);
  void setErrorOnLoadWithDifferentNumberOfThreads(bool value);
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

  /// Boolean to indicate that the restartable data header has been read
  bool _is_header_read;

  /// name of the file extension that we restart from
  bool _use_binary_ext;

  /// A vector of file handles, one per thread
  std::vector<std::shared_ptr<std::ifstream>> _in_file_handles;

  static constexpr auto RESTARTABLE_DATA_EXT = ".rd";
  static constexpr auto ES_BINARY_EXT = ".xdr";
  static constexpr auto ES_ASCII_EXT = ".xda";

  /// Error check controls
  bool _error_on_different_number_of_processors = true;
  bool _error_on_different_number_of_threads = true;
};
