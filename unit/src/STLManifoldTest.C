//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "STLManifold.h"
#include "MooseUnitUtils.h"

#include "gtest/gtest.h"

#include <array>
#include <cstdint>
#include <fstream>

namespace
{
void
writeCubeASCII(const std::filesystem::path & path)
{
  std::ofstream out(path);
  out << R"STL(solid cube
  facet normal 0 0 -1
    outer loop
      vertex -0.25 -0.25 -0.25
      vertex 0.25 -0.25 -0.25
      vertex 0.25 0.25 -0.25
    endloop
  endfacet
  facet normal 0 0 -1
    outer loop
      vertex -0.25 -0.25 -0.25
      vertex 0.25 0.25 -0.25
      vertex -0.25 0.25 -0.25
    endloop
  endfacet
  facet normal 0 0 1
    outer loop
      vertex -0.25 -0.25 0.25
      vertex 0.25 0.25 0.25
      vertex 0.25 -0.25 0.25
    endloop
  endfacet
  facet normal 0 0 1
    outer loop
      vertex -0.25 -0.25 0.25
      vertex -0.25 0.25 0.25
      vertex 0.25 0.25 0.25
    endloop
  endfacet
  facet normal 0 -1 0
    outer loop
      vertex -0.25 -0.25 -0.25
      vertex 0.25 -0.25 0.25
      vertex 0.25 -0.25 -0.25
    endloop
  endfacet
  facet normal 0 -1 0
    outer loop
      vertex -0.25 -0.25 -0.25
      vertex -0.25 -0.25 0.25
      vertex 0.25 -0.25 0.25
    endloop
  endfacet
  facet normal 0 1 0
    outer loop
      vertex -0.25 0.25 -0.25
      vertex 0.25 0.25 -0.25
      vertex 0.25 0.25 0.25
    endloop
  endfacet
  facet normal 0 1 0
    outer loop
      vertex -0.25 0.25 -0.25
      vertex 0.25 0.25 0.25
      vertex -0.25 0.25 0.25
    endloop
  endfacet
  facet normal -1 0 0
    outer loop
      vertex -0.25 -0.25 -0.25
      vertex -0.25 0.25 -0.25
      vertex -0.25 0.25 0.25
    endloop
  endfacet
  facet normal -1 0 0
    outer loop
      vertex -0.25 -0.25 -0.25
      vertex -0.25 0.25 0.25
      vertex -0.25 -0.25 0.25
    endloop
  endfacet
  facet normal 1 0 0
    outer loop
      vertex 0.25 -0.25 -0.25
      vertex 0.25 0.25 0.25
      vertex 0.25 0.25 -0.25
    endloop
  endfacet
  facet normal 1 0 0
    outer loop
      vertex 0.25 -0.25 -0.25
      vertex 0.25 -0.25 0.25
      vertex 0.25 0.25 0.25
    endloop
  endfacet
endsolid cube
)STL";
}

void
writeOpenCubeASCII(const std::filesystem::path & path)
{
  std::ofstream out(path);
  out << R"STL(solid open_cube
  facet normal 0 0 -1
    outer loop
      vertex -0.25 -0.25 -0.25
      vertex 0.25 -0.25 -0.25
      vertex 0.25 0.25 -0.25
    endloop
  endfacet
  facet normal 0 0 -1
    outer loop
      vertex -0.25 -0.25 -0.25
      vertex 0.25 0.25 -0.25
      vertex -0.25 0.25 -0.25
    endloop
  endfacet
  facet normal 0 -1 0
    outer loop
      vertex -0.25 -0.25 -0.25
      vertex 0.25 -0.25 0.25
      vertex 0.25 -0.25 -0.25
    endloop
  endfacet
  facet normal 0 -1 0
    outer loop
      vertex -0.25 -0.25 -0.25
      vertex -0.25 -0.25 0.25
      vertex 0.25 -0.25 0.25
    endloop
  endfacet
  facet normal 0 1 0
    outer loop
      vertex -0.25 0.25 -0.25
      vertex 0.25 0.25 -0.25
      vertex 0.25 0.25 0.25
    endloop
  endfacet
  facet normal 0 1 0
    outer loop
      vertex -0.25 0.25 -0.25
      vertex 0.25 0.25 0.25
      vertex -0.25 0.25 0.25
    endloop
  endfacet
  facet normal -1 0 0
    outer loop
      vertex -0.25 -0.25 -0.25
      vertex -0.25 0.25 -0.25
      vertex -0.25 0.25 0.25
    endloop
  endfacet
  facet normal -1 0 0
    outer loop
      vertex -0.25 -0.25 -0.25
      vertex -0.25 0.25 0.25
      vertex -0.25 -0.25 0.25
    endloop
  endfacet
  facet normal 1 0 0
    outer loop
      vertex 0.25 -0.25 -0.25
      vertex 0.25 0.25 0.25
      vertex 0.25 0.25 -0.25
    endloop
  endfacet
  facet normal 1 0 0
    outer loop
      vertex 0.25 -0.25 -0.25
      vertex 0.25 -0.25 0.25
      vertex 0.25 0.25 0.25
    endloop
  endfacet
endsolid open_cube
)STL";
}

