
#include "TriSubChannelMesh.h"

#include "gtest/gtest.h"
#include <algorithm>

TEST(MeshTests, rodCoordinates)
{

  int nrings = 4;
  Real pitch = 1;
  Point center(0, 0, 0);

  std::vector<Point> positions;
  rodPositions(positions, nrings, pitch, center);
  std::sort(positions.begin(), positions.end());

  std::vector<Point> positions2;
  rodPositions2(positions2, nrings, pitch, center);
  std::sort(positions2.begin(), positions2.end());

  ASSERT_EQ(positions.size(), positions2.size());

  for (int i = 0; i < positions.size(); i++)
    if (!positions[i].absolute_fuzzy_equals(positions2[i]))
      FAIL() << "point " << i - 1 << " differs: " << positions[i] << " != " << positions2[i];
}
