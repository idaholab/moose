//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseObjectUnitTest.h"

#include "FVGeometricAverage.h"
#include "FVHarmonicAverage.h"

#include "gtest/gtest.h"

#include <cmath>

class FVInterpolationMethodTest : public MooseObjectUnitTest
{
public:
  FVInterpolationMethodTest() : MooseObjectUnitTest("MooseUnitApp")
  {
    _mesh->buildFiniteVolumeInfo();
    for (const auto * fi : _mesh->faceInfo())
      if (fi->neighborPtr())
      {
        _internal_face = fi;
        break;
      }

    mooseAssert(_internal_face, "Expected to find at least one internal face in the unit mesh");
  }

protected:
  const FaceInfo * _internal_face = nullptr;
};

TEST_F(FVInterpolationMethodTest, geometricAverageMatchesGC)
{
  InputParameters params = _factory.getValidParams("FVGeometricAverage");
  auto & method =
      addObject<FVGeometricAverage>("FVGeometricAverage", "geom_interp_method", params);

  const Real elem_value = 2.0;
  const Real neighbor_value = 10.0;
  const Real expected =
      _internal_face->gC() * elem_value + (1.0 - _internal_face->gC()) * neighbor_value;

  EXPECT_NEAR(method.faceInterpolator()(*_internal_face, elem_value, neighbor_value), expected, 1e-12);
}

TEST_F(FVInterpolationMethodTest, harmonicAverage)
{
  InputParameters params = _factory.getValidParams("FVHarmonicAverage");
  auto & method =
      addObject<FVHarmonicAverage>("FVHarmonicAverage", "harm_interp_method", params);

  const Real elem_value = 4.0;
  const Real neighbor_value = 1.0;

  const Real gc = _internal_face->gC();
  const Real expected = 1.0 / (gc / elem_value + (1.0 - gc) / neighbor_value);

  EXPECT_NEAR(method.faceInterpolator()(*_internal_face, elem_value, neighbor_value), expected, 1e-12);
}