void
writeBinaryCube(const std::filesystem::path & path)
{
  struct TriangleData
  {
    float coords[12];
  };

  const TriangleData triangles[] = {
      {{0, 0, -1, -0.25f, -0.25f, -0.25f, 0.25f, -0.25f, -0.25f, 0.25f, 0.25f, -0.25f}},
      {{0, 0, -1, -0.25f, -0.25f, -0.25f, 0.25f, 0.25f, -0.25f, -0.25f, 0.25f, -0.25f}},
      {{0, 0, 1, -0.25f, -0.25f, 0.25f, 0.25f, 0.25f, 0.25f, 0.25f, -0.25f, 0.25f}},
      {{0, 0, 1, -0.25f, -0.25f, 0.25f, -0.25f, 0.25f, 0.25f, 0.25f, 0.25f, 0.25f}},
      {{0, -1, 0, -0.25f, -0.25f, -0.25f, 0.25f, -0.25f, 0.25f, 0.25f, -0.25f, -0.25f}},
      {{0, -1, 0, -0.25f, -0.25f, -0.25f, -0.25f, -0.25f, 0.25f, 0.25f, -0.25f, 0.25f}},
      {{0, 1, 0, -0.25f, 0.25f, -0.25f, 0.25f, 0.25f, -0.25f, 0.25f, 0.25f, 0.25f}},
      {{0, 1, 0, -0.25f, 0.25f, -0.25f, 0.25f, 0.25f, 0.25f, -0.25f, 0.25f, 0.25f}},
      {{-1, 0, 0, -0.25f, -0.25f, -0.25f, -0.25f, 0.25f, -0.25f, -0.25f, 0.25f, 0.25f}},
      {{-1, 0, 0, -0.25f, -0.25f, -0.25f, -0.25f, 0.25f, 0.25f, -0.25f, -0.25f, 0.25f}},
      {{1, 0, 0, 0.25f, -0.25f, -0.25f, 0.25f, 0.25f, 0.25f, 0.25f, 0.25f, -0.25f}},
      {{1, 0, 0, 0.25f, -0.25f, -0.25f, 0.25f, -0.25f, 0.25f, 0.25f, 0.25f, 0.25f}},
  };

  std::ofstream out(path, std::ios::binary);
  std::array<char, 80> header = {{0}};
  out.write(header.data(), header.size());
  const std::uint32_t triangle_count = std::size(triangles);
  out.write(reinterpret_cast<const char *>(&triangle_count), sizeof(triangle_count));
  for (const auto & triangle : triangles)
  {
    out.write(reinterpret_cast<const char *>(triangle.coords), sizeof(triangle.coords));
    const std::uint16_t attribute_count = 0;
    out.write(reinterpret_cast<const char *>(&attribute_count), sizeof(attribute_count));
  }
}

void
writeDegenerateASCII(const std::filesystem::path & path)
{
  std::ofstream out(path);
  out << R"STL(solid degenerate
  facet normal 0 0 1
    outer loop
      vertex 0 0 0
      vertex 0 0 0
      vertex 1 0 0
    endloop
  endfacet
endsolid degenerate
)STL";
}
}

