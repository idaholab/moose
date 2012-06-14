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

#ifndef RESURRECTOR_H
#define RESURRECTOR_H

#include <string>
#include <list>
#include "XDAOutput.h"
#include "MaterialPropertyIO.h"
#include "UserDataIO.h"

class FEProblem;

/**
 * Class for doing restart.
 *
 * It takes care of writing and reading the restart files.
 */
class Resurrector
{
public:
  Resurrector(FEProblem & fe_problem);
  virtual ~Resurrector();

  /**
   * Set the file base name from which we will restart
   * @param file_name The file base name of a restart file
   */
  void setRestartFile(const std::string & file_base);
  /**
   * Are we restarting from a file
   * @return true if we are restarting, otherwise false
   */
  bool isOn() { return _restart; }
  /**
   * Perform a restart from a file
   */
  void restartFromFile();

  void restartStatefulMaterialProps();

  void restartUserData();

  /**
   * Set the number of restart file to store
   * @param num_files Number of files to keep around
   */
  void setNumRestartFiles(unsigned int num_files);
  /**
   * Write out a restart file and rotate already written ones
   */
  void write();

protected:
  /// Reference to a FEProblem being restarted
  FEProblem & _fe_problem;

  /// true if restarting from a file, otherwise false
  bool _restart;
  /// name of the file that we restart from
  std::string _restart_file_base;

  /// number of restart files to keep around
  unsigned int _num_restart_files;
  /// XDA writer
  XDAOutput _xda;
  /// Stateful material property output
  MaterialPropertyIO _mat;
  /// User Data IO
  UserDataIO _user_data;
  /// list of file names we keep around
  std::list<std::string> _restart_file_names;

  static const std::string MAT_PROP_EXT;
  static const std::string USER_DATA_EXT;
};

#endif /* RESURRECTOR_H */
