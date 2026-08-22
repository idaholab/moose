//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"
#include "PolygonAreaTestUtils.h"
#include "TriToQuadGenerator.h"

#include "libmesh/int_range.h"
#include "libmesh/point.h"

#include <array>
#include <cmath>
#include <set>
#include <vector>

using RecombineCandidate = TriToQuadGenerator::RecombineCandidate;

namespace
{

/// Tolerance for the scores, which come out of an arc tangent and so are not exact
constexpr Real tol = 1e-12;

using PolygonAreaTestUtils::twiceSignedArea;

/**
 * A candidate carrying only the fields greedyMatching() acts on.
 * @param eta the score of the pair
 * @param first_elem_id the lower numbered triangle of the pair
 * @param second_elem_id the higher numbered triangle of the pair
 * @return the candidate
 */
RecombineCandidate
makeCandidate(const Real eta, const dof_id_type first_elem_id, const dof_id_type second_elem_id)
{
  RecombineCandidate candidate;
  candidate.eta = eta;
  candidate.first_elem_id = first_elem_id;
  candidate.second_elem_id = second_elem_id;
  // greedyMatching() never reads the node ids, so they are only filled to leave nothing unset
  candidate.quad_node_ids.fill(0);

  return candidate;
}

/**
 * Whether no triangle is claimed by more than one of the selected pairs, which is what makes the
 * selection a valid matching.
 * @param selected the pairs greedyMatching() returned
 * @return whether every element id appears at most once
 */
bool
elementsAreDisjoint(const std::vector<RecombineCandidate> & selected)
{
  std::set<dof_id_type> seen;
  for (const auto & candidate : selected)
    if (!seen.insert(candidate.first_elem_id).second ||
        !seen.insert(candidate.second_elem_id).second)
      return false;

  return true;
}

} // namespace

/**
 * eta is 1 for a quadrilateral whose four internal angles are all pi / 2, which is what the two
 * triangles of a rectangle merge into. The metric is built on angles alone, so stretching the
 * rectangle leaves the score at 1.
 */
TEST(TriToQuadScoringTest, rectangleScoresOne)
{
  const std::array<Point, 4> unit_square = {
      Point(0.0, 0.0), Point(1.0, 0.0), Point(1.0, 1.0), Point(0.0, 1.0)};
  ASSERT_GT(twiceSignedArea(unit_square), 0.0);
  EXPECT_NEAR(1.0, TriToQuadGenerator::quadQuality(unit_square), tol);

  const std::array<Point, 4> stretched = {
      Point(0.0, 0.0), Point(3.0, 0.0), Point(3.0, 1.0), Point(0.0, 1.0)};
  ASSERT_GT(twiceSignedArea(stretched), 0.0);
  EXPECT_NEAR(1.0, TriToQuadGenerator::quadQuality(stretched), tol);
}

/**
 * eta is exactly 0 for a quadrilateral with an internal angle of pi or more. The arrowhead below
 * has its tip at (2, 3), its two barbs at the ends of the base, and its reflex corner at the notch
 * (2, 1) between them.
 *
 * The counter-clockwise precondition is asserted because it is what makes this test meaningful:
 * every corner of a clockwise quadrilateral looks reflex to quadQuality(), so clockwise input would
 * score 0 no matter what shape it described.
 */
TEST(TriToQuadScoringTest, nonConvexScoresZero)
{
  const std::array<Point, 4> dart = {
      Point(0.0, 0.0), Point(2.0, 1.0), Point(4.0, 0.0), Point(2.0, 3.0)};
  ASSERT_GT(twiceSignedArea(dart), 0.0);

  EXPECT_DOUBLE_EQ(0.0, TriToQuadGenerator::quadQuality(dart));
}

/**
 * A rhombus with internal angles of 60 and 120 degrees is off a right angle by pi / 6 at every
 * corner, so eta is 1 - (2 / pi) * (pi / 6) = 2 / 3. Pinning an intermediate score keeps the metric
 * from collapsing into a convexity flag that only ever returns 0 or 1.
 */
TEST(TriToQuadScoringTest, rhombusScoresTwoThirds)
{
  const Real half_root_three = std::sqrt(3.0) / 2.0;
  const std::array<Point, 4> rhombus = {
      Point(0.0, 0.0), Point(1.0, 0.0), Point(1.5, half_root_three), Point(0.5, half_root_three)};
  ASSERT_GT(twiceSignedArea(rhombus), 0.0);

  EXPECT_NEAR(2.0 / 3.0, TriToQuadGenerator::quadQuality(rhombus), tol);
}

/**
 * greedyMatching() never selects a pair scoring below eta_min, and does select one scoring exactly
 * eta_min. No two pairs here share a triangle, so the threshold is the only thing that can leave a
 * pair out.
 */
