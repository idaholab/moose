//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDistributedGraph.h"
#include "MooseError.h"
#include "libmesh/int_range.h"

namespace
{

/// One rank's offer of an edge to one of its two endpoint components. The
/// component owner keeps the offer with the smallest weight.
struct Proposal
{
  /// Component the offer is made to; the owner of this id receives it.
  std::int64_t component;
  /// Weight of the offered edge, and the component at its other end.
  std::int64_t wu, wv, partner;
  int origin_rank, origin_index;
};

/// Query/answer record for looking up the current label of a global id.
struct LabelQuery
{
  std::int64_t id;
  std::int64_t label;
  /// Position in the requesting rank's query array, so answers can be unpacked
  /// without depending on the order in which they come back.
  int origin_rank, origin_index;
};
}

namespace Moose::MFEM
{

BlockDistribution::BlockDistribution(std::int64_t n, int nprocs) : _n(n), _nprocs(nprocs)
{
  // Round up so the last rank absorbs the remainder and chunk * nprocs >= n. A
  // chunk of at least 1 keeps owner() well defined for an empty range.
  _chunk = std::max<std::int64_t>(1, (n + nprocs - 1) / nprocs);
}

int
BlockDistribution::owner(std::int64_t g) const
{
  mooseAssert(g >= 0 && g < _n, "Global id out of range");
  // With chunk rounded up, g / chunk is always < nprocs for g < n.
  return static_cast<int>(g / _chunk);
}

std::int64_t
BlockDistribution::begin(int rank) const
{
  return std::min<std::int64_t>(static_cast<std::int64_t>(rank) * _chunk, _n);
}

std::int64_t
BlockDistribution::end(int rank) const
{
  return std::min<std::int64_t>(static_cast<std::int64_t>(rank + 1) * _chunk, _n);
}

std::vector<std::int64_t>
fetchLabels(const std::vector<std::int64_t> & queries,
            const std::vector<std::int64_t> & owned_labels,
            const BlockDistribution & dist,
            MPI_Comm comm)
{
  int nprocs, rank;
  MPI_Comm_size(comm, &nprocs);
  MPI_Comm_rank(comm, &rank);

  std::vector<LabelQuery> outgoing(queries.size());
  for (const auto i : index_range(queries))
    outgoing[i] = {queries[i], 0, rank, static_cast<int>(i)};

  std::vector<int> counts;
  const auto grouped = groupByDestination(
      outgoing, nprocs, [&](const LabelQuery & q) { return dist.owner(q.id); }, counts);

  auto received = allToAll(grouped, counts, comm);

  // Answer from the owned slice, then send each answer straight back to the rank
  // that asked for it.
  const std::int64_t owned_begin = dist.begin(rank);
  for (auto & q : received)
    q.label = owned_labels[q.id - owned_begin];

  std::vector<int> answer_counts;
  const auto answers = groupByDestination(
      received, nprocs, [](const LabelQuery & q) { return q.origin_rank; }, answer_counts);

  std::vector<std::int64_t> labels(queries.size());
  for (const auto & q : allToAll(answers, answer_counts, comm))
    labels[q.origin_index] = q.label;
  return labels;
}

void
distributedSpanningForest(std::vector<DistributedEdge> & edges,
                          std::int64_t num_labels,
                          MPI_Comm comm,
                          std::vector<int> * selected,
                          std::vector<std::int64_t> * owned_labels)
{
  int nprocs, rank;
  MPI_Comm_size(comm, &nprocs);
  MPI_Comm_rank(comm, &rank);

  if (num_labels == 0)
  {
    if (owned_labels)
      owned_labels->clear();
    return;
  }

  const BlockDistribution dist(num_labels, nprocs);
  const std::int64_t owned_begin = dist.begin(rank);
  const auto num_owned = static_cast<std::size_t>(dist.end(rank) - owned_begin);

  // Representative of every id this rank owns. Edge endpoints always name a
  // current representative, but ids absorbed in an earlier round do not, so this
  // has to be carried forward separately from the per-round hooks below.
  std::vector<std::int64_t> label(num_owned);
  std::iota(label.begin(), label.end(), owned_begin);

  while (true)
  {
    // Drop edges whose endpoints have already merged; they are interior to a
    // component and can never join the forest.
    edges.erase(std::remove_if(edges.begin(),
                               edges.end(),
                               [](const DistributedEdge & e) { return e.u == e.v; }),
                edges.end());

    std::int64_t local_active = static_cast<std::int64_t>(edges.size()), global_active = 0;
    MPI_Allreduce(&local_active, &global_active, 1, MPI_INT64_T, MPI_SUM, comm);
    if (global_active == 0)
      break;

    // Offer every remaining edge to the components at both of its ends.
    std::vector<Proposal> proposals;
    proposals.reserve(2 * edges.size());
    for (const auto & e : edges)
    {
      proposals.push_back({e.u, e.wu, e.wv, e.v, e.origin_rank, e.origin_index});
      proposals.push_back({e.v, e.wu, e.wv, e.u, e.origin_rank, e.origin_index});
    }

    std::vector<int> counts;
    const auto grouped = groupByDestination(
        proposals, nprocs, [&](const Proposal & p) { return dist.owner(p.component); }, counts);
    const auto received = allToAll(grouped, counts, comm);

    // Each component keeps its lightest incident edge. Distinct weights make the
    // choice unique, so the forest matches a serial Kruskal pass exactly.
    std::vector<const Proposal *> best(num_owned, nullptr);
    for (const auto & p : received)
    {
      const auto slot = static_cast<std::size_t>(p.component - owned_begin);
      if (!best[slot] || p.wu < best[slot]->wu || (p.wu == best[slot]->wu && p.wv < best[slot]->wv))
        best[slot] = &p;
    }

    // Tell the owning rank about every edge that was picked. An edge chosen by
    // both of its endpoints arrives twice; marking it is idempotent.
    if (selected)
    {
      std::vector<Proposal> picks;
      picks.reserve(num_owned);
      for (const auto slot : index_range(best))
        if (best[slot])
          picks.push_back(*best[slot]);

      std::vector<int> pick_counts;
      const auto pick_grouped = groupByDestination(
          picks, nprocs, [](const Proposal & p) { return p.origin_rank; }, pick_counts);
      for (const auto & p : allToAll(pick_grouped, pick_counts, comm))
        selected->push_back(p.origin_index);
    }

    // Hook each component onto the component at the far end of its chosen edge.
    // A component that received no offer is already a root and hooks to itself.
    std::vector<std::int64_t> hook(num_owned), self(num_owned);
    std::iota(self.begin(), self.end(), owned_begin);
    for (const auto slot : index_range(best))
      hook[slot] = best[slot] ? best[slot]->partner : self[slot];

    // Two components that chose the same edge point at each other. Root that pair
    // at the smaller id; every other chain terminates at such a pair, because the
    // chosen weights strictly decrease along a chain.
    const auto partner_hook = fetchLabels(hook, hook, dist, comm);
    for (const auto slot : index_range(hook))
      if (partner_hook[slot] == self[slot])
        hook[slot] = std::min(self[slot], hook[slot]);

    // Pointer-jump until every component points directly at its root.
    while (true)
    {
      const auto grandparent = fetchLabels(hook, hook, dist, comm);
      std::int64_t local_changed = 0;
      for (const auto slot : index_range(hook))
        if (hook[slot] != grandparent[slot])
        {
          hook[slot] = grandparent[slot];
          local_changed = 1;
        }

      std::int64_t global_changed = 0;
      MPI_Allreduce(&local_changed, &global_changed, 1, MPI_INT64_T, MPI_MAX, comm);
      if (global_changed == 0)
        break;
    }

    // Carry every id's representative through this round's merge.
    label = fetchLabels(label, hook, dist, comm);

    // Relabel this rank's edges onto the merged components.
    std::vector<std::int64_t> endpoints;
    endpoints.reserve(2 * edges.size());
    for (const auto & e : edges)
    {
      endpoints.push_back(e.u);
      endpoints.push_back(e.v);
    }
    const auto relabelled = fetchLabels(endpoints, hook, dist, comm);
    for (const auto i : index_range(edges))
    {
      edges[i].u = relabelled[2 * i];
      edges[i].v = relabelled[2 * i + 1];
    }
  }

  if (owned_labels)
    *owned_labels = std::move(label);
}
}

#endif
