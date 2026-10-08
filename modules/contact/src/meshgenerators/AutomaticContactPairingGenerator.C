//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AutomaticContactPairingGenerator.h"
#include "MooseMeshUtils.h"
#include "PointListAdaptor.h"

#include "libmesh/boundary_info.h"
#include "libmesh/elem.h"
#include "libmesh/elem_side_builder.h"
#include "libmesh/parallel_algebra.h"

#include <algorithm>

// Make newer nanoflann API compatible with older nanoflann versions
#if NANOFLANN_VERSION < 0x150
namespace nanoflann
{
typedef SearchParams SearchParameters;

template <typename T, typename U>
using ResultItem = std::pair<T, U>;
}
#endif

using NodeBoundaryIDInfo = std::pair<Point, BoundaryID>;

template <>
inline const Point &
PointListAdaptor<NodeBoundaryIDInfo>::getPoint(const NodeBoundaryIDInfo & item) const
{
  return item.first;
}

registerMooseObject("ContactApp", AutomaticContactPairingGenerator);

InputParameters
AutomaticContactPairingGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh to modify");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "automatic_pairing_boundaries",
      "List of boundary names to consider when automatically detecting contact pairs.");
  params.addRequiredParam<Real>(
      "automatic_pairing_distance",
      "Maximum distance (node-to-node or centroid-to-centroid) at which two boundaries are "
      "considered a contact pair.");
  params.addRequiredParam<MooseEnum>(
      "automatic_pairing_method",
      MooseEnum("NODE CENTROID"),
      "Strategy used to detect pairs: NODE uses a KD-tree over boundary nodes; "
      "CENTROID uses sideset center-of-gravity distances.");
  params.addRequiredParam<std::string>(
      "prefix", "Prefix prepended to the names of generated subdomain blocks.");
  params.addRequiredParam<bool>(
      "create_lower_d_blocks",
      "Whether to create primary and secondary lower-dimensional subdomain blocks for each "
      "detected pair.");
  params.addClassDescription(
      "Detects contact surface pairs by proximity, stores them in the mesh meta-data, and "
      "optionally creates lower-dimensional subdomain blocks for use in mortar contact.");
  return params;
}

