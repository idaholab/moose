/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef RESTARTABLEDATAIO_H
#define RESTARTABLEDATAIO_H

// MOOSE includes
#include "DataIO.h"
#include "Backup.h"

// C++ includes
#include <sstream>
#include <string>
#include <list>

// Forward declarations
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
