//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "MooseRandom.h"
#include "libmesh/communicator.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_sync.h"
#include "libmesh/libmesh_common.h"
#include <list>
#include <memory>
#include <iterator>
#include <algorithm>

namespace MooseUtils
{
///@{
/**
 * Swap function for serial or distributed vector of data.
 * @param data The vector on which the values are to be swapped
 * @param idx0, idx1 The global indices to be swapped
 * @param comm_ptr Optional Communicator, if provided and running with multiple processors the
 *                 vector is assumed to be distributed
 */
template <typename T>
void swap(std::vector<T> & data,
          const std::size_t idx0,
          const std::size_t idx1,
          const libMesh::Parallel::Communicator & comm);
template <typename T>
void swap(std::vector<T> & data,
          const std::size_t idx0,
          const std::size_t idx1,
          const libMesh::Parallel::Communicator * comm_ptr = nullptr);
///@}

///@{
/**
 * Shuffle function for serial or distributed vector of data that shuffles in place.
 * @param data The vector on which the values are to be swapped
 * @param generator Random number generator to use for shuffle
 * @param seed_index (default: 0) The seed index to use for calls to randl
 * @param comm_ptr Optional Communicator, if provided and running with multiple processors the
 *                 vector is assumed to be distributed
 *
 * Both the serial and distributed version implement a Fisher-Yates shuffle
 * https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
 *
 * NOTE: This distributed shuffle I have here does a parallel communication with each swap pair
 *       generated. I am certain that there are more efficient ways to shuffle a distributed vector,
 *       but there doesn't seem to be an algorithm in the literature (my search was not extensive).
 *
 *       The reason I came to this conclusion was because of a 2019 paper, which states the
 *       following (https://iopscience.iop.org/article/10.1088/1742-6596/1196/1/012035):
 *
 *           The study also says that the Fisher-Yates Shuffle can be developed in two ways,
 *           namely the algorithm's initial assumptions that allow for discrete uniform variables,
 *           and also with the avent of large core clusters and GPUs, there is an interest in making
 *           parallel versions of this algorithm.
 *
 *       This paper discusses the MergeShuffle (https://arxiv.org/abs/1508.03167), but that is a
 *       shared memory parallel algorithm.
 *
 *       Hence, if you want to become famous create a parallel Fisher-Yates algorithm for MPI.
 */
template <typename T>
void shuffle(std::vector<T> & data, MooseRandom & generator, const std::size_t seed_index = 0);
template <typename T>
void shuffle(std::vector<T> & data,
             MooseRandom & generator,
             const libMesh::Parallel::Communicator & comm);
template <typename T>
void shuffle(std::vector<T> & data,
             MooseRandom & generator,
             const std::size_t seed_index,
             const libMesh::Parallel::Communicator & comm);
template <typename T>
void shuffle(std::vector<T> & data,
             MooseRandom & generator,
             const std::size_t seed_index,
             const libMesh::Parallel::Communicator * comm_ptr);
///@}

///@{
/**
 * Randomly resample a vector of data, allowing a value to be repeated.
 * @param data The vector on which the values are to be swapped
 * @param generator Random number generator to use for shuffle
 * @param seed_index (default: 0) The seed index to use for calls to randl
 * @param comm_ptr Optional Communicator, if provided and running with multiple processors the
 *                 vector is assumed to be distributed
 */
template <typename T>
std::vector<T>
resample(const std::vector<T> & data, MooseRandom & generator, const std::size_t seed_index = 0);
template <typename T>
std::vector<T> resample(const std::vector<T> & data,
                        MooseRandom & generator,
                        const libMesh::Parallel::Communicator & comm);
template <typename T>
std::vector<T> resample(const std::vector<T> & data,
                        MooseRandom & generator,
                        const std::size_t seed_index,
                        const libMesh::Parallel::Communicator & comm);
template <typename T>
std::vector<T> resample(const std::vector<T> & data,
                        MooseRandom & generator,
                        const std::size_t seed_index,
                        const libMesh::Parallel::Communicator * comm_ptr);
//@}

///@{
/**
 * Randomly resample a vector of data and apply a functor, allowing a value to be repeated.
 * @param data The vector on which the values are to be swapped
 * @param functor Functor to apply to each entry of the resampled vector
 * @param generator Random number generator to use for shuffle
 * @param seed_index (default: 0) The seed index to use for calls to randl
 * @param comm_ptr Optional Communicator, if provided and running with multiple processors the
 *                 vector is assumed to be distributed
 */
template <typename T, typename ActionFunctor>
void resampleWithFunctor(const std::vector<T> & data,
                         const ActionFunctor & functor,
                         MooseRandom & generator,
                         const std::size_t seed_index = 0);
template <typename T, typename ActionFunctor>
void resampleWithFunctor(const std::vector<T> & data,
                         const ActionFunctor & functor,
                         MooseRandom & generator,
                         const libMesh::Parallel::Communicator & comm);
template <typename T, typename ActionFunctor>
void resampleWithFunctor(const std::vector<T> & data,
                         const ActionFunctor & functor,
                         MooseRandom & generator,
                         const std::size_t seed_index,
                         const libMesh::Parallel::Communicator & comm);
template <typename T, typename ActionFunctor>
void resampleWithFunctor(const std::vector<T> & data,
                         const ActionFunctor & functor,
                         MooseRandom & generator,
                         const std::size_t seed_index,
                         const libMesh::Parallel::Communicator * comm_ptr);
//@}
}

