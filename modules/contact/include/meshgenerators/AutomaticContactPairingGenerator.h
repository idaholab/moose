//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

/**
 * Detects contact surface pairs from a list of candidate boundaries using proximity, stores them
 * in the mesh meta-data, and optionally creates lower-dimensional subdomain blocks for each pair,
 * suitable for use in mortar contact via ContactAction.
 */
class AutomaticContactPairingGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();
  AutomaticContactPairingGenerator(const InputParameters & parameters);
  std::unique_ptr<MeshBase> generate() override;

  /// Type of the mesh meta-data holding the detected (primary, secondary) boundary name pairs
  typedef std::vector<std::pair<std::string, std::string>> ContactPairs;

  /// Name of the mesh meta-data holding the detected contact pairs
  static constexpr auto contact_pairs_property = "contact_pairs";

  /**
   * Name suffix identifying a (primary, secondary) boundary pair, used to build unique object
   * names when there is more than one contact pair
   */
  static std::string pairSuffix(const std::pair<BoundaryName, BoundaryName> & pair);

protected:
  std::unique_ptr<MeshBase> & _input;

private:
  /// Candidate boundaries to pair
  const std::vector<BoundaryName> & _pairing_boundaries;
  /// Maximum center-to-center or node-to-node distance for pairing
  const Real _pairing_distance;
  /// Pairing method: NODE or CENTROID
  const MooseEnum & _pairing_method;
  /// Prefix prepended to the names of generated subdomain blocks
  const std::string & _prefix;
  /// Whether to create lower-dimensional subdomain blocks for each detected pair
  const bool _create_lower_d_blocks;

  /// A candidate contact boundary with its sideset area and area-weighted centroid
  struct CandidateBoundary
  {
    BoundaryName name;
    BoundaryID id;
    Real area;
    Point centroid;
  };

  /// Find pairs by node-proximity KD-tree search; returns deduplicated pairs
  std::vector<std::pair<BoundaryName, BoundaryName>> findPairsNodeProximity(const MeshBase & mesh);

  /// Find pairs by sideset centroid distance; returns deduplicated pairs
  std::vector<std::pair<BoundaryName, BoundaryName>> findPairsCentroid(const MeshBase & mesh);

  /**
   * Resolve the candidate boundary names to boundary IDs, erroring if any boundary is listed more
   * than once, and compute the area and centroid of each candidate sideset. Areas and centroids are
   * computed in Cartesian coordinates.
   */
  std::vector<CandidateBoundary> candidateBoundaries(const MeshBase & mesh);

  /// Order two paired boundaries as (primary, secondary), with the larger surface as primary
  static std::pair<BoundaryName, BoundaryName> orderPair(const CandidateBoundary & a,
                                                         const CandidateBoundary & b);

  static void removeDuplicatePairs(std::vector<std::pair<BoundaryName, BoundaryName>> & pairs);
};
