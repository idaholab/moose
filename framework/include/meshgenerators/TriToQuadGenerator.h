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
#include "MooseEnum.h"

#include <array>
#include <vector>

/**
 * This TriToQuadGenerator object converts a mesh made of TRI3 elements into a mesh made of QUAD4
 * elements, either by splitting every triangle into three quadrilaterals or by merging pairs of
 * adjacent triangles into quadrilaterals.
 */
class TriToQuadGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  TriToQuadGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  /// The four corners of one quadrilateral, as indices into the point list of a template
  using QuadCorners = std::array<unsigned int, 4>;

  /**
   * A pair of adjacent triangles that the recombination algorithm can merge into one
   * quadrilateral.
   */
  struct RecombineCandidate
  {
    /// Quality score of the quadrilateral that the two triangles would form
    Real eta;
    /// Id of the lower numbered triangle of the pair
    dof_id_type first_elem_id;
    /// Id of the higher numbered triangle of the pair
    dof_id_type second_elem_id;
    /// Node ids of the quadrilateral, in counter-clockwise order
    std::array<dof_id_type, 4> quad_node_ids;
  };

  /**
   * Compute the quality score of a planar quadrilateral,
   * eta = max(0, 1 - (2 / pi) * max_k |pi / 2 - alpha_k|), in which alpha_1 to alpha_4 are its
   * internal angles. A perfect rectangle scores 1. A degenerate or non-convex quadrilateral, which
   * is one with an internal angle of pi or more, scores 0.
   * @param quad_points The four corners of the quadrilateral, in counter-clockwise order
   * @return The quality score, between 0 and 1
   */
  static Real quadQuality(const std::array<Point, 4> & quad_points);

  /**
   * Select the pairs of triangles to merge, taking the highest scoring candidates first and
   * consuming each triangle at most once.
   * @param candidates The candidate pairs, sorted in place by decreasing score with ties broken by
   * increasing element id
   * @param eta_min The score below which a candidate is never selected
   * @return The selected candidates, in the order in which they were selected
   */
  static std::vector<RecombineCandidate>
  greedyMatching(std::vector<RecombineCandidate> & candidates, const Real eta_min);

  /**
   * The three quadrilaterals that a triangle is split into, one per corner, each of them built on
   * that corner, the midpoints of the two sides the corner is an end of, and the centroid.
   * @return The quadrilaterals, as indices into a point list holding the three corners at 0 to 2,
   * the midpoint of the side running from corner s to corner s + 1 at 3 + s, and the centroid at 6
   */
  static std::vector<QuadCorners> triSubdivisionTemplate();

  /**
   * The four quadrilaterals that a quadrilateral is split into, one per corner, each of them built
   * on that corner, the midpoints of the two sides the corner is an end of, and the centroid.
   * @return The quadrilaterals, as indices into a point list holding the four corners at 0 to 3,
   * the midpoint of the side running from corner s to corner s + 1 at 4 + s, and the centroid at 8
   */
  static std::vector<QuadCorners> quadSubdivisionTemplate();

protected:
  /// Mesh that possibly comes from another generator
  std::unique_ptr<MeshBase> & _input;
  /// Algorithm used to build the quadrilaterals
  const MooseEnum _algorithm;
  /// Algorithm used to pair adjacent triangles during recombination
  const MooseEnum _matching;
  /// Score below which a pair of adjacent triangles is not recombined
  const Real _eta_min;
  /// Whether the triangles that survive recombination are moved into their own subdomain
  const bool _has_tri_subdomain;
  /// Name of the subdomain that the triangles surviving recombination are moved into
  const SubdomainName _tri_subdomain_name;
  /// Whether the triangles that survive recombination are eliminated
  const bool _all_quad;

private:
  /**
   * Build the merge candidate for the two triangles that share a side.
   * @param elem One of the two triangles
   * @param side The index, local to \p elem, of the shared side
   * @param neighbor The triangle on the other side of the shared side
   * @return The candidate formed by that pair
   */
  static RecombineCandidate
  buildCandidate(const Elem & elem, const unsigned int side, const Elem & neighbor);

  /**
   * Replace every element of the mesh by one quadrilateral per corner, built on its centroid and
   * its edge midpoints, which turns a triangle into three quadrilaterals and a quadrilateral into
   * four. Every side of the mesh is split at its midpoint, so the elements on both sides of a side
   * split it at the same point and the result is conformal.
   * @param mesh The mesh to convert
   */
  void subdivide(ReplicatedMesh & mesh) const;

  /**
   * Replace the highest scoring pairs of adjacent triangles of the mesh by quadrilaterals.
   * @param mesh The mesh to convert
   */
  void recombine(ReplicatedMesh & mesh) const;

  /**
   * Move the triangles that recombination did not consume into the subdomain named by the
   * tri_subdomain_name parameter.
   * @param mesh The mesh being converted
   * @param scratch_subdomain_id The subdomain holding the triangles that recombination consumed
   */
  void moveSurvivingTriangles(ReplicatedMesh & mesh,
                              const subdomain_id_type scratch_subdomain_id) const;
};
