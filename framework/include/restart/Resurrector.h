//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RESURRECTOR_H
#define RESURRECTOR_H

// MOOSE includes
#include "RestartableDataIO.h"

// C++ includes
#include <string>

// Forward declarations
class FEProblemBase;

/**
 * Class for doing restart.
 *
 * It takes care of writing and reading the restart files.
 */
class Resurrector
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
   * Set the file extension from which we will restart libMesh
   * equation systems.  The default suffix is "xdr".
   * @param file_ext The file extension of a restart file
   */
  void setRestartSuffix(const std::string & file_ext);

  /**
   * Perform a restart from a file
   */
  void restartFromFile();

  void restartRestartableData();

protected:
  /// Reference to a FEProblemBase being restarted
  FEProblemBase & _fe_problem;

  /// name of the file that we restart from
  std::string _restart_file_base;

  /// name of the file extension that we restart from
  std::string _restart_file_suffix;

  /// Restartable Data
  RestartableDataIO _restartable;

  static const std::string MAT_PROP_EXT;
  static const std::string RESTARTABLE_DATA_EXT;
};

#endif /* RESURRECTOR_H */
