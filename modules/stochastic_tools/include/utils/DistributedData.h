//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/parallel_object.h"
#include "libmesh/parallel.h"
#include "MooseTypes.h"

namespace StochasticTools
{

/**
 * Templated class that specifies a distributed storage of a vector of given
 * objects. It has a helper vector that contains global IDs for every stored item.
 */
template <typename T>
class DistributedData : public libMesh::ParallelObject
{
public:
  DistributedData(const libMesh::Parallel::Communicator & comm_in);

  /// Initialize the container with a given number of samples. this partitions
  /// the samples using linearPartitioning.
  void initializeContainer(unsigned int num_global_entries);

  /// Changing a sample with a global index if it is owned locally.
  void changeEntry(unsigned int glob_i, const T & sample);

  /// Adding a new sample locally with a global index.
  void addNewEntry(unsigned int glob_i, const T & sample);

  /// Closes the container meaning that no new samples can be added or the
  /// already existing samples canot be changed.
  void closeContainer() { _closed = true; };

  /// Checking of sample with global ID is locally owned ot not.
  bool hasGlobalEntry(unsigned int glob_i) const;

  /// Getting an itertor to the beginning of the local samples.
  typename std::vector<T>::iterator localEntryBegin() { return _local_entries.begin(); };

  /// Getting an iterator to the end of the locally owned samples.
  typename std::vector<T>::iterator localEntryEnd() { return _local_entries.end(); };

  /// Getting an iterator to the beginning of the locally owned sample IDs.
  typename std::vector<unsigned int>::iterator localEntryIDBegin()
  {
    return _local_entry_ids.begin();
  };

  /// Getting an iterator to the end of the locally owned sample IDs.
  typename std::vector<unsigned int>::iterator localEntryIDEnd() { return _local_entry_ids.end(); };

  /// Getting a sample using its global index.
  const T & getGlobalEntry(unsigned int glob_i) const;

  /// Getting a sample using its local index.
  const T & getLocalEntry(unsigned int loc_i) const;

  /// Getting all of the locally owned samples.
  const std::vector<T> & getLocalEntries() const { return _local_entries; };

  /// Getting the vector of sample IDs for locally owned samples.
  const std::vector<unsigned int> & getLocalEntryIDs() const { return _local_entry_ids; };

  /// Getting the number of global samples.
  unsigned int getNumberOfGlobalEntries() const;

  /// Getting the number of locally owned samples.
  unsigned int getNumberOfLocalEntries() const { return _n_local_entries; };

  /// Getting the local index of a global sample if locally owned.
  unsigned int getLocalIndex(unsigned int glob_i) const;

  /// Getting the global index of a locally owned sample.
  unsigned int getGlobalIndex(unsigned int loc_i) const;

protected:
  /// The vector where the samples are stored.
  std::vector<T> _local_entries;

  /// The vector where the global sample IDs are stored.
  std::vector<unsigned int> _local_entry_ids;

  /// Flag which shows if the container is closed or not.
  bool _closed;

  /// Number of local samples.
  unsigned int _n_local_entries;
};

} // StochasticTools namespace