AutomaticContactPairingGenerator::AutomaticContactPairingGenerator(
    const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _pairing_boundaries(getParam<std::vector<BoundaryName>>("automatic_pairing_boundaries")),
    _pairing_distance(getParam<Real>("automatic_pairing_distance")),
    _pairing_method(getParam<MooseEnum>("automatic_pairing_method")),
    _prefix(getParam<std::string>("prefix")),
    _create_lower_d_blocks(getParam<bool>("create_lower_d_blocks"))
{
  // Declared here, and assigned during generation, so that the pairs are restored from the mesh
  // meta-data on recover, when generation is skipped
  declareMeshProperty<ContactPairs>(contact_pairs_property);
}

std::unique_ptr<MeshBase>
AutomaticContactPairingGenerator::generate()
{
  auto mesh = std::move(_input);

  std::vector<std::pair<BoundaryName, BoundaryName>> pairs;
  if (_pairing_method == "NODE")
    pairs = findPairsNodeProximity(*mesh);
  else
    pairs = findPairsCentroid(*mesh);

  // Mortar contact needs at least one pair to build its lower-dimensional blocks, while node-face
  // contact may proceed with none
  if (pairs.empty() && _create_lower_d_blocks)
    mooseError("AutomaticContactPairingGenerator '",
               name(),
               "': no contact pairs found within distance ",
               _pairing_distance,
               " among boundaries ",
               Moose::stringify(_pairing_boundaries));

  setMeshProperty<ContactPairs>(contact_pairs_property, ContactPairs(pairs.begin(), pairs.end()));

  if (!_create_lower_d_blocks)
    return mesh;

  const bool multiple_pairs = pairs.size() > 1;
  for (const auto & pair : pairs)
  {
    const auto & [primary_boundary, secondary_boundary] = pair;
    const std::string suffix = multiple_pairs ? pairSuffix(pair) : "";
    const std::string primary_name = _prefix + "_primary_subdomain" + suffix;
    const std::string secondary_name = _prefix + "_secondary_subdomain" + suffix;

    MooseMeshUtils::createSubdomainFromSidesets(
        *mesh, {primary_boundary}, MooseMeshUtils::getNextFreeSubdomainID(*mesh), primary_name);
    MooseMeshUtils::createSubdomainFromSidesets(
        *mesh, {secondary_boundary}, MooseMeshUtils::getNextFreeSubdomainID(*mesh), secondary_name);
  }

  return mesh;
}

std::string
AutomaticContactPairingGenerator::pairSuffix(const std::pair<BoundaryName, BoundaryName> & pair)
{
  // Labeling each boundary keeps the suffix unambiguous when boundary names contain underscores,
  // e.g. (top_left, bottom) -> _p_top_left_s_bottom and (top, left_bottom) -> _p_top_s_left_bottom.
  // The single-letter labels keep generated names short, since exodus output truncates names to 32
  // characters by default
  return "_p_" + pair.first + "_s_" + pair.second;
}

std::vector<AutomaticContactPairingGenerator::CandidateBoundary>
AutomaticContactPairingGenerator::candidateBoundaries(const MeshBase & mesh)
{
  // A boundary listed more than once, either repeated or as both a name and an ID, would otherwise
  // be paired with itself
  std::vector<CandidateBoundary> candidates;
  for (const auto & bname : _pairing_boundaries)
  {
    const BoundaryID bid = MooseMeshUtils::getBoundaryID(bname, mesh);
    const auto it =
        std::find_if(candidates.begin(),
                     candidates.end(),
                     [bid](const CandidateBoundary & candidate) { return candidate.id == bid; });
    if (it != candidates.end())
    {
      if (it->name == bname)
        ::mooseError("Boundary '",
                     bname,
                     "' is listed more than once in 'automatic_pairing_boundaries'. Each boundary "
                     "may be listed only once.");
      else
        ::mooseError("Boundaries '",
                     it->name,
                     "' and '",
                     bname,
                     "' in 'automatic_pairing_boundaries' refer to the same boundary (ID ",
                     bid,
                     "). Each boundary may be listed only once.");
    }
    candidates.push_back({bname, bid, 0, Point(0, 0, 0)});
  }

  // Accumulate the area and area-weighted centroid of each candidate sideset. On a distributed
  // mesh, each process accumulates over the sides of its local elements so that ghosted sides are
  // counted once, and the partial sums are then summed across processes
  const bool serial = mesh.is_serial();
  libMesh::ElemSideBuilder side_builder;
  for (const auto & [eid, side, bid] : mesh.get_boundary_info().build_side_list())
  {
    const auto it = std::find_if(candidates.begin(),
                                 candidates.end(),
                                 [bid = bid](const CandidateBoundary & candidate)
                                 { return candidate.id == bid; });
    if (it == candidates.end())
      continue;

    const Elem & elem = *mesh.elem_ptr(eid);
    if (!serial && elem.processor_id() != mesh.processor_id())
      continue;

    const Elem & side_elem = side_builder(elem, side);
    const Real area = side_elem.volume();
    it->area += area;
    it->centroid += side_elem.true_centroid() * area;
  }

  if (!serial)
  {
    // Pack the areas and weighted centroids so that a single reduction suffices
    std::vector<Real> sums;
    sums.reserve(candidates.size() * (1 + Moose::dim));
    for (const auto & candidate : candidates)
    {
      sums.push_back(candidate.area);
      for (const auto d : make_range(Moose::dim))
        sums.push_back(candidate.centroid(d));
    }
    mesh.comm().sum(sums);

    auto sum_it = sums.begin();
    for (auto & candidate : candidates)
    {
      candidate.area = *sum_it++;
      for (const auto d : make_range(Moose::dim))
        candidate.centroid(d) = *sum_it++;
    }
  }

  for (auto & candidate : candidates)
    if (candidate.area > 0)
      candidate.centroid /= candidate.area;

  return candidates;
}

std::pair<BoundaryName, BoundaryName>
AutomaticContactPairingGenerator::orderPair(const CandidateBoundary & a,
                                            const CandidateBoundary & b)
{
  // The larger surface is chosen as primary, which places the Lagrange multipliers on the smaller
  // surface and reduces the number of secondary elements that are only partially covered by the
  // primary surface. Areas that agree to within libMesh's relative TOLERANCE (e.g. matching
  // surfaces whose computed areas differ only by roundoff) fall back to the larger boundary ID so
  // that the assignment is deterministic
  const Real area_tol = TOLERANCE * std::max(a.area, b.area);
  const bool a_is_primary = std::abs(a.area - b.area) > area_tol ? a.area > b.area : a.id > b.id;
  if (a_is_primary)
    return {a.name, b.name};
  else
    return {b.name, a.name};
}

std::vector<std::pair<BoundaryName, BoundaryName>>
AutomaticContactPairingGenerator::findPairsNodeProximity(const MeshBase & mesh)
{
  const auto candidates = candidateBoundaries(mesh);
  const auto find_candidate = [&candidates](const BoundaryID bid) -> const CandidateBoundary &
  {
    const auto it =
        std::find_if(candidates.begin(),
                     candidates.end(),
                     [bid](const CandidateBoundary & candidate) { return candidate.id == bid; });
    mooseAssert(it != candidates.end(), "Boundary not in candidate list");
    return *it;
  };

  // Collect nodes on candidate boundaries, from both sidesets and nodesets. On a distributed mesh,
  // each process contributes the boundary nodes it owns, which it sees together with all of their
  // boundary sides through the default point-neighbor ghosting, and the lists are then gathered so
  // that every process searches the complete set
  const bool serial = mesh.is_serial();
  std::vector<NodeBoundaryIDInfo> node_bid_list;
  for (const auto & candidate : candidates)
    for (const auto node_id : MooseMeshUtils::getBoundaryNodes(mesh, candidate.id))
    {
      const Node & node = mesh.node_ref(node_id);
      if (serial || node.processor_id() == mesh.processor_id())
        node_bid_list.emplace_back(node, candidate.id);
    }

  if (!serial)
    mesh.comm().allgather(node_bid_list);

  // Sort by boundary id for deterministic ordering
  std::sort(node_bid_list.begin(),
            node_bid_list.end(),
            [](const NodeBoundaryIDInfo & a, const NodeBoundaryIDInfo & b)
            { return a.second < b.second; });

  using KDTreeType = nanoflann::KDTreeSingleIndexAdaptor<
      nanoflann::L2_Simple_Adaptor<Real, PointListAdaptor<NodeBoundaryIDInfo>, Real, std::size_t>,
      PointListAdaptor<NodeBoundaryIDInfo>,
      Moose::dim,
      std::size_t>;

  const unsigned int max_leaf_size = 20;
  auto point_list =
      PointListAdaptor<NodeBoundaryIDInfo>(node_bid_list.begin(), node_bid_list.end());
  auto kd_tree = std::make_unique<KDTreeType>(
      Moose::dim, point_list, nanoflann::KDTreeSingleIndexAdaptorParams(max_leaf_size));
  kd_tree->buildIndex();

  nanoflann::SearchParameters search_params;
  std::vector<nanoflann::ResultItem<std::size_t, Real>> ret_matches;
  std::vector<std::pair<BoundaryName, BoundaryName>> pairs;

  for (const auto & [entry_node, entry_bid] : node_bid_list)
  {
    ret_matches.clear();
    kd_tree->radiusSearch(
        &entry_node(0), _pairing_distance * _pairing_distance, ret_matches, search_params);

    for (const auto & [pair_idx, _] : ret_matches)
    {
      const auto & [pair_node, pair_bid] = node_bid_list[pair_idx];
      libmesh_ignore(pair_node);

      // Skip nodes on the same boundary
      if (pair_bid == entry_bid)
        continue;

      pairs.push_back(orderPair(find_candidate(entry_bid), find_candidate(pair_bid)));
    }
  }

  removeDuplicatePairs(pairs);
  return pairs;
}

std::vector<std::pair<BoundaryName, BoundaryName>>
AutomaticContactPairingGenerator::findPairsCentroid(const MeshBase & mesh)
{
  const auto candidates = candidateBoundaries(mesh);
  for (const auto & candidate : candidates)
    if (candidate.area == 0)
      ::mooseError("Boundary '", candidate.name, "' not found in mesh.");

  // Find all pairs within distance
  std::vector<std::pair<BoundaryName, BoundaryName>> pairs;
  for (const auto i : index_range(candidates))
    for (const auto j : make_range(i + 1, candidates.size()))
      if ((candidates[i].centroid - candidates[j].centroid).norm() <= _pairing_distance)
        pairs.push_back(orderPair(candidates[i], candidates[j]));

  removeDuplicatePairs(pairs);
  return pairs;
}

void
AutomaticContactPairingGenerator::removeDuplicatePairs(
    std::vector<std::pair<BoundaryName, BoundaryName>> & pairs)
{
  std::sort(pairs.begin(), pairs.end());
  pairs.erase(std::unique(pairs.begin(), pairs.end()), pairs.end());
}
