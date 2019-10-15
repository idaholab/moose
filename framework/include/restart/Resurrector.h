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
#include "RestartableDataIO.h"
#include "PerfGraphInterface.h"

// C++ includes
#include <string>

// Forward declarations
class FEProblemBase;

/**
 * Class for doing restart.
 *
 * It takes care of writing and reading the restart files.
 */
class Resurrector : public PerfGraphInterface
{
public:
  Resurrector(FEProblemBase & fe_problem);
  virtual ~Resurrector() = default;

  /**
   * Set the file base name from which we will restart
   * @param file_base The file base name of a restart file
   */
  void setRestartFile(const std::string & file_base);

  /**
   * Tell the Resurrector to use Ascii formatted data instead of the default binary format.
   */
  void useAsciiExtension();

  /**
   * Perform a restart of the libMesh Equation Systems from a file.
   */
  void restartEquationSystemsObject();

  /**
   * Perform a restart of the MOOSE user-defined restartable data from a file.
   */
  void restartRestartableData();

protected:
  /// Reference to a FEProblemBase being restarted
  FEProblemBase & _fe_problem;

  /// name of the file that we restart from
  std::string _restart_file_base;

  /// name of the file extension that we restart from
  bool _use_binary_ext;

  /// Restartable Data
  RestartableDataIO _restartable;

  /// Timers
  const PerfID _restart_es_timer;
  const PerfID _restart_restartable_data_timer;

  static const std::string RESTARTABLE_DATA_EXT;
  static const std::string RESTARTABLE_MESH_DATA_EXT;
  static const std::string ES_BINARY_EXT;
  static const std::string ES_ASCII_EXT;
};
