//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DistributedData.h"
#include "MooseUtils.h"
#include "MooseError.h"
#include "libmesh/dense_vector.h"

namespace StochasticTools
{

template <typename T>
DistributedData<T>::DistributedData(const libMesh::Parallel::Communicator & comm_in)
  : libMesh::ParallelObject(comm_in), _closed(false), _n_local_entries(0)
{
}

template <typename T>
void
DistributedData<T>::initializeContainer(unsigned int n_global_entries)
{
  // This function can be used when a linear partitioning is required and the
  // number of global samples is known in advance. We must temporarily
  // use dof_id_type for the last three args (pass by reference),
  // but will cast back to unsigned int later
  dof_id_type local_entry_begin;
  dof_id_type local_entry_end;
  dof_id_type n_local_entries;

  MooseUtils::linearPartitionItems(n_global_entries,
                                   n_processors(),
                                   processor_id(),
                                   n_local_entries,
                                   local_entry_begin,
                                   local_entry_end);
  _n_local_entries = n_local_entries;
  _local_entries.resize(_n_local_entries);
  _local_entry_ids.resize(_n_local_entries);

  // Filling the sample ID vector, leaving the elements of the sample vector
  // with the default constructor.
  for (unsigned int entry_i = local_entry_begin; entry_i < (unsigned int)local_entry_end; ++entry_i)
  {
    _local_entry_ids[entry_i] = entry_i;
  }
}

template <typename T>
void
DistributedData<T>::addNewEntry(unsigned int glob_i, const T & entry)
{
  auto it = std::find(_local_entry_ids.begin(), _local_entry_ids.end(), glob_i);
  if (it != _local_entry_ids.end())
    ::mooseError("Local object ID (", glob_i, ") already exists!");
  if (_closed)
    ::mooseError("DistributeData has already been closed, cannot add new elements!");

  _local_entries.push_back(entry);
  _local_entry_ids.push_back(glob_i);
  _n_local_entries += 1;
}

template <typename T>
void
DistributedData<T>::changeEntry(unsigned int glob_i, const T & entry)
{
  auto it = std::find(_local_entry_ids.begin(), _local_entry_ids.end(), glob_i);
  if (it == _local_entry_ids.end())
    ::mooseError("Local object ID (", glob_i, ") does not exists!");
  if (_closed)
    ::mooseError("DistributeData has already been closed, cannot change elements!");

  _local_entries[std::distance(_local_entry_ids.begin(), it)] = entry;
}

template <typename T>
const T &
DistributedData<T>::getGlobalEntry(unsigned int glob_i) const
{
  auto it = std::find(_local_entry_ids.begin(), _local_entry_ids.end(), glob_i);
  if (it == _local_entry_ids.end())
    ::mooseError("Local object ID (", glob_i, ") does not exists!");

  return _local_entries[std::distance(_local_entry_ids.begin(), it)];
}

template <typename T>
const T &
DistributedData<T>::getLocalEntry(unsigned int loc_i) const
{
  if (loc_i > _n_local_entries - 1)
    ::mooseError("The requested local index (",
                 loc_i,
                 ") is greater than the size (",
                 _n_local_entries,
                 ") of the locally stored vector!");

  return _local_entries[loc_i];
}

template <typename T>
unsigned int
DistributedData<T>::getNumberOfGlobalEntries() const
{
  unsigned int val = _n_local_entries;
  _communicator.sum(val);
  return val;
}

template <typename T>
bool
DistributedData<T>::hasGlobalEntry(unsigned int glob_i) const
{
  const auto it = std::find(_local_entry_ids.begin(), _local_entry_ids.end(), glob_i);
  if (it != _local_entry_ids.end())
    return true;

  return false;
}

template <typename T>
unsigned int
DistributedData<T>::getLocalIndex(unsigned int glob_i) const
{
  const auto it = std::find(_local_entry_ids.begin(), _local_entry_ids.end(), glob_i);
  if (it == _local_entry_ids.end())
    ::mooseError("Local object ID (", glob_i, ") does not exists!");

  return std::distance(_local_entry_ids.begin(), it);
}

template <typename T>
unsigned int
DistributedData<T>::getGlobalIndex(unsigned int loc_i) const
{
  if (loc_i > _n_local_entries - 1)
    ::mooseError("The requested local index (",
                 loc_i,
                 ") is greater than the size (",
                 _n_local_entries,
                 ") of the locally stored vector!");

  return _local_entry_ids[loc_i];
}

// Explicit instantiation of types that are necessary.
template class DistributedData<DenseVector<Real>>;
template class DistributedData<std::shared_ptr<DenseVector<Real>>>;
template class DistributedData<std::vector<Real>>;

} // StochasticTools namespace
