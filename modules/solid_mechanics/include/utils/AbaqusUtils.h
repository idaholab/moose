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

/**
 * Utility class to interface between the object oriented MOOSE app structure and
 * global Abaqus utility functions. The class encapsulates static members for
 * intermediate data storage.
 */
class AbaqusUtils
{
public:
  /**
   * Global storage for the simulation output directory, this will be set by any Abaqus class. MOOSE
   * will throw a warning if multiple objects try to set this to different values.
   */
  static void setInputFile(const std::string & input_file);
  static std::string getOutputDir() { return _output_dir; }
  static std::string getJobName() { return _job_name; }

  /**
   * Global storage for the MPI communicator, this will be set by any Abaqus class. MOOSE
   * will throw a warning if multiple objects try to set this to different values.
   */
  static void setCommunicator(const libMesh::Parallel::Communicator * communicator);
  static const libMesh::Parallel::Communicator * getCommunicator() { return _communicator; }

  /// get data vector iterator with error checking
  template <typename T>
  static typename std::map<int, std::vector<T>>::iterator
  getSMAIterator(std::map<int, std::vector<T>> & array, int id, const std::string & function);

  /// get thread local array
  template <typename T>
  static typename std::map<int, std::vector<T>> &
  getSMAThreadArray(std::vector<std::map<int, std::vector<T>>> & local_array,
                    const std::string & function);

  /// thread storage initialization
  static void smaInitialize();

  ///@{ Shared Memory Arrays
  static std::map<int, std::vector<int>> _sma_int_array;
  static std::map<int, std::vector<Real>> _sma_float_array;
  static std::vector<std::map<int, std::vector<int>>> _sma_local_int_array;
  static std::vector<std::map<int, std::vector<Real>>> _sma_local_float_array;
  ///@}

  ///@{ Mutex API
  static void mutexInit(std::size_t n);
  static void mutexLock(std::size_t n);
  static void mutexUnlock(std::size_t n);
  ///@}
private:
  static std::array<std::unique_ptr<Threads::spin_mutex>, 101> _mutex;
  static std::string _output_dir;
  static std::string _job_name;
  static const libMesh::Parallel::Communicator * _communicator;
};

template <typename T>
typename std::map<int, std::vector<T>>::iterator
AbaqusUtils::getSMAIterator(std::map<int, std::vector<T>> & array,
                            int id,
                            const std::string & function)
{
  auto it = array.find(id);
  if (it == array.end())
    mooseError("Invalid id ", id, " in ", function, ".");
  return it;
}

template <typename T>
typename std::map<int, std::vector<T>> &
AbaqusUtils::getSMAThreadArray(std::vector<std::map<int, std::vector<T>>> & local_array,
                               const std::string & function)
{
  ParallelUniqueId puid;
  if (puid.id >= local_array.size())
    mooseError("SMA storage not properly initialized in ", function, ".");
  return local_array[puid.id];
}