template <typename T>
void
MooseUtils::swap(std::vector<T> & data,
                 const std::size_t idx0,
                 const std::size_t idx1,
                 const libMesh::Parallel::Communicator * comm_ptr)
{
  if (!comm_ptr || comm_ptr->size() == 1)
  {
    mooseAssert(idx0 < data.size(),
                "idx0 (" << idx0 << ") out of range, data.size() is " << data.size());
    mooseAssert(idx1 < data.size(),
                "idx1 (" << idx1 << ") out of range, data.size() is " << data.size());
    std::swap(data[idx0], data[idx1]);
  }

  else
  {
    // Size of the local input data
    const auto n_local = data.size();
    const auto rank = comm_ptr->rank();

    // Compute the global size of the vector
    std::size_t n_global = n_local;
    comm_ptr->sum(n_global);
    mooseAssert(idx0 < n_global,
                "idx0 (" << idx0 << ") out of range, the global data size is " << n_global);
    mooseAssert(idx1 < n_global,
                "idx1 (" << idx1 << ") out of range, the global data size is " << n_global);

    // Compute the vector data offsets, the scope cleans up the "n_local" vector
    std::vector<std::size_t> offsets(comm_ptr->size());
    {
      std::vector<std::size_t> local_sizes;
      comm_ptr->allgather(n_local, local_sizes);
      for (std::size_t i = 0; i < local_sizes.size() - 1; ++i)
        offsets[i + 1] = offsets[i] + local_sizes[i];
    }

    // Locate the rank and local index of the data to swap
    auto idx0_offset_iter = std::prev(std::upper_bound(offsets.begin(), offsets.end(), idx0));
    auto idx0_rank = std::distance(offsets.begin(), idx0_offset_iter);
    auto idx0_local_idx = idx0 - *idx0_offset_iter;

    auto idx1_offset_iter = std::prev(std::upper_bound(offsets.begin(), offsets.end(), idx1));
    auto idx1_rank = std::distance(offsets.begin(), idx1_offset_iter);
    auto idx1_local_idx = idx1 - *idx1_offset_iter;

    // The values, if any, needed from other rank
    std::unordered_map<processor_id_type, std::vector<std::size_t>> needs;
    if (idx0_rank != rank && idx1_rank == rank)
      needs[idx0_rank].push_back(idx0_local_idx);
    if (idx0_rank == rank && idx1_rank != rank)
      needs[idx1_rank].push_back(idx1_local_idx);

    // Collect the values needed by this processor
    std::unordered_map<processor_id_type, std::vector<T>> returns;
    auto return_functor =
        [&data, &returns](processor_id_type pid, const std::vector<std::size_t> & indices)
    {
      auto & returns_pid = returns[pid];
      for (auto idx : indices)
        returns_pid.push_back(data[idx]);
    };
    Parallel::push_parallel_vector_data(*comm_ptr, needs, return_functor);

    // Receive needed values from the others processors
    std::vector<T> incoming;
    auto recv_functor = [&incoming](processor_id_type /*pid*/, const std::vector<T> & values)
    { incoming = values; };
    Parallel::push_parallel_vector_data(*comm_ptr, returns, recv_functor);

    if (idx0_rank == rank && idx1_rank == rank)
      MooseUtils::swap(data, idx0_local_idx, idx1_local_idx);

    else if (idx0_rank == rank)
    {
      mooseAssert(incoming.size() == 1, "Only one value should be received");
      data[idx0_local_idx] = incoming[0];
    }
    else if (idx1_rank == rank)
    {
      mooseAssert(incoming.size() == 1, "Only one value should be received");
      data[idx1_local_idx] = incoming[0];
    }
  }
}

