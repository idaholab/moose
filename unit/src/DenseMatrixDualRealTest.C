//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DualReal.h"
#include "DenseMatrix.h"

#include "libmesh/dense_vector.h"

#include "DualRealOps.h"

#include "gtest/gtest.h"

using namespace libMesh;

TEST(DenseMatrixDualReal, TryOperations)
{
  DenseMatrix<DualReal> a(3, 3);
  DenseMatrix<DualReal> b(3, 3);
  a.left_multiply(b);
  a.left_multiply_transpose(b);
  a.right_multiply(b);
  a.right_multiply_transpose(b);

  DenseVector<DualReal> vec(3);
  DenseVector<DualReal> dest(3);

  a.vector_mult(dest, vec);
  a.vector_mult_transpose(dest, vec);
  a.vector_mult_add(dest, 1, vec);
  a.outer_product(vec, dest);

  DenseMatrix<DualReal> sub(2, 2);
  a.get_principal_submatrix(2, 2, sub);

  vec(0) = 1;
  vec(1) = 2;
  vec(2) = 3;

  a(0, 0) = 1;
  a(1, 1) = 1;
  a(2, 2) = 1;

  a.lu_solve(vec, dest);
  EXPECT_NEAR(dest(0).value(), vec(0).value(), TOLERANCE);
  EXPECT_NEAR(dest(1).value(), vec(1).value(), TOLERANCE);
  EXPECT_NEAR(dest(2).value(), vec(2).value(), TOLERANCE);

  EXPECT_NEAR(1, a.det().value(), TOLERANCE);

  b(0, 0) = 1;
  b(1, 1) = 1;
  b(2, 2) = 1;

  b.cholesky_solve(vec, dest);
  EXPECT_NEAR(dest(0).value(), vec(0).value(), TOLERANCE);
  EXPECT_NEAR(dest(1).value(), vec(1).value(), TOLERANCE);
  EXPECT_NEAR(dest(2).value(), vec(2).value(), TOLERANCE);
}
