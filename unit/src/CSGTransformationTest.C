//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "CSGTransformation.h"

#include "MooseUnitUtils.h"

namespace CSG
{

/// Test that invalid transformation values are properly handled by isValidTransformationValue
TEST(CSGTransformationTest, testInvalidValues)
{
  // note: valid values inherently tested in CSGBaseTest
  // test invalid values for scale transformation
  {
    std::tuple<Real, Real, Real> invalid_scale_values = {
        1.0, -2.0, 0.0}; // negative value is invalid for scale
    ASSERT_FALSE(isValidTransformationValue(TransformationType::SCALE, invalid_scale_values));
  }
}

/// tests that the string representation of transformation types is correct and that the conversion function properly converts a vector of transformation pairs to string representations for types
TEST(CSGTransformationTest, testConvertToString)
{
  std::vector<std::pair<TransformationType, std::tuple<Real, Real, Real>>> transformations = {
      {TransformationType::TRANSLATION, {1.0, 2.0, 3.0}},
      {TransformationType::ROTATION, {45.0, 30.0, 60.0}},
      {TransformationType::SCALE, {-2.0, 2.0, 2.0}}};
  std::vector<std::pair<std::string, std::tuple<Real, Real, Real>>> exp_strings = {
      {"TRANSLATION", {1.0, 2.0, 3.0}},
      {"ROTATION", {45.0, 30.0, 60.0}},
      {"SCALE", {-2.0, 2.0, 2.0}}};
  auto out_strings = convertTransformationsToString(transformations);
  ASSERT_EQ(out_strings, exp_strings);
}
}