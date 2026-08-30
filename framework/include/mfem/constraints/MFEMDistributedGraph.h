//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "libmesh/int_range.h"

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <vector>

#include <mpi.h>

namespace Moose::MFEM
{

/**
 * Block distribution of a dense global index range [0, n) over the ranks of a
 * communicator: rank r owns [begin(r), end(r)). Used to give every global vertex
 * or component id a unique owning rank without any replicated lookup table.
 */
class BlockDistribution
{
public:
  BlockDistribution(std::int64_t n, int nprocs);

  /// Owning rank of global id @p g, which must lie in [0, n).
  int owner(std::int64_t g) const;
  std::int64_t begin(int rank) const;
  std::int64_t end(int rank) const;
  std::int64_t size() const { return _n; }

private:
  std::int64_t _n;
  int _nprocs;
  /// Ids per rank, rounded up so that _chunk * _nprocs >= _n. At least 1.
  std::int64_t _chunk;
};

/**
 * Group @p items by the destination rank returned by @p destination, so they can
 * be handed to allToAll. The per-rank counts are written to @p counts.
 */
template <typename T, typename DestinationFn>
std::vector<T>
groupByDestination(const std::vector<T> & items,
                   int nprocs,
                   DestinationFn destination,
                   std::vector<int> & counts)
{
  // Evaluate the destination once per item and remember it: it can be a binary
  // search over the splitters, which the placement pass would otherwise repeat.
  std::vector<int> destinations(items.size());
  counts.assign(nprocs, 0);
  for (const auto i : index_range(items))
    counts[destinations[i] = destination(items[i])]++;

  std::vector<int> offset(nprocs, 0);
  std::partial_sum(counts.begin(), counts.end() - 1, offset.begin() + 1);

  std::vector<T> grouped(items.size());
  for (const auto i : index_range(items))
    grouped[offset[destinations[i]]++] = items[i];
  return grouped;
}

/**
 * MPI_Alltoallv over a trivially copyable payload. @p send holds the outgoing
 * items grouped by destination rank (see groupByDestination), @p send_counts
 * their per-rank counts. The received items are returned grouped by source rank.
 */
template <typename T>
std::vector<T>
allToAll(const std::vector<T> & send, const std::vector<int> & send_counts, MPI_Comm comm)
{
  int nprocs;
  MPI_Comm_size(comm, &nprocs);

  std::vector<int> recv_counts(nprocs);
  MPI_Alltoall(send_counts.data(), 1, MPI_INT, recv_counts.data(), 1, MPI_INT, comm);

  std::vector<int> send_displs(nprocs, 0), recv_displs(nprocs, 0);
  std::partial_sum(send_counts.begin(), send_counts.end() - 1, send_displs.begin() + 1);
  std::partial_sum(recv_counts.begin(), recv_counts.end() - 1, recv_displs.begin() + 1);

  std::vector<T> recv(static_cast<std::size_t>(recv_displs.back()) + recv_counts.back());

  // A contiguous datatype of sizeof(T) bytes keeps the MPI counts in elements
  // rather than bytes, so a large exchange cannot overflow the int counts.
  MPI_Datatype item_type;
  MPI_Type_contiguous(sizeof(T), MPI_BYTE, &item_type);
  MPI_Type_commit(&item_type);
  MPI_Alltoallv(send.data(),
                send_counts.data(),
                send_displs.data(),
                item_type,
                recv.data(),
                recv_counts.data(),
                recv_displs.data(),
                item_type,
                comm);
  MPI_Type_free(&item_type);

  return recv;
}

/**
 * An undirected edge of the distributed vertex graph.
 *
 * The weight is the pair of *original* canonical vertex ids and never changes;
 * it defines the total edge order that makes the resulting spanning forest
 * independent of the partitioning. The endpoints are the current component
 * labels and are rewritten as components merge.
 */
struct DistributedEdge
{
  /// Weight: original canonical endpoint ids, wu < wv. Unique across the graph.
  std::int64_t wu, wv;
  /// Current component label of each endpoint.
  std::int64_t u, v;
  /// Rank that owns this edge and its index in that rank's edge array, so a
  /// selected edge can be routed back to the rank that has to act on it.
  int origin_rank, origin_index;

  /// Total order on the (unique) weights, giving Kruskal's processing order.
  bool operator<(const DistributedEdge & o) const { return (wu != o.wu) ? wu < o.wu : wv < o.wv; }
};

/**
 * Distributed Boruvka over the edges held collectively by the ranks of @p comm.
 * Each rank passes only its own edges; nothing is replicated, so the memory used
 * is proportional to this rank's share of the graph.
 *
 * Because the edge weights are distinct, the minimum spanning forest is unique,
 * so the result is identical to a serial Kruskal pass over the globally sorted
 * edge list and does not depend on how the graph is distributed.
 *
 * @param edges       this rank's edges. Endpoint labels are updated in place to
 *                    the final component representative of each endpoint
 * @param num_labels  size of the dense label space the endpoints live in
 * @param selected    if non-null, receives `origin_index` for every edge of this
 *                    rank that was selected into the spanning forest. Pass
 *                    nullptr when only the components are wanted
 * @param owned_labels if non-null, receives the final component representative of
 *                    every label this rank owns under BlockDistribution(num_labels,
 *                    comm size), ready to be passed to fetchLabels
 */
void distributedSpanningForest(std::vector<DistributedEdge> & edges,
                               std::int64_t num_labels,
                               MPI_Comm comm,
                               std::vector<int> * selected,
                               std::vector<std::int64_t> * owned_labels = nullptr);

/**
 * Final component representative of each id in @p queries, looked up from the
 * distributed label array. @p owned_labels holds the labels of the ids this rank
 * owns under @p dist.
 */
std::vector<std::int64_t> fetchLabels(const std::vector<std::int64_t> & queries,
                                      const std::vector<std::int64_t> & owned_labels,
                                      const BlockDistribution & dist,
                                      MPI_Comm comm);
}

#endif
