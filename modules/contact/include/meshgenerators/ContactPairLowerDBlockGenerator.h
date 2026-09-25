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
 * Detects contact surface pairs from a list of candidate boundaries using proximity
 * and creates lower-dimensional subdomain blocks for each pair, suitable for use
 * in mortar contact via ContactAction.
 */
class ContactPairLowerDBlockGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();
  ContactPairLowerDBlockGenerator(const InputParameters & parameters);
  std::unique_ptr<MeshBase> generate() override;

  /// Find pairs by node-proximity KD-tree search; returns deduplicated pairs
  static std::vector<std::pair<BoundaryName, BoundaryName>> findPairsNodeProximity(
      MeshBase & mesh, const std::vector<BoundaryName> & boundaries, Real distance);

  /// Find pairs by sideset centroid distance; returns deduplicated pairs
  static std::vector<std::pair<BoundaryName, BoundaryName>>
  findPairsCentroid(MeshBase & mesh, const std::vector<BoundaryName> & boundaries, Real distance);

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

  /// A candidate contact boundary with its sideset area and area-weighted centroid
  struct CandidateBoundary
  {
    BoundaryName name;
    BoundaryID id;
    Real area;
    Point centroid;
  };

  /**
   * Resolve the candidate boundary names to boundary IDs, erroring if any boundary is listed more
   * than once, and compute the area and centroid of each candidate sideset. Areas and centroids are
   * computed in Cartesian coordinates.
   */
  static std::vector<CandidateBoundary>
  candidateBoundaries(const MeshBase & mesh, const std::vector<BoundaryName> & boundaries);

  /// Order two paired boundaries as (primary, secondary), with the larger surface as primary
  static std::pair<BoundaryName, BoundaryName> orderPair(const CandidateBoundary & a,
                                                         const CandidateBoundary & b);

  static void removeDuplicatePairs(std::vector<std::pair<BoundaryName, BoundaryName>> & pairs);
};