template <typename T>
void
MooseUtils::shuffle(std::vector<T> & data,
                    MooseRandom & generator,
                    const std::size_t seed_index,
                    const libMesh::Parallel::Communicator * comm_ptr)
{
  // REPLICATED data
  if (!comm_ptr || comm_ptr->size() == 1)
  {
    std::size_t n_global = data.size();
    for (std::size_t i = n_global - 1; i > 0; --i)
    {
      auto j = generator.randl(seed_index, 0, i);
      MooseUtils::swap(data, i, j, nullptr);
    }
  }

  // DISTRIBUTED data
  else
  {
    // Local/global size
    std::size_t n_local = data.size();
    std::size_t n_global = n_local;
    comm_ptr->sum(n_global);

    // Compute the vector data offsets, the scope cleans up the "n_local" vector
    std::vector<std::size_t> offsets(comm_ptr->size());
    {
      std::vector<std::size_t> local_sizes;
      comm_ptr->allgather(n_local, local_sizes);
      for (std::size_t i = 0; i < local_sizes.size() - 1; ++i)
        offsets[i + 1] = offsets[i] + local_sizes[i];
    }

    // Perform swaps
    auto rank = comm_ptr->rank();
    for (std::size_t idx0 = n_global - 1; idx0 > 0; --idx0)
    {
      auto idx1 = generator.randl(seed_index, 0, idx0);

      // Locate the rank and local index of the data to swap
      auto idx0_offset_iter = std::prev(std::upper_bound(offsets.begin(), offsets.end(), idx0));
      auto idx0_rank = std::distance(offsets.begin(), idx0_offset_iter);
      auto idx0_local_idx = idx0 - *idx0_offset_iter;

      auto idx1_offset_iter = std::prev(std::upper_bound(offsets.begin(), offsets.end(), idx1));
      auto idx1_rank = std::distance(offsets.begin(), idx1_offset_iter);
      auto idx1_local_idx = idx1 - *idx1_offset_iter;

      // The values, if any, needed from other rank
      std::unordered_map<processor_id_type, std::vector<std::size_t>> needs;
      if (idx0_rank != rank && idx1_rank == rank)
        needs[idx0_rank].push_back(idx0_local_idx);
      if (idx0_rank == rank && idx1_rank != rank)
        needs[idx1_rank].push_back(idx1_local_idx);

      // Collect the values needed by this processor
      std::unordered_map<processor_id_type, std::vector<T>> returns;
      auto return_functor =
          [&data, &returns](processor_id_type pid, const std::vector<std::size_t> & indices)
      {
        auto & returns_pid = returns[pid];
        for (auto idx : indices)
          returns_pid.push_back(data[idx]);
      };
      Parallel::push_parallel_vector_data(*comm_ptr, needs, return_functor);

      // Receive needed values from the others processors
      std::vector<T> incoming;
      auto recv_functor = [&incoming](processor_id_type /*pid*/, const std::vector<T> & values)
      { incoming = values; };
      Parallel::push_parallel_vector_data(*comm_ptr, returns, recv_functor);

      if (idx0_rank == rank && idx1_rank == rank)
        MooseUtils::swap(data, idx0_local_idx, idx1_local_idx);

      else if (idx0_rank == rank)
      {
        mooseAssert(incoming.size() == 1, "Only one value should be received");
        data[idx0_local_idx] = incoming[0];
      }
      else if (idx1_rank == rank)
      {
        mooseAssert(incoming.size() == 1, "Only one value should be received");
        data[idx1_local_idx] = incoming[0];
      }
    }
  }
}

