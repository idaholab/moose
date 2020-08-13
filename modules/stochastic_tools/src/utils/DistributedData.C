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
DistributedData<T>::initializeContainer(dof_id_type n_global_entries)
{
  // This function can be used when a linear partitioning is required and the
  // number of global samples is known in advance.
  dof_id_type local_entry_begin;
  dof_id_type local_entry_end;
  MooseUtils::linearPartitionItems(n_global_entries,
                                   n_processors(),
                                   processor_id(),
                                   _n_local_entries,
                                   local_entry_begin,
                                   local_entry_end);

  _local_entries.resize(_n_local_entries);
  _local_entry_ids.resize(_n_local_entries);

  // Filling the sample ID vector, leaving the elements of the sample vector
  // with the default constructor.
  for (dof_id_type entry_i = local_entry_begin; entry_i < local_entry_end; ++entry_i)
  {
    _local_entry_ids[entry_i] = entry_i;
  }
}

template <typename T>
void
DistributedData<T>::reassignGlobalIndices()
{
  std::vector<dof_id_type> tmp_local_entries(n_processors());
  for (dof_id_type proc_i = 0; proc_i < n_processors(); ++proc_i)
  {
    if (proc_i == processor_id())
    {
      tmp_local_entries[proc_i] = _n_local_entries;
    }
  }
  _communicator.sum(tmp_local_entries);

  dof_id_type sum = 0;
  std::vector<dof_id_type> cum_local_entries(n_processors());
  for (dof_id_type proc_i = 0; proc_i < n_processors(); ++proc_i)
  {
    sum += tmp_local_entries[proc_i];
    cum_local_entries[proc_i] = sum;
  }

  for (dof_id_type proc_i = 0; proc_i < n_processors(); ++proc_i)
  {
    if (proc_i == processor_id())
    {
      dof_id_type start_index = 0;
      if (proc_i != 0)
        start_index = cum_local_entries[proc_i - 1];

      for (dof_id_type entry_i = start_index; entry_i < cum_local_entries[proc_i]; ++entry_i)
      {
        _local_entry_ids[entry_i-start_index] = entry_i;
      }
    }
  }
}

template <typename T>
void
DistributedData<T>::addNewEntry(dof_id_type glob_i, const T & entry)
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
DistributedData<T>::addNewEntry(const T & entry)
{
  if (_closed)
    ::mooseError("DistributeData has already been closed, cannot add new elements!");

  dof_id_type n_global_entries = getNumberOfGlobalEntries();

  if (n_global_entries == 0)
    _local_entry_ids.push_back(0);
  else
    _local_entry_ids.push_back(getMaxGlobalIndex() + 1);

  _local_entries.push_back(entry);

  _n_local_entries += 1;
}

template <typename T>
void
DistributedData<T>::changeEntry(dof_id_type glob_i, const T & entry)
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
DistributedData<T>::getGlobalEntry(dof_id_type glob_i) const
{
  auto it = std::find(_local_entry_ids.begin(), _local_entry_ids.end(), glob_i);
  if (it == _local_entry_ids.end())
    ::mooseError("Local object ID (", glob_i, ") does not exists!");

  return _local_entries[std::distance(_local_entry_ids.begin(), it)];
}

template <typename T>
const T &
DistributedData<T>::getLocalEntry(dof_id_type loc_i) const
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
dof_id_type
DistributedData<T>::getNumberOfGlobalEntries() const
{
  dof_id_type val = _n_local_entries;
  _communicator.sum(val);
  return val;
}

template <typename T>
dof_id_type
DistributedData<T>::getMaxGlobalIndex() const
{
  long long int max_val = -1;
  if (!_local_entry_ids.empty())
  {
    auto it = std::max_element(_local_entry_ids.begin(), _local_entry_ids.end());
    max_val = *it;
  }

  _communicator.max(max_val);

  if (max_val == -1)
    mooseError("No data entry has been added yet, the maximum global entry ID cannot be "
               "determined.");

  return (dof_id_type)max_val;
}

template <typename T>
bool
DistributedData<T>::hasGlobalEntry(dof_id_type glob_i) const
{
  const auto it = std::find(_local_entry_ids.begin(), _local_entry_ids.end(), glob_i);
  if (it != _local_entry_ids.end())
    return true;

  return false;
}

template <typename T>
dof_id_type
DistributedData<T>::getLocalIndex(dof_id_type glob_i) const
{
  const auto it = std::find(_local_entry_ids.begin(), _local_entry_ids.end(), glob_i);
  if (it == _local_entry_ids.end())
    ::mooseError("Local object ID (", glob_i, ") does not exists!");

  return std::distance(_local_entry_ids.begin(), it);
}

template <typename T>
dof_id_type
DistributedData<T>::getGlobalIndex(dof_id_type loc_i) const
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
