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

template <typename T>
class DistributedData : public libMesh::ParallelObject
{
public:
  DistributedData(const libMesh::Parallel::Communicator & comm_in);

  void initializeContainer(dof_id_type num_global_samples);

  void changeSample(dof_id_type sample_id, const T & sample);

  void addNewSample(dof_id_type sample_id, const T & sample);

  const T & getSample(dof_id_type glob_i);

  const T & getLocalSample(dof_id_type loc_i);

  dof_id_type getNumberOfGlobalSamples() const;

  dof_id_type getNumberOfLocalSamples() const { return _n_local_samples; };

  void closeContainer() { _closed = true; };

  std::vector<T> & getLocalSamples() { return _local_samples; };

  std::vector<dof_id_type> & getLocalSampleIDs() { return _local_sample_ids; };

  bool hasGlobalSample(dof_id_type glob_i);

  dof_id_type getLocalIndex(dof_id_type glob_i);

  dof_id_type getGlobalIndex(dof_id_type loc_i);

  typename std::vector<T>::iterator localSampleBegin() { return _local_samples.begin(); };

  typename std::vector<T>::iterator localSampleEnd() { return _local_samples.end(); };

  typename std::vector<dof_id_type>::iterator localSampleIDBegin()
  {
    return _local_sample_ids.begin();
  };

  typename std::vector<dof_id_type>::iterator localSampleIDEnd()
  {
    return _local_sample_ids.end();
  };

protected:
  std::vector<T> _local_samples;

  std::vector<dof_id_type> _local_sample_ids;

  bool _closed;

  dof_id_type _n_local_samples;
};

} // StochasticTools namespace