template <typename T>
std::vector<T>
MooseUtils::resample(const std::vector<T> & data,
                     MooseRandom & generator,
                     const std::size_t seed_index,
                     const libMesh::Parallel::Communicator * comm_ptr)
{
  // Size of the local input data
  const std::size_t n_local = data.size();

  // Re-sampled data vector to be returned
  std::vector<T> replicate(n_local);

  // REPLICATED data
  if (!comm_ptr || comm_ptr->size() == 1)
  {
    replicate.resize(n_local);
    for (std::size_t j = 0; j < n_local; ++j)
    {
      auto index = generator.randl(seed_index, 0, n_local);
      replicate[j] = data[index];
    }
  }

  // DISTRIBUTED data
  else
  {
    // Compute the global size of the vector
    std::size_t n_global = n_local;
    comm_ptr->sum(n_global);

    // Compute the vector data offsets, the scope cleans up the "n_local" vector
    std::vector<std::size_t> offsets(comm_ptr->size());
    {
      std::vector<std::size_t> local_sizes;
      comm_ptr->allgather(n_local, local_sizes);
      for (std::size_t i = 0; i < local_sizes.size() - 1; ++i)
        offsets[i + 1] = offsets[i] + local_sizes[i];
    }

    // Advance the random number generator to the current offset
    const auto rank = comm_ptr->rank();
    for (std::size_t i = 0; i < offsets[rank]; ++i)
      generator.randl(seed_index, 0, n_global);

    // Compute the needs for this processor
    std::unordered_map<processor_id_type, std::vector<std::pair<std::size_t, std::size_t>>> needs;
    for (std::size_t i = 0; i < n_local; ++i)
    {
      const auto idx = generator.randl(seed_index, 0, n_global); // random global index

      // Locate the rank and local index of the data desired
      const auto idx_offset_iter = std::prev(std::upper_bound(offsets.begin(), offsets.end(), idx));
      const auto idx_rank = std::distance(offsets.begin(), idx_offset_iter);
      const auto idx_local_idx = idx - *idx_offset_iter;

      // Local available data can be inserted into the re-sample, non-local data is add the the
      // needs from other ranks
      if (idx_rank == rank)
        replicate[i] = data[idx_local_idx];
      else
        needs[idx_rank].emplace_back(idx_local_idx, i);
    }

    // Advance the random number generator to the end of the global vector
    for (std::size_t i = offsets[rank] + n_local; i < n_global; ++i)
      generator.randl(seed_index, 0, n_global);

    // Collect the values to be returned to the various processors
    std::unordered_map<processor_id_type, std::vector<std::pair<T, std::size_t>>> returns;
    auto return_functor =
        [&data, &returns](processor_id_type pid,
                          const std::vector<std::pair<std::size_t, std::size_t>> & indices)
    {
      auto & returns_pid = returns[pid];
      for (const auto & idx : indices)
        returns_pid.emplace_back(data[idx.first], idx.second);
    };
    Parallel::push_parallel_vector_data(*comm_ptr, needs, return_functor);

    // Receive resampled values from the various processors
    auto recv_functor =
        [&replicate](processor_id_type, const std::vector<std::pair<T, std::size_t>> & values)
    {
      for (const auto & value : values)
        replicate[value.second] = value.first;
    };
    Parallel::push_parallel_vector_data(*comm_ptr, returns, recv_functor);
  }
  return replicate;
}

