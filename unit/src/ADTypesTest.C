//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Moose.h"
#include "Material.h"
#include "DenseMatrix.h"
#include "EigenADReal.h"

#include "libmesh/dense_vector.h"
#include "gtest_include.h"
#include "Eigen/Dense"

#include <vector>

template <typename T>
class MyMat
{
public:
  ADMaterialProperty<T> prop;
};

TEST(ADTypesTest, vector_DenseMatrix_Real)
{
  std::vector<DenseMatrix<ADReal>> vm(2);
  auto & m1 = vm[0];
  auto & m2 = vm[1];
  m1.resize(3, 3);
  m2.resize(3, 3);
  m1(0, 0) = 1;
  m1(1, 1) = 2;
  m1(2, 2) = 3;
  m2(0, 0) = 1;
  m2(1, 1) = 2;
  m2(2, 2) = 3;
  m1.right_multiply(m2);
  EXPECT_EQ(1.0, m1(0, 0));
  EXPECT_EQ(4.0, m1(1, 1));
  EXPECT_EQ(9.0, m1(2, 2));
}

TEST(ADTypesTest, vector_DenseVector_Real)
{
  std::vector<DenseVector<ADReal>> vv(2);
  auto & v1 = vv[0];
  auto & v2 = vv[1];
  v1.resize(3);
  v2.resize(3);
  v1(0) = 1;
  v1(1) = 2;
  v1(2) = 3;
  v2(0) = 1;
  v2(1) = 2;
  v2(2) = 3;
  auto val = v1.dot(v2);
  EXPECT_EQ(14.0, val);
}

TEST(ADTypesTest, EigenMatrix_ADReal)
{
  Eigen::Matrix<ADReal, 3, 3> m1;
  m1 << 1, 2, 3, 4, 5, 6, 7, 8, 9;

  const Eigen::Matrix<Real, 3, 3> m2 = MetaPhysicL::raw_value(m1);

  EXPECT_EQ((m1 - m2).norm(), 0.0);
}
