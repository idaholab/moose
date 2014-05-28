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

#include "Moose.h"

#include <string>
#include <list>

class RestartableDatas;

class FEProblem;

/**
 * Class for doing restart.
 *
 * It takes care of writing and reading the restart files.
 */
class RestartableDataIO
{
public:
  RestartableDataIO(FEProblem & fe_problem);

  virtual ~RestartableDataIO();

  /**
   * Write out the restartable data.
   */
  void writeRestartableData(std::string base_file_name, const RestartableDatas & restartable_datas, std::set<std::string> & _recoverable_data);

  /**
   * Read restartable data header to verify that we are restarting on the correct number of processors and threads.
   */
  void readRestartableDataHeader(std::string base_file_name);

  /**
   * Read the restartable data.
   */
  void readRestartableData(RestartableDatas & restartable_datas, std::set<std::string> & _recoverable_data);

private:
  /// Reference to a FEProblem being restarted
  FEProblem & _fe_problem;

  /// A vector of file handles, one per thread
  std::vector<std::ifstream *> _in_file_handles;
};

#endif /* RESTARTABLEDATAIO_H */