TEST(TriToQuadScoringTest, matchingRejectsBelowEtaMin)
{
  const Real eta_min = 0.5;
  std::vector<RecombineCandidate> candidates = {makeCandidate(0.4, 2, 3),
                                                makeCandidate(0.9, 0, 1),
                                                makeCandidate(0.1, 6, 7),
                                                makeCandidate(0.5, 8, 9),
                                                makeCandidate(0.6, 4, 5)};

  const auto selected = TriToQuadGenerator::greedyMatching(candidates, eta_min);

  ASSERT_EQ(3u, selected.size());
  for (const auto & candidate : selected)
    EXPECT_GE(candidate.eta, eta_min);

  // The pairs scoring 0.4 and 0.1 are gone; the one scoring exactly eta_min is kept, last
  EXPECT_EQ(0u, selected[0].first_elem_id);
  EXPECT_EQ(4u, selected[1].first_elem_id);
  EXPECT_EQ(8u, selected[2].first_elem_id);
}

/**
 * On a strip of four triangles the pairs compete for the triangles they share. greedyMatching()
 * takes the highest scoring pair first and then skips every later pair that needs a triangle
 * already spent.
 */
TEST(TriToQuadScoringTest, matchingConsumesEachTriangleOnce)
{
  // Triangles 0 to 3 in a strip, so the pairs that can merge are (0, 1), (1, 2) and (2, 3)
  std::vector<RecombineCandidate> candidates = {
      makeCandidate(0.85, 1, 2), makeCandidate(0.9, 0, 1), makeCandidate(0.8, 2, 3)};

  // An eta_min of 0 leaves the consumption rule as the only reason a pair can be left out
  const auto selected = TriToQuadGenerator::greedyMatching(candidates, 0.0);

  ASSERT_EQ(2u, selected.size());
  EXPECT_TRUE(elementsAreDisjoint(selected));

  // The highest scoring pair is taken first
  EXPECT_EQ(0u, selected[0].first_elem_id);
  EXPECT_EQ(1u, selected[0].second_elem_id);

  // (1, 2) scores next but needs triangle 1, so (2, 3) is taken ahead of it
  EXPECT_EQ(2u, selected[1].first_elem_id);
  EXPECT_EQ(3u, selected[1].second_elem_id);
}

/**
 * Equal scores are resolved by increasing element id, on the lower id of the pair first and then on
 * the higher one, so that the same pairs are merged from one run to the next. All four pairs below
 * score the same and two of them share triangle 2, so the tie-break is what decides which of those
 * two is merged.
 */
TEST(TriToQuadScoringTest, matchingBreaksTiesByElementId)
{
  const Real eta = 0.8;
  const Real eta_min = 0.5;
  std::vector<RecombineCandidate> candidates = {makeCandidate(eta, 4, 5),
                                                makeCandidate(eta, 2, 7),
                                                makeCandidate(eta, 2, 3),
                                                makeCandidate(eta, 0, 9)};

  const auto selected = TriToQuadGenerator::greedyMatching(candidates, eta_min);

  // greedyMatching() sorts its argument in place, by decreasing score with ties broken by
  // increasing element id
  ASSERT_EQ(4u, candidates.size());
  EXPECT_EQ(0u, candidates[0].first_elem_id);
  EXPECT_EQ(2u, candidates[1].first_elem_id);
  EXPECT_EQ(3u, candidates[1].second_elem_id);
  EXPECT_EQ(2u, candidates[2].first_elem_id);
  EXPECT_EQ(7u, candidates[2].second_elem_id);
  EXPECT_EQ(4u, candidates[3].first_elem_id);

  // (2, 3) beats (2, 7) on the higher id of the pair, so (2, 7) is the pair left out
  ASSERT_EQ(3u, selected.size());
  EXPECT_TRUE(elementsAreDisjoint(selected));
  EXPECT_EQ(0u, selected[0].first_elem_id);
  EXPECT_EQ(9u, selected[0].second_elem_id);
  EXPECT_EQ(2u, selected[1].first_elem_id);
  EXPECT_EQ(3u, selected[1].second_elem_id);
  EXPECT_EQ(4u, selected[2].first_elem_id);
  EXPECT_EQ(5u, selected[2].second_elem_id);

  // The same pairs handed in in a different order give the same selection, which is what makes the
  // meshes the generator writes reproducible
  std::vector<RecombineCandidate> shuffled = {makeCandidate(eta, 2, 3),
                                              makeCandidate(eta, 0, 9),
                                              makeCandidate(eta, 4, 5),
                                              makeCandidate(eta, 2, 7)};

  const auto shuffled_selected = TriToQuadGenerator::greedyMatching(shuffled, eta_min);

  ASSERT_EQ(selected.size(), shuffled_selected.size());
  for (const auto k : index_range(selected))
  {
    EXPECT_EQ(selected[k].first_elem_id, shuffled_selected[k].first_elem_id);
    EXPECT_EQ(selected[k].second_elem_id, shuffled_selected[k].second_elem_id);
  }
}