TEST(STLManifold, asciiParseContainmentAndFallback)
{
  Moose::UnitUtils::TempFile temp_file;
  writeCubeASCII(temp_file.path());

  STLManifold manifold(temp_file.path().string(),
                       RealVectorValue(1, 1, 1),
                       RealVectorValue(0, 0, 0),
                       RealVectorValue(0, 0, 0),
                       1e-10);

  EXPECT_EQ(manifold.numTriangles(), 12);
  EXPECT_NEAR(manifold.boundingBox().min()(0), -0.25, 1e-12);
  EXPECT_NEAR(manifold.boundingBox().max()(2), 0.25, 1e-12);
  EXPECT_TRUE(manifold.contains(Point(0.0, 0.0, 0.0)));
  EXPECT_TRUE(manifold.contains(Point(0.25, 0.0, 0.0)));
  EXPECT_FALSE(manifold.contains(Point(0.5, 0.0, 0.0)));
}

TEST(STLManifold, binaryParse)
{
  Moose::UnitUtils::TempFile temp_file;
  writeBinaryCube(temp_file.path());

  STLManifold manifold(temp_file.path().string(),
                       RealVectorValue(1, 1, 1),
                       RealVectorValue(0, 0, 0),
                       RealVectorValue(0, 0, 0),
                       1e-10);

  EXPECT_EQ(manifold.numTriangles(), 12);
  EXPECT_TRUE(manifold.contains(Point(0.0, 0.0, 0.0)));
  EXPECT_FALSE(manifold.contains(Point(0.5, 0.0, 0.0)));
}

TEST(STLManifold, scaleRotateTranslate)
{
  Moose::UnitUtils::TempFile temp_file;
  writeCubeASCII(temp_file.path());

  STLManifold rotated(temp_file.path().string(),
                      RealVectorValue(1, 1, 1),
                      RealVectorValue(0, 0, 45),
                      RealVectorValue(0.5, 0.5, 0.5),
                      1e-10);
  STLManifold unrotated(temp_file.path().string(),
                        RealVectorValue(1, 1, 1),
                        RealVectorValue(0, 0, 0),
                        RealVectorValue(0.5, 0.5, 0.5),
                        1e-10);
  STLManifold scaled_rotated(temp_file.path().string(),
                             RealVectorValue(2, 1, 1),
                             RealVectorValue(0, 0, 90),
                             RealVectorValue(0.5, 0.5, 0.5),
                             1e-10);

  EXPECT_TRUE(rotated.contains(Point(0.5, 0.5, 0.5)));
  EXPECT_TRUE(rotated.contains(Point(0.625, 0.5, 0.5)));
  EXPECT_TRUE(rotated.contains(Point(0.8, 0.5, 0.5)));
  EXPECT_FALSE(unrotated.contains(Point(0.8, 0.5, 0.5)));
  EXPECT_FALSE(rotated.contains(Point(0.86, 0.5, 0.5)));

  EXPECT_TRUE(scaled_rotated.contains(Point(0.5, 0.85, 0.5)));
  EXPECT_FALSE(scaled_rotated.contains(Point(0.85, 0.5, 0.5)));
}

TEST(STLManifold, errors)
{
  {
    Moose::UnitUtils::TempFile temp_file;
    writeOpenCubeASCII(temp_file.path());
    EXPECT_MOOSEERROR_MSG_CONTAINS(STLManifold(temp_file.path().string(),
                                               RealVectorValue(1, 1, 1),
                                               RealVectorValue(0, 0, 0),
                                               RealVectorValue(0, 0, 0),
                                               1e-10),
                                   "watertight 2-manifold surface");
  }

  {
    Moose::UnitUtils::TempFile temp_file;
    writeDegenerateASCII(temp_file.path());
    EXPECT_MOOSEERROR_MSG_CONTAINS(STLManifold(temp_file.path().string(),
                                               RealVectorValue(1, 1, 1),
                                               RealVectorValue(0, 0, 0),
                                               RealVectorValue(0, 0, 0),
                                               1e-10),
                                   "degenerate triangle");
  }

  {
    Moose::UnitUtils::TempFile temp_file;
    writeCubeASCII(temp_file.path());
    EXPECT_MOOSEERROR_MSG_CONTAINS(STLManifold(temp_file.path().string(),
                                               RealVectorValue(0, 1, 1),
                                               RealVectorValue(0, 0, 0),
                                               RealVectorValue(0, 0, 0),
                                               1e-10),
                                   "scale components must be strictly positive");
  }
}