template <typename T, typename ActionFunctor>
void
MooseUtils::resampleWithFunctor(const std::vector<T> & data,
                                const ActionFunctor & functor,
                                MooseRandom & generator,
                                const std::size_t seed_index,
                                const libMesh::Parallel::Communicator * comm_ptr)
{
  const std::size_t n_local = data.size();

  if (!comm_ptr || comm_ptr->size() == 1)
  {
    for (std::size_t j = 0; j < n_local; ++j)
    {
      auto index = generator.randl(seed_index, 0, n_local);
      functor(data[index]);
    }
  }
  else
  {
    // Compute the global size of the vector
    std::size_t n_global = n_local;
    comm_ptr->sum(n_global);

    // Compute the vector data offsets, the scope cleans up the "n_local" vector
    std::vector<std::size_t> offsets(comm_ptr->size());
    {
      std::vector<std::size_t> local_sizes;
      comm_ptr->allgather(n_local, local_sizes);
      for (std::size_t i = 0; i < local_sizes.size() - 1; ++i)
        offsets[i + 1] = offsets[i] + local_sizes[i];
    }

    // Advance the random number generator to the current offset
    const auto rank = comm_ptr->rank();
    for (std::size_t i = 0; i < offsets[rank]; ++i)
      generator.randl(seed_index, 0, n_global);

    // Compute the needs for this processor
    std::unordered_map<processor_id_type, std::vector<std::size_t>> indices;
    for (std::size_t i = 0; i < n_local; ++i)
    {
      const auto idx = generator.randl(seed_index, 0, n_global); // random global index

      // Locate the rank and local index of the data desired
      const auto idx_offset_iter = std::prev(std::upper_bound(offsets.begin(), offsets.end(), idx));
      const auto idx_rank = std::distance(offsets.begin(), idx_offset_iter);
      const auto idx_local_idx = idx - *idx_offset_iter;

      // Push back the index to appropriate rank
      indices[idx_rank].push_back(idx_local_idx);
    }

    // Advance the random number generator to the end of the global vector
    for (std::size_t i = offsets[rank] + n_local; i < n_global; ++i)
      generator.randl(seed_index, 0, n_global);

    // Send the indices to the appropriate rank and have the calculator do its work
    auto act_functor =
        [&functor, &data](processor_id_type /*pid*/, const std::vector<std::size_t> & indices)
    {
      for (const auto & idx : indices)
        functor(data[idx]);
    };
    Parallel::push_parallel_vector_data(*comm_ptr, indices, act_functor);
  }
}

template <typename T>
void
MooseUtils::swap(std::vector<T> & data,
                 const std::size_t idx0,
                 const std::size_t idx1,
                 const libMesh::Parallel::Communicator & comm)
{
  MooseUtils::swap<T>(data, idx0, idx1, &comm);
}

template <typename T>
void
MooseUtils::shuffle(std::vector<T> & data, MooseRandom & generator, const std::size_t seed_index)
{
  return MooseUtils::shuffle(data, generator, seed_index, nullptr);
}

template <typename T>
void
MooseUtils::shuffle(std::vector<T> & data,
                    MooseRandom & generator,
                    const libMesh::Parallel::Communicator & comm)
{
  return MooseUtils::shuffle(data, generator, 0, &comm);
}

template <typename T>
void
MooseUtils::shuffle(std::vector<T> & data,
                    MooseRandom & generator,
                    const std::size_t seed_index,
                    const libMesh::Parallel::Communicator & comm)
{
  return MooseUtils::shuffle(data, generator, seed_index, &comm);
}

template <typename T>
std::vector<T>
MooseUtils::resample(const std::vector<T> & data,
                     MooseRandom & generator,
                     const std::size_t seed_index)
{
  return MooseUtils::resample(data, generator, seed_index, nullptr);
}

template <typename T>
std::vector<T>
MooseUtils::resample(const std::vector<T> & data,
                     MooseRandom & generator,
                     const libMesh::Parallel::Communicator & comm)
{
  return MooseUtils::resample(data, generator, 0, &comm);
}

template <typename T>
std::vector<T>
MooseUtils::resample(const std::vector<T> & data,
                     MooseRandom & generator,
                     const std::size_t seed_index,
                     const libMesh::Parallel::Communicator & comm)
{
  return MooseUtils::resample(data, generator, seed_index, &comm);
}

template <typename T, typename ActionFunctor>
void
MooseUtils::resampleWithFunctor(const std::vector<T> & data,
                                const ActionFunctor & functor,
                                MooseRandom & generator,
                                const std::size_t seed_index)
{
  return MooseUtils::resampleWithFunctor(data, functor, generator, seed_index, nullptr);
}

template <typename T, typename ActionFunctor>
void
MooseUtils::resampleWithFunctor(const std::vector<T> & data,
                                const ActionFunctor & functor,
                                MooseRandom & generator,
                                const libMesh::Parallel::Communicator & comm)
{
  return MooseUtils::resampleWithFunctor(data, functor, generator, 0, &comm);
}

template <typename T, typename ActionFunctor>
void
MooseUtils::resampleWithFunctor(const std::vector<T> & data,
                                const ActionFunctor & functor,
                                MooseRandom & generator,
                                const std::size_t seed_index,
                                const libMesh::Parallel::Communicator & comm)
{
  return MooseUtils::resampleWithFunctor(data, functor, generator, seed_index, &comm);
}
