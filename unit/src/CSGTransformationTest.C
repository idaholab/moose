//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "CSGTransformationHelper.h"

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
    ASSERT_FALSE(CSGTransformationHelper::isValidTransformationValue(TransformationType::SCALE,
                                                                     invalid_scale_values));
  }
}

/// tests that the string representation of transformation types is correct and that the conversion function properly converts a vector of transformation pairs to string representations for types
TEST(CSGTransformationTest, testGetStrings)
{
  std::vector<TransformationType> transformations = {
      TransformationType::TRANSLATION, TransformationType::ROTATION, TransformationType::SCALE};
  std::vector<std::string> exp_strings = {"TRANSLATION", "ROTATION", "SCALE"};
  for (size_t i = 0; i < transformations.size(); ++i)
    ASSERT_EQ(CSGTransformationHelper::getTransformationTypeString(transformations[i]),
              exp_strings[i]);
}
}
