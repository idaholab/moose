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
  : libMesh::ParallelObject(comm_in), _closed(false), _n_local_samples(0)
{
}

template <typename T>
void
DistributedData<T>::initializeContainer(dof_id_type n_global_samples)
{
  dof_id_type local_sample_begin;
  dof_id_type local_sample_end;
  MooseUtils::linearPartitionItems(n_global_samples,
                                   n_processors(),
                                   processor_id(),
                                   _n_local_samples,
                                   local_sample_begin,
                                   local_sample_end);

  _local_samples.resize(_n_local_samples);
  _local_sample_ids.resize(_n_local_samples);

  for (dof_id_type sample_i = local_sample_begin; sample_i < local_sample_end; ++sample_i)
  {
    _local_sample_ids[sample_i] = sample_i;
  }
}

template <typename T>
const T &
DistributedData<T>::getSample(dof_id_type glob_i)
{
  auto it = std::find(_local_sample_ids.begin(), _local_sample_ids.end(), glob_i);
  if (it == _local_sample_ids.end())
    ::mooseError("Local object ID (", glob_i, ") does not exists!");

  return _local_samples[std::distance(_local_sample_ids.begin(), it)];
}

template <typename T>
const T &
DistributedData<T>::getLocalSample(dof_id_type loc_i)
{
  if (loc_i > _n_local_samples - 1)
    ::mooseError("The requested local index (",
                 loc_i,
                 ") is greater than the size (",
                 _n_local_samples,
                 ") of the locally stored vector!");

  return _local_samples[loc_i];
}

template <typename T>
void
DistributedData<T>::addNewSample(dof_id_type sample_id, const T & sample)
{
  auto it = std::find(_local_sample_ids.begin(), _local_sample_ids.end(), sample_id);
  if (it != _local_sample_ids.end())
    ::mooseError("Local object ID (", sample_id, ") already exists!");

  _local_samples.push_back(sample);
  _local_sample_ids.push_back(sample_id);
  _n_local_samples += 1;
}

template <typename T>
void
DistributedData<T>::changeSample(dof_id_type sample_id, const T & sample)
{
  auto it = std::find(_local_sample_ids.begin(), _local_sample_ids.end(), sample_id);
  if (it == _local_sample_ids.end())
    ::mooseError("Local object ID (", sample_id, ") does not exists!");

  _local_samples[std::distance(_local_sample_ids.begin(), it)] = sample;
}

template <typename T>
dof_id_type
DistributedData<T>::getNumberOfGlobalSamples() const
{
  dof_id_type val = _n_local_samples;
  _communicator.sum(val);
  return val;
}

template <typename T>
bool
DistributedData<T>::hasGlobalSample(dof_id_type glob_i)
{
  const auto it = std::find(_local_sample_ids.begin(), _local_sample_ids.end(), glob_i);
  if (it != _local_sample_ids.end())
    return true;

  return false;
}

template <typename T>
dof_id_type
DistributedData<T>::getLocalIndex(dof_id_type glob_i)
{
  const auto it = std::find(_local_sample_ids.begin(), _local_sample_ids.end(), glob_i);
  if (it == _local_sample_ids.end())
    ::mooseError("Local object ID (", glob_i, ") does not exists!");

  return std::distance(_local_sample_ids.begin(), it);
}

template <typename T>
dof_id_type
DistributedData<T>::getGlobalIndex(dof_id_type loc_i)
{
  if (loc_i > _n_local_samples - 1)
    ::mooseError("The requested local index (",
                 loc_i,
                 ") is greater than the size (",
                 _n_local_samples,
                 ") of the locally stored vector!");

  return _local_sample_ids[loc_i];
}

template class DistributedData<DenseVector<Real>>;
template class DistributedData<std::shared_ptr<DenseVector<Real>>>;
template class DistributedData<std::vector<Real>>;

} // StochasticTools namespace
