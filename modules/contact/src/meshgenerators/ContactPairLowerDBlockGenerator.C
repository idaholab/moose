//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContactPairLowerDBlockGenerator.h"
#include "MooseMeshUtils.h"
#include "PointListAdaptor.h"

#include "libmesh/boundary_info.h"
#include "libmesh/elem.h"

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

using NodeBoundaryIDInfo = std::pair<const Node *, BoundaryID>;

template <>
inline const Point &
PointListAdaptor<NodeBoundaryIDInfo>::getPoint(const NodeBoundaryIDInfo & item) const
{
  return *(item.first);
}

registerMooseObject("ContactApp", ContactPairLowerDBlockGenerator);

InputParameters
ContactPairLowerDBlockGenerator::validParams()
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
  params.addClassDescription(
      "Detects contact surface pairs by proximity and creates lower-dimensional subdomain blocks "
      "for use in mortar contact.");
  return params;
}

ContactPairLowerDBlockGenerator::ContactPairLowerDBlockGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _pairing_boundaries(getParam<std::vector<BoundaryName>>("automatic_pairing_boundaries")),
    _pairing_distance(getParam<Real>("automatic_pairing_distance")),
    _pairing_method(getParam<MooseEnum>("automatic_pairing_method")),
    _prefix(getParam<std::string>("prefix"))
{
}

std::unique_ptr<MeshBase>
ContactPairLowerDBlockGenerator::generate()
{
  auto mesh = std::move(_input);

  std::vector<std::pair<BoundaryName, BoundaryName>> pairs;
  if (_pairing_method == "NODE")
    pairs = findPairsNodeProximity(*mesh, _pairing_boundaries, _pairing_distance);
  else
    pairs = findPairsCentroid(*mesh, _pairing_boundaries, _pairing_distance);

  if (pairs.empty())
    mooseError("ContactPairLowerDBlockGenerator '",
               name(),
               "': no contact pairs found within distance ",
               _pairing_distance,
               " among boundaries ",
               Moose::stringify(_pairing_boundaries));

  const bool multiple_pairs = pairs.size() > 1;
  for (const auto & pair : pairs)
  {
    const std::string suffix = multiple_pairs ? "_" + pair.first + "_" + pair.second : "";
    const std::string primary_name = _prefix + "_primary_subdomain" + suffix;
    const std::string secondary_name = _prefix + "_secondary_subdomain" + suffix;

    MooseMeshUtils::createSubdomainFromSidesets(
        *mesh, {pair.first}, MooseMeshUtils::getNextFreeSubdomainID(*mesh), primary_name);
    MooseMeshUtils::createSubdomainFromSidesets(
        *mesh, {pair.second}, MooseMeshUtils::getNextFreeSubdomainID(*mesh), secondary_name);
  }

  return mesh;
}

std::vector<std::pair<BoundaryName, BoundaryName>>
ContactPairLowerDBlockGenerator::findPairsNodeProximity(
    MeshBase & mesh, const std::vector<BoundaryName> & boundaries, Real distance)
{
  if (!mesh.is_serial())
    ::mooseError("Automatic contact pair detection requires a serial mesh.");

  // Map boundary names to IDs
  std::vector<BoundaryID> boundary_ids;
  boundary_ids.reserve(boundaries.size());
  for (const auto & bname : boundaries)
    boundary_ids.push_back(MooseMeshUtils::getBoundaryID(bname, mesh));

  // Collect nodes on candidate boundaries
  std::vector<NodeBoundaryIDInfo> node_bid_list;
  for (const auto & [node_id, bid] : mesh.get_boundary_info().build_node_list())
  {
    auto it = std::find(boundary_ids.begin(), boundary_ids.end(), bid);
    if (it != boundary_ids.end())
      node_bid_list.emplace_back(mesh.node_ptr(node_id), bid);
  }

  // Sort by boundary id for deterministic ordering
  std::sort(node_bid_list.begin(),
            node_bid_list.end(),
            [](const NodeBoundaryIDInfo & a, const NodeBoundaryIDInfo & b)
            { return a.second < b.second; });

  using KDTreeType = nanoflann::KDTreeSingleIndexAdaptor<
      nanoflann::L2_Simple_Adaptor<Real, PointListAdaptor<NodeBoundaryIDInfo>, Real, std::size_t>,
      PointListAdaptor<NodeBoundaryIDInfo>,
      LIBMESH_DIM,
      std::size_t>;

  const unsigned int max_leaf_size = 20;
  auto point_list =
      PointListAdaptor<NodeBoundaryIDInfo>(node_bid_list.begin(), node_bid_list.end());
  auto kd_tree = std::make_unique<KDTreeType>(
      LIBMESH_DIM, point_list, nanoflann::KDTreeSingleIndexAdaptorParams(max_leaf_size));
  kd_tree->buildIndex();

  nanoflann::SearchParameters search_params;
  std::vector<nanoflann::ResultItem<std::size_t, Real>> ret_matches;
  std::vector<std::pair<BoundaryName, BoundaryName>> pairs;

  for (const auto & entry : node_bid_list)
  {
    ret_matches.clear();
    const Point search_point = *entry.first;
    kd_tree->radiusSearch(&search_point(0), distance * distance, ret_matches, search_params);

    for (const auto & match_item : ret_matches)
    {
      const auto & match = node_bid_list[match_item.first];

      // Skip nodes on the same boundary
      if (match.second == entry.second)
        continue;

      auto it_match = std::find(boundary_ids.begin(), boundary_ids.end(), match.second);
      if (it_match == boundary_ids.end())
        continue;

      auto it_entry = std::find(boundary_ids.begin(), boundary_ids.end(), entry.second);
      mooseAssert(it_entry != boundary_ids.end(), "Entry boundary not in candidate list");

      const auto idx_match = cast_int<int>(it_match - boundary_ids.begin());
      const auto idx_entry = cast_int<int>(it_entry - boundary_ids.begin());

      // Assign primary/secondary such that primary has the larger boundary id
      if (entry.second > match.second)
        pairs.push_back({boundaries[idx_entry], boundaries[idx_match]});
      else
        pairs.push_back({boundaries[idx_match], boundaries[idx_entry]});
    }
  }

  removeDuplicatePairs(pairs);
  return pairs;
}

