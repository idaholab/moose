
#include "TriSubChannelMesh.h"

#include "gtest/gtest.h"
#include <algorithm>
#include <sstream>

const double tol = 1e-8;
const double eps = 1e-9;

bool
relativeEq(double x, double y)
{
  if (std::abs(x) < eps && std::abs(y) < eps)
    return true;

  float diff = fabs(x - y);
  x = std::abs(x);
  y = std::abs(y);
  float largest = (y > x) ? y : x;
  return diff <= largest * tol;
}

bool
pointLess(const Point & a, const Point & b)
{
  for (int i = 0; i < LIBMESH_DIM; i++)
  {
    if (relativeEq(a(i), b(i)))
      continue;
    return a(i) < b(i);
  }
  return false;
}

TEST(MeshTests, rodCoordinates)
{

  int nrings = 4;
  Real pitch = 1;
  Point center(0, 0, 0);

  std::vector<Point> positions;
  rodPositions(positions, nrings, pitch, center);
  std::sort(positions.begin(), positions.end(), pointLess);

  std::vector<Point> positions2;
  rodPositions2(positions2, nrings, pitch, center);
  std::sort(positions2.begin(), positions2.end(), pointLess);

  ASSERT_EQ(positions.size(), positions2.size());

  std::stringstream msg;
  for (int i = 0; i < positions.size(); i++)
    if (!positions[i].absolute_fuzzy_equals(positions2[i]))
      msg << "point " << i + 1 << " differs: " << positions[i] << " != " << positions2[i] << "\n";
  ;

  if (msg.str().size() > 0)
    FAIL() << msg.str();
}
