//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LatinHypercubeSampler.h"
#include "Distribution.h"
#include <algorithm>
#include <unordered_map>
registerMooseObjectAliased("StochasticToolsApp", LatinHypercubeSampler, "LatinHypercube");

InputParameters
LatinHypercubeSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Latin Hypercube Sampler.");
  params.addRequiredParam<dof_id_type>("num_rows", "The size of the square matrix to generate.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  return params;
}

LatinHypercubeSampler::LatinHypercubeSampler(const InputParameters & parameters)
  : Sampler(parameters)
{
  const auto & distribution_names = getParam<std::vector<DistributionName>>("distributions");
  for (const DistributionName & name : distribution_names)
    _distributions.push_back(&getDistributionByName(name));

  setNumberOfRows(getParam<dof_id_type>("num_rows"));
  setNumberOfCols(distribution_names.size());
  setNumberOfRandomSeeds(2 * distribution_names.size());

  // The use of MooseRandom in this Sampler is fairly complex. There are two sets of random
  // generators. The first set (n = number columns) is used to generate the random probability
  // within each bin of the Latin hypercube sample. The second set (n) is used to shuffle the
  // probability values. Mainly due to how the shuffle operates, it is necessary for the management
  // of advancement of the generators to be handled manually.
  setAutoAdvanceGenerators(false);
}

void
LatinHypercubeSampler::buildProbabilities(const Sampler::SampleMode mode)
{
  // Flag to indicate what vector index to use in computeSample method.
  _is_local = mode == Sampler::SampleMode::LOCAL;

  const Real bin_size = 1. / getNumberOfRows();
  _probabilities.resize(getNumberOfCols());
  if (mode == Sampler::SampleMode::GLOBAL)
  {
    for (const auto col : make_range(getNumberOfCols()))
    {
      std::vector<Real> & local = _probabilities[col];
      local.resize(getNumberOfRows());
      for (const auto row : make_range(getNumberOfRows()))
      {
        const auto lower = row * bin_size;
        const auto upper = (row + 1) * bin_size;
        local[row] = getRandStateless(static_cast<std::size_t>(row), col) * (upper - lower) + lower;
      }
      shuffleStateless(local, col + getNumberOfCols(), CommMethod::NONE);
    }
  }

  else
  {
    for (const auto col : make_range(getNumberOfCols()))
    {
      std::vector<Real> & local = _probabilities[col];
      local.resize(getNumberOfLocalRows());
      for (const auto row : make_range(getLocalRowBegin(), getLocalRowEnd()))
      {
        const auto lower = row * bin_size;
        const auto upper = (row + 1) * bin_size;
        local[row - getLocalRowBegin()] =
            getRandStateless(static_cast<std::size_t>(row), col) * (upper - lower) + lower;
      }

      // Do not advance generator for shuffle, the shuffle handles it
      shuffleStateless(local, col + getNumberOfCols(), CommMethod::SEMI_LOCAL);
    }
  }

  _stateless_advance_pending = true;
}

Real
LatinHypercubeSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  auto row = _is_local ? row_index - getLocalRowBegin() : row_index;
  return _distributions[col_index]->quantile(_probabilities[col_index][row]);
}

void
LatinHypercubeSampler::computeSampleMatrix(DenseMatrix<Real> & matrix)
{
  _building_matrix = true;
  buildProbabilities(Sampler::SampleMode::GLOBAL);
  Sampler::computeSampleMatrix(matrix);
  _probabilities_ready = false;
  _building_matrix = false;
}

void
LatinHypercubeSampler::computeLocalSampleMatrix(DenseMatrix<Real> & matrix)
{
  _building_matrix = true;
  buildProbabilities(Sampler::SampleMode::LOCAL);
  Sampler::computeLocalSampleMatrix(matrix);
  _probabilities_ready = false;
  _building_matrix = false;
}

void
LatinHypercubeSampler::computeSampleRow(dof_id_type i, std::vector<Real> & data)
{
  if (!_building_matrix && !_probabilities_ready)
  {
    buildProbabilities(Sampler::SampleMode::LOCAL);
    _probabilities_ready = true;
  }

  Sampler::computeSampleRow(i, data);

  if (!_building_matrix && i + 1 == getLocalRowEnd())
    _probabilities_ready = false;
}

