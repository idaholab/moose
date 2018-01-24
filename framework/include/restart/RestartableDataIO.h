//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RESTARTABLEDATAIO_H
#define RESTARTABLEDATAIO_H

// MOOSE includes
#include "DataIO.h"

// C++ includes
#include <sstream>
#include <string>
#include <list>

// Forward declarations
class Backup;
class RestartableDatas;
class RestartableDataValue;
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

  virtual ~RestartableDataIO() = default;

  /**
   * Write out the restartable data.
   */
  void writeRestartableData(std::string base_file_name,
                            const RestartableDatas & restartable_datas,
                            std::set<std::string> & _recoverable_data);

  /**
   * Read restartable data header to verify that we are restarting on the correct number of
   * processors and threads.
   */
  void readRestartableDataHeader(std::string base_file_name);

  /**
   * Read the restartable data.
   */
  void readRestartableData(const RestartableDatas & restartable_datas,
                           const std::set<std::string> & _recoverable_data);

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
  void
  serializeRestartableData(const std::map<std::string, RestartableDataValue *> & restartable_data,
                           std::ostream & stream);

  /**
   * Deserializes the data from the stream object.
   */
  void
  deserializeRestartableData(const std::map<std::string, RestartableDataValue *> & restartable_data,
                             std::istream & stream,
                             const std::set<std::string> & recoverable_data);

  /**
   * Serializes the data for the Systems in FEProblemBase
   */
  void serializeSystems(std::ostream & stream);

  /**
   * Deserializes the data for the Systems in FEProblemBase
   */
  void deserializeSystems(std::istream & stream);

  /// Reference to a FEProblemBase being restarted
  FEProblemBase & _fe_problem;

  /// A vector of file handles, one per thread
  std::vector<std::shared_ptr<std::ifstream>> _in_file_handles;
};

#endif /* RESTARTABLEDATAIO_H */
