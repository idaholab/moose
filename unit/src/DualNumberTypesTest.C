//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseTypes.h"
#include <iostream>

using MetaPhysicL::DualNumber;
using MetaPhysicL::NDDualNumber;
using MetaPhysicL::NumberArray;

const double tol = 1e-8;

TEST(DualNumberTypesTest, Test)
{
  NDDualNumber<Real, NumberArray<2, Real>> scalar_ad_prop;
  NDDualNumber<VectorValue<Real>, NumberArray<2, VectorValue<Real>>> vector_ad_prop;
  NDDualNumber<TensorValue<Real>, NumberArray<2, TensorValue<Real>>> tensor_ad_prop;

  Real scalar_reg_prop;
  RealVectorValue vector_reg_prop;
  RealTensorValue tensor_reg_prop;

  scalar_ad_prop = 2.;
  scalar_ad_prop.derivatives()[0] = 1.;
  scalar_ad_prop.derivatives()[1] = 2.;
  scalar_ad_prop *= scalar_ad_prop;
  EXPECT_NEAR(scalar_ad_prop.value(), 4., tol);
  EXPECT_NEAR(scalar_ad_prop.derivatives()[0], 4., tol);
  EXPECT_NEAR(scalar_ad_prop.derivatives()[1], 8., tol);

  scalar_ad_prop = scalar_ad_prop * scalar_ad_prop;
  EXPECT_NEAR(scalar_ad_prop.value(), 16., tol);
  EXPECT_NEAR(scalar_ad_prop.derivatives()[0], 32., tol);
  EXPECT_NEAR(scalar_ad_prop.derivatives()[1], 64., tol);

  vector_ad_prop = scalar_ad_prop * RealVectorValue(1., 1., 1.);
  for (decltype(LIBMESH_DIM) i = 0; i != LIBMESH_DIM; ++i)
  {
    EXPECT_NEAR(vector_ad_prop.value()(i), 16., tol);
    EXPECT_NEAR(vector_ad_prop.derivatives()[0](i), 32., tol);
    EXPECT_NEAR(vector_ad_prop.derivatives()[1](i), 64., tol);
  }
  scalar_ad_prop = vector_ad_prop * vector_ad_prop;
  EXPECT_NEAR(scalar_ad_prop.value(), 768., tol);
  EXPECT_NEAR(scalar_ad_prop.derivatives()[0], 3072., tol);
  EXPECT_NEAR(scalar_ad_prop.derivatives()[1], 6144., tol);

  vector_ad_prop *= 3.;
  for (decltype(LIBMESH_DIM) i = 0; i != LIBMESH_DIM; ++i)
  {
    EXPECT_NEAR(vector_ad_prop.value()(i), 48., tol);
    EXPECT_NEAR(vector_ad_prop.derivatives()[0](i), 96., tol);
    EXPECT_NEAR(vector_ad_prop.derivatives()[1](i), 192., tol);
  }
  vector_ad_prop = 4. * vector_ad_prop + vector_ad_prop * 5.;
  for (decltype(LIBMESH_DIM) i = 0; i != LIBMESH_DIM; ++i)
  {
    EXPECT_NEAR(vector_ad_prop.value()(i), 432., tol);
    EXPECT_NEAR(vector_ad_prop.derivatives()[0](i), 864., tol);
    EXPECT_NEAR(vector_ad_prop.derivatives()[1](i), 1728., tol);
  }

  tensor_ad_prop = scalar_ad_prop * RealTensorValue(1., 1., 1., 1., 1., 1., 1., 1., 1.);
  for (decltype(LIBMESH_DIM) i = 0; i != LIBMESH_DIM; ++i)
  {
    for (decltype(LIBMESH_DIM) j = 0; j != LIBMESH_DIM; ++j)
    {
      EXPECT_NEAR(tensor_ad_prop.value()(i, j), 768., tol);
      EXPECT_NEAR(tensor_ad_prop.derivatives()[0](i, j), 3072., tol);
      EXPECT_NEAR(tensor_ad_prop.derivatives()[1](i, j), 6144., tol);
    }
  }
  tensor_ad_prop *= 1. / 768.;
  for (decltype(LIBMESH_DIM) i = 0; i != LIBMESH_DIM; ++i)
  {
    for (decltype(LIBMESH_DIM) j = 0; j != LIBMESH_DIM; ++j)
    {
      EXPECT_NEAR(tensor_ad_prop.value()(i, j), 1., tol);
      EXPECT_NEAR(tensor_ad_prop.derivatives()[0](i, j), 4., tol);
      EXPECT_NEAR(tensor_ad_prop.derivatives()[1](i, j), 8., tol);
      tensor_ad_prop.value()(i, j) = i + j;
      tensor_ad_prop.derivatives()[0](i, j) = 2. * i + 4. * j;
      tensor_ad_prop.derivatives()[1](i, j) = 3. * i + 5. * j;
    }
  }
  vector_ad_prop *= 1. / 432.;
  for (decltype(LIBMESH_DIM) i = 0; i != LIBMESH_DIM; ++i)
  {
    EXPECT_NEAR(vector_ad_prop.value()(i), 1., tol);
    EXPECT_NEAR(vector_ad_prop.derivatives()[0](i), 2., tol);
    EXPECT_NEAR(vector_ad_prop.derivatives()[1](i), 4., tol);
    vector_ad_prop.value()(i) = 10. - i;
    vector_ad_prop.derivatives()[0](i) = 10. - 2. * i;
    vector_ad_prop.derivatives()[1](i) = 10. - 3. * i;
  }

  vector_ad_prop = tensor_ad_prop * vector_ad_prop;
  EXPECT_NEAR(vector_ad_prop.value()(0), 25, tol);
  EXPECT_NEAR(vector_ad_prop.value()(1), 52, tol);
  EXPECT_NEAR(vector_ad_prop.value()(2), 79, tol);
  EXPECT_NEAR(vector_ad_prop.derivatives()[0](0), 120, tol);
  EXPECT_NEAR(vector_ad_prop.derivatives()[0](1), 198, tol);
  EXPECT_NEAR(vector_ad_prop.derivatives()[0](2), 276, tol);
  EXPECT_NEAR(vector_ad_prop.derivatives()[1](0), 140, tol);
  EXPECT_NEAR(vector_ad_prop.derivatives()[1](1), 242, tol);
  EXPECT_NEAR(vector_ad_prop.derivatives()[1](2), 344, tol);

  VectorValue<DualNumber<Real, NumberArray<2, Real>>> vector_comparator;
  TensorValue<DualNumber<Real, NumberArray<2, Real>>> tensor_comparator;
  vector_comparator(0).value() = 10;
  vector_comparator(0).derivatives()[0] = 10;
  vector_comparator(0).derivatives()[1] = 10;
  vector_comparator(1).value() = 9;
  vector_comparator(1).derivatives()[0] = 8;
  vector_comparator(1).derivatives()[1] = 7;
  vector_comparator(2).value() = 8;
  vector_comparator(2).derivatives()[0] = 6;
  vector_comparator(2).derivatives()[1] = 4;
  tensor_comparator(0, 0).value() = 0;
  tensor_comparator(0, 0).derivatives()[0] = 0;
  tensor_comparator(0, 0).derivatives()[1] = 0;
  tensor_comparator(0, 1).value() = 1;
  tensor_comparator(0, 1).derivatives()[0] = 4;
  tensor_comparator(0, 1).derivatives()[1] = 5;
  tensor_comparator(0, 2).value() = 2;
  tensor_comparator(0, 2).derivatives()[0] = 8;
  tensor_comparator(0, 2).derivatives()[1] = 10;
  tensor_comparator(1, 0).value() = 1;
  tensor_comparator(1, 0).derivatives()[0] = 2;
  tensor_comparator(1, 0).derivatives()[1] = 3;
  tensor_comparator(1, 1).value() = 2;
  tensor_comparator(1, 1).derivatives()[0] = 6;
  tensor_comparator(1, 1).derivatives()[1] = 8;
  tensor_comparator(1, 2).value() = 3;
  tensor_comparator(1, 2).derivatives()[0] = 10;
  tensor_comparator(1, 2).derivatives()[1] = 13;
  tensor_comparator(2, 0).value() = 2;
  tensor_comparator(2, 0).derivatives()[0] = 4;
  tensor_comparator(2, 0).derivatives()[1] = 6;
  tensor_comparator(2, 1).value() = 3;
  tensor_comparator(2, 1).derivatives()[0] = 8;
  tensor_comparator(2, 1).derivatives()[1] = 11;
  tensor_comparator(2, 2).value() = 4;
  tensor_comparator(2, 2).derivatives()[0] = 12;
  tensor_comparator(2, 2).derivatives()[1] = 16;

  vector_comparator = tensor_comparator * vector_comparator;
  EXPECT_NEAR(vector_comparator(0).value(), 25, tol);
  EXPECT_NEAR(vector_comparator(1).value(), 52, tol);
  EXPECT_NEAR(vector_comparator(2).value(), 79, tol);
  EXPECT_NEAR(vector_comparator(0).derivatives()[0], 120, tol);
  EXPECT_NEAR(vector_comparator(1).derivatives()[0], 198, tol);
  EXPECT_NEAR(vector_comparator(2).derivatives()[0], 276, tol);
  EXPECT_NEAR(vector_comparator(0).derivatives()[1], 140, tol);
  EXPECT_NEAR(vector_comparator(1).derivatives()[1], 242, tol);
  EXPECT_NEAR(vector_comparator(2).derivatives()[1], 344, tol);

  tensor_ad_prop *= tensor_ad_prop;
  EXPECT_NEAR(tensor_ad_prop.value()(0, 0), 5, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[0](0, 0), 30, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[1](0, 0), 40, tol);
  EXPECT_NEAR(tensor_ad_prop.value()(0, 1), 8, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[0](0, 1), 54, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[1](0, 1), 70, tol);
  EXPECT_NEAR(tensor_ad_prop.value()(0, 2), 11, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[0](0, 2), 78, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[1](0, 2), 100, tol);
  EXPECT_NEAR(tensor_ad_prop.value()(1, 0), 8, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[0](1, 0), 42, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[1](1, 0), 58, tol);
  EXPECT_NEAR(tensor_ad_prop.value()(1, 1), 14, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[0](1, 1), 84, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[1](1, 1), 112, tol);
  EXPECT_NEAR(tensor_ad_prop.value()(1, 2), 20, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[0](1, 2), 126, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[1](1, 2), 166, tol);
  EXPECT_NEAR(tensor_ad_prop.value()(2, 0), 11, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[0](2, 0), 54, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[1](2, 0), 76, tol);
  EXPECT_NEAR(tensor_ad_prop.value()(2, 1), 20, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[0](2, 1), 114, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[1](2, 1), 154, tol);
  EXPECT_NEAR(tensor_ad_prop.value()(2, 2), 29, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[0](2, 2), 174, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[1](2, 2), 232, tol);

  tensor_comparator *= tensor_comparator;
  EXPECT_NEAR(tensor_comparator(0, 0).value(), 5, tol);
  EXPECT_NEAR(tensor_comparator(0, 0).derivatives()[0], 30, tol);
  EXPECT_NEAR(tensor_comparator(0, 0).derivatives()[1], 40, tol);
  EXPECT_NEAR(tensor_comparator(0, 1).value(), 8, tol);
  EXPECT_NEAR(tensor_comparator(0, 1).derivatives()[0], 54, tol);
  EXPECT_NEAR(tensor_comparator(0, 1).derivatives()[1], 70, tol);
  EXPECT_NEAR(tensor_comparator(0, 2).value(), 11, tol);
  EXPECT_NEAR(tensor_comparator(0, 2).derivatives()[0], 78, tol);
  EXPECT_NEAR(tensor_comparator(0, 2).derivatives()[1], 100, tol);
  EXPECT_NEAR(tensor_comparator(1, 0).value(), 8, tol);
  EXPECT_NEAR(tensor_comparator(1, 0).derivatives()[0], 42, tol);
  EXPECT_NEAR(tensor_comparator(1, 0).derivatives()[1], 58, tol);
  EXPECT_NEAR(tensor_comparator(1, 1).value(), 14, tol);
  EXPECT_NEAR(tensor_comparator(1, 1).derivatives()[0], 84, tol);
  EXPECT_NEAR(tensor_comparator(1, 1).derivatives()[1], 112, tol);
  EXPECT_NEAR(tensor_comparator(1, 2).value(), 20, tol);
  EXPECT_NEAR(tensor_comparator(1, 2).derivatives()[0], 126, tol);
  EXPECT_NEAR(tensor_comparator(1, 2).derivatives()[1], 166, tol);
  EXPECT_NEAR(tensor_comparator(2, 0).value(), 11, tol);
  EXPECT_NEAR(tensor_comparator(2, 0).derivatives()[0], 54, tol);
  EXPECT_NEAR(tensor_comparator(2, 0).derivatives()[1], 76, tol);
  EXPECT_NEAR(tensor_comparator(2, 1).value(), 20, tol);
  EXPECT_NEAR(tensor_comparator(2, 1).derivatives()[0], 114, tol);
  EXPECT_NEAR(tensor_comparator(2, 1).derivatives()[1], 154, tol);
  EXPECT_NEAR(tensor_comparator(2, 2).value(), 29, tol);
  EXPECT_NEAR(tensor_comparator(2, 2).derivatives()[0], 174, tol);
  EXPECT_NEAR(tensor_comparator(2, 2).derivatives()[1], 232, tol);

  tensor_ad_prop = tensor_ad_prop * tensor_ad_prop;
  tensor_ad_prop = 7. * tensor_ad_prop + tensor_ad_prop * 8.;
  tensor_ad_prop = 9. * tensor_ad_prop - tensor_ad_prop * 10.;
  tensor_comparator = tensor_comparator * tensor_comparator;
  tensor_comparator = tensor_comparator * 7. + tensor_comparator * 8.;
  tensor_comparator = tensor_comparator * 9. - tensor_comparator * 10.;

  EXPECT_NEAR(tensor_ad_prop.value()(0, 0), -3150, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[0](0, 0), -37800, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[1](0, 0), -50400, tol);
  EXPECT_NEAR(tensor_ad_prop.value()(0, 1), -5580, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[0](0, 1), -71280, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[1](0, 1), -93600, tol);
  EXPECT_NEAR(tensor_ad_prop.value()(0, 2), -8010, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[0](0, 2), -104760, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[1](0, 2), -136800, tol);
  EXPECT_NEAR(tensor_ad_prop.value()(1, 0), -5580, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[0](1, 0), -62640, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[1](1, 0), -84960, tol);
  EXPECT_NEAR(tensor_ad_prop.value()(1, 1), -9900, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[0](1, 1), -118800, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[1](1, 1), -158400, tol);
  EXPECT_NEAR(tensor_ad_prop.value()(1, 2), -14220, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[0](1, 2), -174960, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[1](1, 2), -231840, tol);
  EXPECT_NEAR(tensor_ad_prop.value()(2, 0), -8010, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[0](2, 0), -87480, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[1](2, 0), -119520, tol);
  EXPECT_NEAR(tensor_ad_prop.value()(2, 1), -14220, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[0](2, 1), -166320, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[1](2, 1), -223200, tol);
  EXPECT_NEAR(tensor_ad_prop.value()(2, 2), -20430, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[0](2, 2), -245160, tol);
  EXPECT_NEAR(tensor_ad_prop.derivatives()[1](2, 2), -326880, tol);

  EXPECT_NEAR(tensor_comparator(0, 0).value(), -3150, tol);
  EXPECT_NEAR(tensor_comparator(0, 0).derivatives()[0], -37800, tol);
  EXPECT_NEAR(tensor_comparator(0, 0).derivatives()[1], -50400, tol);
  EXPECT_NEAR(tensor_comparator(0, 1).value(), -5580, tol);
  EXPECT_NEAR(tensor_comparator(0, 1).derivatives()[0], -71280, tol);
  EXPECT_NEAR(tensor_comparator(0, 1).derivatives()[1], -93600, tol);
  EXPECT_NEAR(tensor_comparator(0, 2).value(), -8010, tol);
  EXPECT_NEAR(tensor_comparator(0, 2).derivatives()[0], -104760, tol);
  EXPECT_NEAR(tensor_comparator(0, 2).derivatives()[1], -136800, tol);
  EXPECT_NEAR(tensor_comparator(1, 0).value(), -5580, tol);
  EXPECT_NEAR(tensor_comparator(1, 0).derivatives()[0], -62640, tol);
  EXPECT_NEAR(tensor_comparator(1, 0).derivatives()[1], -84960, tol);
  EXPECT_NEAR(tensor_comparator(1, 1).value(), -9900, tol);
  EXPECT_NEAR(tensor_comparator(1, 1).derivatives()[0], -118800, tol);
  EXPECT_NEAR(tensor_comparator(1, 1).derivatives()[1], -158400, tol);
  EXPECT_NEAR(tensor_comparator(1, 2).value(), -14220, tol);
  EXPECT_NEAR(tensor_comparator(1, 2).derivatives()[0], -174960, tol);
  EXPECT_NEAR(tensor_comparator(1, 2).derivatives()[1], -231840, tol);
  EXPECT_NEAR(tensor_comparator(2, 0).value(), -8010, tol);
  EXPECT_NEAR(tensor_comparator(2, 0).derivatives()[0], -87480, tol);
  EXPECT_NEAR(tensor_comparator(2, 0).derivatives()[1], -119520, tol);
  EXPECT_NEAR(tensor_comparator(2, 1).value(), -14220, tol);
  EXPECT_NEAR(tensor_comparator(2, 1).derivatives()[0], -166320, tol);
  EXPECT_NEAR(tensor_comparator(2, 1).derivatives()[1], -223200, tol);
  EXPECT_NEAR(tensor_comparator(2, 2).value(), -20430, tol);
  EXPECT_NEAR(tensor_comparator(2, 2).derivatives()[0], -245160, tol);
  EXPECT_NEAR(tensor_comparator(2, 2).derivatives()[1], -326880, tol);

  scalar_reg_prop = scalar_ad_prop.value();
  vector_reg_prop = vector_ad_prop.value();
  tensor_reg_prop = tensor_ad_prop.value();
  EXPECT_NEAR(scalar_reg_prop, 768, tol);
  EXPECT_NEAR(vector_reg_prop(0), 25, tol);
  EXPECT_NEAR(vector_reg_prop(1), 52, tol);
  EXPECT_NEAR(vector_reg_prop(2), 79, tol);
  EXPECT_NEAR(tensor_reg_prop(0, 0), -3150, tol);
  EXPECT_NEAR(tensor_reg_prop(0, 1), -5580, tol);
  EXPECT_NEAR(tensor_reg_prop(0, 2), -8010, tol);
  EXPECT_NEAR(tensor_reg_prop(1, 0), -5580, tol);
  EXPECT_NEAR(tensor_reg_prop(1, 1), -9900, tol);
  EXPECT_NEAR(tensor_reg_prop(1, 2), -14220, tol);
  EXPECT_NEAR(tensor_reg_prop(2, 0), -8010, tol);
  EXPECT_NEAR(tensor_reg_prop(2, 1), -14220, tol);
  EXPECT_NEAR(tensor_reg_prop(2, 2), -20430, tol);
}
