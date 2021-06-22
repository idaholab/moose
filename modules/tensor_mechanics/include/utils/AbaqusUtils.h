//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParallelUniqueId.h"

extern "C" void getnumcpus_(int * num);
extern "C" void getrank_(int * rank);

extern "C" int
getcommunicator_()
{
  mooseWarning("GETCOMMUNICATOR() is not implemented");
  return 0;
}

extern "C" int getnumthreads_();
extern "C" int get_thread_id_();

extern "C" int getoutdir_();

class AbaqusUtils
{
public:
  /**
   * Global storage for the simulation output directory, this will be set by any Abaqus class. MOOSE
   * will throw a warning if multiple objects try to set this to different values.
   */
  static void setOutputDir(const std::string & output_dir);
  static std::string getOutputDir() { return _output_dir; }

  /**
   * Global storage for the MPI communicator, this will be set by any Abaqus class. MOOSE
   * will throw a warning if multiple objects try to set this to different values.
   */
  static void setCommunicator(const libMesh::Parallel::Communicator * communicator);
  static const libMesh::Parallel::Communicator * getCommunicator() { return _communicator; }

private:
  static std::string _output_dir;
  static const libMesh::Parallel::Communicator * _communicator;
};