std::vector<std::pair<BoundaryName, BoundaryName>>
ContactPairLowerDBlockGenerator::findPairsCentroid(MeshBase & mesh,
                                                   const std::vector<BoundaryName> & boundaries,
                                                   Real distance)
{
  if (!mesh.is_serial())
    ::mooseError("Automatic contact pair detection requires a serial mesh.");

  // Group side list by boundary id
  std::map<boundary_id_type, std::vector<std::pair<dof_id_type, unsigned short>>> bnd_sides;
  for (const auto & [eid, side, bid] : mesh.get_boundary_info().build_side_list())
    bnd_sides[bid].emplace_back(eid, side);

  // Compute center of gravity for each candidate boundary
  std::vector<std::pair<BoundaryName, Point>> boundary_cogs;
  for (const auto & bname : boundaries)
  {
    const BoundaryID bid = MooseMeshUtils::getBoundaryID(bname, mesh);
    auto it = bnd_sides.find(bid);
    if (it == bnd_sides.end())
      ::mooseError("Boundary '", bname, "' not found in mesh.");

    Point cog(0, 0, 0);
    Real total_area = 0;
    std::unique_ptr<const Elem> side_ptr;
    for (const auto & [eid, side] : it->second)
    {
      const Elem * elem = mesh.elem_ptr(eid);
      elem->side_ptr(side_ptr, side);
      const Real area = side_ptr->volume();
      cog += side_ptr->true_centroid() * area;
      total_area += area;
    }
    cog /= total_area;
    boundary_cogs.emplace_back(bname, cog);
  }

  // Find all pairs within distance
  std::vector<std::pair<BoundaryName, BoundaryName>> pairs;
  for (std::size_t i = 0; i < boundary_cogs.size(); ++i)
    for (std::size_t j = i + 1; j < boundary_cogs.size(); ++j)
    {
      const Real dist = (boundary_cogs[i].second - boundary_cogs[j].second).norm();
      if (dist <= distance)
      {
        const BoundaryID bid_i = MooseMeshUtils::getBoundaryID(boundary_cogs[i].first, mesh);
        const BoundaryID bid_j = MooseMeshUtils::getBoundaryID(boundary_cogs[j].first, mesh);
        // Primary gets the larger boundary id
        if (bid_i > bid_j)
          pairs.push_back({boundary_cogs[i].first, boundary_cogs[j].first});
        else
          pairs.push_back({boundary_cogs[j].first, boundary_cogs[i].first});
      }
    }

  removeDuplicatePairs(pairs);
  return pairs;
}

void
ContactPairLowerDBlockGenerator::removeDuplicatePairs(
    std::vector<std::pair<BoundaryName, BoundaryName>> & pairs)
{
  std::vector<std::pair<BoundaryName, BoundaryName>> unique_pairs;
  for (const auto & [primary, secondary] : pairs)
  {
    auto it = std::find_if(
        unique_pairs.begin(),
        unique_pairs.end(),
        [&, p = primary, s = secondary](const auto & q)
        { return (q.first == p && q.second == s) || (q.first == s && q.second == p); });
    if (it == unique_pairs.end())
      unique_pairs.emplace_back(primary, secondary);
  }
  pairs = std::move(unique_pairs);
}