std::size_t
LatinHypercubeSampler::getStatelessAdvanceCount(unsigned int seed_index) const
{
  if (!_stateless_advance_pending)
    return 0;

  const auto n_rows = static_cast<std::size_t>(getNumberOfRows());
  const auto n_cols = getNumberOfCols();
  if (n_rows == 0 || n_cols == 0)
    return 0;

  if (seed_index < n_cols)
    return n_rows;

  const auto shuffle_count = n_rows > 0 ? n_rows - 1 : 0;
  if (seed_index < 2 * n_cols)
    return shuffle_count;

  return 0;
}

void
LatinHypercubeSampler::finalizeStatelessAdvance()
{
  _stateless_advance_pending = false;
}

void
LatinHypercubeSampler::shuffleStateless(std::vector<Real> & data,
                                        const std::size_t seed_index,
                                        const CommMethod method)
{
  /**
   * Stateless Fisher-Yates shuffle that uses getRandlStateless for determinism.
   *
   * For distributed data, this computes global indices and performs the required
   * inter-rank swaps via point-to-point exchanges.
   */
  const libMesh::Parallel::Communicator * comm_ptr = nullptr;
  if (method == CommMethod::LOCAL)
    comm_ptr = &_communicator;
  else if (method == CommMethod::SEMI_LOCAL)
    comm_ptr = &_local_comm;

  if (!comm_ptr || comm_ptr->size() == 1)
  {
    const std::size_t n_global = data.size();
    if (n_global < 2)
      return;

    std::size_t rn_ind = 0;
    for (std::size_t i = n_global - 1; i > 0; --i)
    {
      const auto j = getRandlShuffleStateless(rn_ind++, n_global, seed_index);
      // Local Fisher-Yates swap.
      MooseUtils::swap(data, i, j, nullptr);
    }
  }
  else
  {
    std::size_t n_local = data.size();
    std::size_t n_global = n_local;
    comm_ptr->sum(n_global);
    if (n_global < 2)
      return;

    std::vector<std::size_t> offsets(comm_ptr->size());
    {
      std::vector<std::size_t> local_sizes;
      comm_ptr->allgather(n_local, local_sizes);
      // Build prefix offsets to map global indices to rank-local indices.
      for (const auto i : make_range(local_sizes.size() - 1))
        offsets[i + 1] = offsets[i] + local_sizes[i];
    }

    const auto rank = comm_ptr->rank();
    std::size_t rn_ind = 0;
    for (std::size_t idx0 = n_global - 1; idx0 > 0; --idx0)
    {
      const auto idx1 = getRandlShuffleStateless(rn_ind++, n_global, seed_index);

      auto idx0_offset_iter = std::prev(std::upper_bound(offsets.begin(), offsets.end(), idx0));
      auto idx0_rank = std::distance(offsets.begin(), idx0_offset_iter);
      auto idx0_local_idx = idx0 - *idx0_offset_iter;

      auto idx1_offset_iter = std::prev(std::upper_bound(offsets.begin(), offsets.end(), idx1));
      auto idx1_rank = std::distance(offsets.begin(), idx1_offset_iter);
      auto idx1_local_idx = idx1 - *idx1_offset_iter;

      // Request the remote value needed for the swap (if any).
      std::unordered_map<processor_id_type, std::vector<std::size_t>> needs;
      if (idx0_rank != rank && idx1_rank == rank)
        needs[idx0_rank].push_back(idx0_local_idx);
      if (idx0_rank == rank && idx1_rank != rank)
        needs[idx1_rank].push_back(idx1_local_idx);

      std::unordered_map<processor_id_type, std::vector<Real>> returns;
      auto return_functor =
          [&data, &returns](processor_id_type pid, const std::vector<std::size_t> & indices)
      {
        auto & returns_pid = returns[pid];
        for (auto idx : indices)
          returns_pid.push_back(data[idx]);
      };
      Parallel::push_parallel_vector_data(*comm_ptr, needs, return_functor);

      // Receive the remote value needed to complete the swap.
      std::vector<Real> incoming;
      auto recv_functor = [&incoming](processor_id_type /*pid*/, const std::vector<Real> & values)
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
