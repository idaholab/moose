//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ColumnMajorMatrixTest.h"

// Moose includes
#include "ColumnMajorMatrix.h"

using libMesh::VectorValue;

TEST_F(ColumnMajorMatrixTest, addMatrixScalar)
{
  ColumnMajorMatrix as(3, 3);

  as(0, 0) = 11;
  as(1, 0) = 12;
  as(2, 0) = 13;
  as(0, 1) = 14;
  as(1, 1) = 15;
  as(2, 1) = 16;
  as(0, 2) = 17;
  as(1, 2) = 18;
  as(2, 2) = 19;

  EXPECT_EQ(as, a + 10);
}

TEST_F(ColumnMajorMatrixTest, divideMatrixScalarEquals)
{
  ColumnMajorMatrix divide(2, 2);

  divide(0, 0) = 2;
  divide(1, 0) = 4;
  divide(0, 1) = 6;
  divide(1, 1) = 8;

  divide /= 2;
  EXPECT_EQ(divide(0, 0), 1);
  EXPECT_EQ(divide(1, 0), 2);
  EXPECT_EQ(divide(0, 1), 3);
  EXPECT_EQ(divide(1, 1), 4);
}

TEST_F(ColumnMajorMatrixTest, addMatrixScalarEquals)
{
  ColumnMajorMatrix as(3, 3);

  as(0, 0) = 11;
  as(1, 0) = 12;
  as(2, 0) = 13;
  as(0, 1) = 14;
  as(1, 1) = 15;
  as(2, 1) = 16;
  as(0, 2) = 17;
  as(1, 2) = 18;
  as(2, 2) = 19;

  // Scalar add and update
  a += 10;
  EXPECT_EQ(as, a);
}

TEST_F(ColumnMajorMatrixTest, multMatrixScalar)
{
  ColumnMajorMatrix mult_solution(3, 3);

  mult_solution(0, 0) = 2;
  mult_solution(1, 0) = 4;
  mult_solution(2, 0) = 6;
  mult_solution(0, 1) = 8;
  mult_solution(1, 1) = 10;
  mult_solution(2, 1) = 12;
  mult_solution(0, 2) = 14;
  mult_solution(1, 2) = 16;
  mult_solution(2, 2) = 18;

  EXPECT_EQ(mult_solution, a * 2);
}

TEST_F(ColumnMajorMatrixTest, multMatrixScalarEquals)
{
  ColumnMajorMatrix mult_solution(3, 3);

  mult_solution(0, 0) = 2;
  mult_solution(1, 0) = 4;
  mult_solution(2, 0) = 6;
  mult_solution(0, 1) = 8;
  mult_solution(1, 1) = 10;
  mult_solution(2, 1) = 12;
  mult_solution(0, 2) = 14;
  mult_solution(1, 2) = 16;
  mult_solution(2, 2) = 18;

  // Scalar multiply and update
  a *= 2;

  EXPECT_EQ(mult_solution, a);
}

TEST_F(ColumnMajorMatrixTest, multMatrixMatrix)
{
  ColumnMajorMatrix mult_solution(3, 3);

  mult_solution(0, 0) = 30;
  mult_solution(1, 0) = 36;
  mult_solution(2, 0) = 42;
  mult_solution(0, 1) = 66;
  mult_solution(1, 1) = 81;
  mult_solution(2, 1) = 96;
  mult_solution(0, 2) = 102;
  mult_solution(1, 2) = 126;
  mult_solution(2, 2) = 150;

  EXPECT_EQ(mult_solution, a * a);
}

TEST_F(ColumnMajorMatrixTest, multMatrixVec)
{
  ColumnMajorMatrix vec(3, 1), mult_solution(3, 1);

  vec(0, 0) = 1;
  vec(1, 0) = 2;
  vec(2, 0) = 3;

  mult_solution(0, 0) = 30;
  mult_solution(1, 0) = 36;
  mult_solution(2, 0) = 42;

  EXPECT_EQ(mult_solution, a * vec);
}

TEST_F(ColumnMajorMatrixTest, reshapeMatrix)
{
  ColumnMajorMatrix mat(3, 2);

  mat(0, 0) = 1;
  mat(1, 0) = 2;
  mat(2, 0) = 3;
  mat(0, 1) = 4;
  mat(1, 1) = 5;
  mat(2, 1) = 6;
  mat.reshape(2, 3);

  /** from:
   * 1 4
   * 2 5
   * 3 6
   *
   * to:
   *
   * 1 3 5
   * 2 4 6
   */

  EXPECT_EQ(mat(0, 1), 3);
  EXPECT_EQ(mat(1, 2), 6);

  mat.reshape(1, 6);

  EXPECT_EQ(mat(0, 1), 2);
  EXPECT_EQ(mat(0, 5), 6);
}

TEST_F(ColumnMajorMatrixTest, setDiagMatrix)
{
  ColumnMajorMatrix mat = two_mat;
  mat.setDiag(9);

  EXPECT_EQ(mat(0, 0), 9);
  EXPECT_EQ(mat(1, 0), 2);
  EXPECT_EQ(mat(0, 1), 3);
  EXPECT_EQ(mat(1, 1), 9);
}

TEST_F(ColumnMajorMatrixTest, setDiagMatrixError)
{
  ColumnMajorMatrix mat(1, 2);
  mat(0, 0) = 1;
  mat(0, 1) = 1;

  try
  {
    mat.setDiag(9);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "ColumnMajorMatrix error: Unable to perform the operation on a non-square matrix.") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST_F(ColumnMajorMatrixTest, trMatrix)
{
  ColumnMajorMatrix mat(2, 2);
  mat(0, 0) = 1;
  mat(0, 1) = 2;
  mat(1, 0) = 3;
  mat(1, 1) = 4;
  EXPECT_EQ(mat.tr(), 5);
}

TEST_F(ColumnMajorMatrixTest, zeroMatrix)
{
  ColumnMajorMatrix mat = two_mat;
  mat.zero();

  EXPECT_EQ(mat(0, 0), 0);
  EXPECT_EQ(mat(1, 0), 0);
  EXPECT_EQ(mat(0, 1), 0);
  EXPECT_EQ(mat(1, 1), 0);
}

TEST_F(ColumnMajorMatrixTest, identityMatrix)
{
  ColumnMajorMatrix mat = two_mat;
  mat.identity();

  EXPECT_EQ(mat(0, 0), 1);
  EXPECT_EQ(mat(1, 0), 0);
  EXPECT_EQ(mat(0, 1), 0);
  EXPECT_EQ(mat(1, 1), 1);
}

TEST_F(ColumnMajorMatrixTest, contractionMatrix)
{
  ColumnMajorMatrix rhs(4, 1);
  ColumnMajorMatrix lhs(4, 1);

  rhs(0, 0) = 4;
  rhs(1, 0) = 3;
  rhs(2, 0) = 2;
  rhs(3, 0) = 1;

  lhs(0, 0) = 1;
  lhs(1, 0) = 2;
  lhs(2, 0) = 3;
  lhs(3, 0) = 4;

  EXPECT_EQ(lhs.doubleContraction(rhs), (1 * 4 + 2 * 3 + 3 * 2 + 4 * 1));
}

TEST_F(ColumnMajorMatrixTest, contractionMatrixError)
{
  ColumnMajorMatrix rhs(4, 1);
  ColumnMajorMatrix lhs(4, 2);

  rhs(0, 0) = 4;
  rhs(1, 0) = 3;
  rhs(2, 0) = 2;
  rhs(3, 0) = 1;

  lhs(0, 0) = 1;
  lhs(1, 0) = 2;
  lhs(2, 0) = 3;
  lhs(3, 0) = 4;

  lhs(0, 1) = 1;
  lhs(1, 1) = 2;
  lhs(2, 1) = 3;
  lhs(3, 1) = 4;

  try
  {
    // Attempt doubleContraction
    lhs.doubleContraction(rhs);
    FAIL() << "missing expected exception";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("ColumnMajorMatrix error: Unable to perform the operation on matrices of "
                         "different shapes.") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST_F(ColumnMajorMatrixTest, normMatrix)
{
  ColumnMajorMatrix mat(4, 1);
  mat(0, 0) = 1;
  mat(1, 0) = 2;
  mat(2, 0) = 2;
  mat(3, 0) = 4;

  EXPECT_EQ(mat.norm(), 5);
}

TEST_F(ColumnMajorMatrixTest, transposeMatrix)
{
  ColumnMajorMatrix mat = two_mat;
  ColumnMajorMatrix test = mat.transpose();

  EXPECT_EQ(test(0, 0), 1);
  EXPECT_EQ(test(1, 0), 3);
  EXPECT_EQ(test(0, 1), 2);
  EXPECT_EQ(test(1, 1), 4);
}

TEST_F(ColumnMajorMatrixTest, rowColConstructor)
{
  ColumnMajorMatrix mat = two_mat;

  EXPECT_EQ(mat(0, 0), 1);
  EXPECT_EQ(mat(1, 0), 2);
  EXPECT_EQ(mat(0, 1), 3);
  EXPECT_EQ(mat(1, 1), 4);
}

TEST_F(ColumnMajorMatrixTest, copyConstructor)
{
  ColumnMajorMatrix mat = two_mat;
  ColumnMajorMatrix test = mat;
  test(1, 1) = 9;

  EXPECT_EQ(test(0, 0), 1);
  EXPECT_EQ(test(1, 0), 2);
  EXPECT_EQ(test(0, 1), 3);
  EXPECT_EQ(test(1, 1), 9);
  EXPECT_EQ(mat(1, 1), 4);

  EXPECT_EQ(test.numEntries(), 4);
}

TEST_F(ColumnMajorMatrixTest, tensorConstructor)
{
  TensorValue<Real> tensor(1, 4, 7, 2, 5, 8, 3, 6, 9);
  ColumnMajorMatrix test(tensor);

  EXPECT_EQ(test, a);
  EXPECT_EQ(test.numEntries(), 9);
}

TEST_F(ColumnMajorMatrixTest, ThreeColConstructor)
{
  VectorValue<Real> col1(1, 2, 3);
  VectorValue<Real> col2(4, 5, 6);
  VectorValue<Real> col3(7, 8, 9);

  ColumnMajorMatrix test(col1, col2, col3);

  EXPECT_EQ(test, a);
  EXPECT_EQ(test.numEntries(), 9);
}

TEST_F(ColumnMajorMatrixTest, numEntries)
{
  // numEntries is tested in other functions, like after different
  // constructors to make sure the number of entries copied over correctly
  ColumnMajorMatrix mat = two_mat;
  ColumnMajorMatrix mat2(3, 4);

  EXPECT_EQ(mat.numEntries(), 4);
  EXPECT_EQ(mat2.numEntries(), 12);
}

TEST_F(ColumnMajorMatrixTest, accessMatrix)
{
  // tests operator()
  ColumnMajorMatrix mat = two_mat;

  EXPECT_EQ(mat(0, 0), 1);
  EXPECT_EQ(mat(1, 0), 2);
  EXPECT_EQ(mat(0, 1), 3);
  EXPECT_EQ(mat(1, 1), 4);
}

TEST_F(ColumnMajorMatrixTest, print)
{
  // TODO?
}

TEST_F(ColumnMajorMatrixTest, fillMatrix)
{
  TensorValue<Real> tensor(0, 0, 0, 0, 0, 0, 0, 0, 0);
  ColumnMajorMatrix & a_ref = a;
  a_ref.fill(tensor);

  EXPECT_EQ(tensor(0, 0), a_ref(0, 0));
  EXPECT_EQ(tensor(1, 0), a_ref(1, 0));
  EXPECT_EQ(tensor(2, 0), a_ref(2, 0));
  EXPECT_EQ(tensor(0, 1), a_ref(0, 1));
  EXPECT_EQ(tensor(1, 1), a_ref(1, 1));
  EXPECT_EQ(tensor(2, 1), a_ref(2, 1));
  EXPECT_EQ(tensor(0, 2), a_ref(0, 2));
  EXPECT_EQ(tensor(1, 2), a_ref(1, 2));
  EXPECT_EQ(tensor(2, 2), a_ref(2, 2));
}

TEST_F(ColumnMajorMatrixTest, fillTensorError)
{
  try
  {
    TensorValue<Real> tensor(0, 0, 0, 0, 0, 0, 0, 0, 0);
    ColumnMajorMatrix & t_ref = t;
    t_ref.fill(tensor);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "Cannot fill tensor! The ColumnMajorMatrix doesn't have the same number of entries!") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST_F(ColumnMajorMatrixTest, fillMatrixError)
{
  try
  {
    DenseMatrix<Real> mat(2, 2);
    ColumnMajorMatrix & a_ref = a;
    a_ref.fill(mat);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot fill dense matrix! The ColumnMajorMatrix doesn't have the same "
                         "number of entries!") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST_F(ColumnMajorMatrixTest, fillVectorError)
{
  try
  {
    DenseVector<Real> vec(2);
    ColumnMajorMatrix & a_ref = a;
    a_ref.fill(vec);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("ColumnMajorMatrix and DenseVector must be the same shape for a fill!") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST_F(ColumnMajorMatrixTest, tensorAssignOperator)
{
  ColumnMajorMatrix test(3, 3);
  TensorValue<Real> tensor(1, 4, 7, 2, 5, 8, 3, 6, 9);
  test = tensor;

  EXPECT_EQ(test, a);
  EXPECT_EQ(test.numEntries(), 9);
}

TEST_F(ColumnMajorMatrixTest, matrixAssignOperator)
{
  ColumnMajorMatrix mat = two_mat;
  ColumnMajorMatrix test(1, 2);

  test = mat;
  test(1, 1) = 9;

  EXPECT_EQ(test(0, 0), 1);
  EXPECT_EQ(test(1, 0), 2);
  EXPECT_EQ(test(0, 1), 3);
  EXPECT_EQ(test(1, 1), 9);
  EXPECT_EQ(mat(1, 1), 4);

  EXPECT_EQ(test.numEntries(), 4);
}

TEST_F(ColumnMajorMatrixTest, addMatrixMatrix) { EXPECT_EQ(t + add, add_solution); }

TEST_F(ColumnMajorMatrixTest, addMatrixMatrixEquals)
{
  ColumnMajorMatrix mat = t;
  mat += add;

  EXPECT_EQ(mat, add_solution);
}

TEST_F(ColumnMajorMatrixTest, subMatrixMatrixEquals)
{
  ColumnMajorMatrix mat = t;
  mat -= sub;

  EXPECT_EQ(mat, sub_solution);
}

TEST_F(ColumnMajorMatrixTest, equalMatrix)
{
  ColumnMajorMatrix mat(1, 1);
  ColumnMajorMatrix mat1(1, 1);

  mat(0, 0) = 2;
  mat1(0, 0) = 2;

  ASSERT_NE(a, t);
  EXPECT_EQ(mat, mat1);
}

TEST_F(ColumnMajorMatrixTest, notEqualMatrix)
{
  ColumnMajorMatrix mat(1, 1);
  ColumnMajorMatrix mat1(1, 1);

  mat(0, 0) = 2;
  mat1(0, 0) = 2;

  ASSERT_NE(a, t);
  EXPECT_EQ(mat, mat1);
}

TEST_F(ColumnMajorMatrixTest, kroneckererror)
{
  ColumnMajorMatrix mat1(2, 1);
  mat1(0, 0) = 1;
  mat1(1, 0) = 2;
  try
  {
    mat1.kronecker(t);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "ColumnMajorMatrix error: Unable to perform the operation on a non-square matrix.") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST_F(ColumnMajorMatrixTest, kronecker)
{
  ColumnMajorMatrix lhs(2, 2);
  lhs(0, 0) = 1;
  lhs(0, 1) = 2;
  lhs(1, 0) = 3;
  lhs(1, 1) = 4;

  ColumnMajorMatrix rhs(2, 2);
  rhs(0, 0) = 0;
  rhs(0, 1) = 5;
  rhs(1, 0) = 6;
  rhs(1, 1) = 7;

  ColumnMajorMatrix ans = lhs.kronecker(rhs);

  EXPECT_EQ(ans(0, 0), 0);
  EXPECT_EQ(ans(0, 3), 10);
  EXPECT_EQ(ans(1, 0), 6);
  EXPECT_EQ(ans(1, 1), 7);
  EXPECT_EQ(ans(1, 3), 14);
  EXPECT_EQ(ans(2, 1), 15);
  EXPECT_EQ(ans(2, 2), 0);
  EXPECT_EQ(ans(3, 0), 18);
  EXPECT_EQ(ans(3, 1), 21);
  EXPECT_EQ(ans(3, 2), 24);
  EXPECT_EQ(ans(3, 3), 28);
}

TEST_F(ColumnMajorMatrixTest, inverse)
{
  ColumnMajorMatrix matrix(3, 3), matrix_inverse(3, 3), e_val(3, 3), e_vec(3, 3);

  matrix(0, 0) = 1.0;
  matrix(0, 1) = 3.0;
  matrix(0, 2) = 3.0;

  matrix(1, 0) = 1.0;
  matrix(1, 1) = 4.0;
  matrix(1, 2) = 3.0;

  matrix(2, 0) = 1.0;
  matrix(2, 1) = 3.0;
  matrix(2, 2) = 4.0;

  matrix.inverse(matrix_inverse);

  EXPECT_EQ(matrix_inverse(0, 0), 7.0);
  EXPECT_EQ(matrix_inverse(0, 1), -3.0);
  EXPECT_EQ(matrix_inverse(0, 2), -3.0);

  EXPECT_EQ(matrix_inverse(1, 0), -1.0);
  EXPECT_EQ(matrix_inverse(1, 1), 1.0);
  EXPECT_EQ(matrix_inverse(1, 2), 0.0);

  EXPECT_EQ(matrix_inverse(2, 0), -1.0);
  EXPECT_EQ(matrix_inverse(2, 1), 0.0);
  EXPECT_EQ(matrix_inverse(2, 2), 1.0);

  /*matrix(0,0) = 2.0;
  matrix(0,1) = 1.0;
  matrix(0,2) = 1.0;

  matrix(1,0) = 1.0;
  matrix(1,1) = 2.0;
  matrix(1,2) = 1.0;

  matrix(2,0) = 1.0;
  matrix(2,1) = 1.0;
  matrix(2,2) = 2.0;

  matrix.eigen(e_val, e_vec);
  e_vec.inverse(matrix_inverse);

  e_val.print();
  e_vec.print();
  matrix_inverse.print();*/
}

TEST_F(ColumnMajorMatrixTest, eigen)
{
  ColumnMajorMatrix matrix(2, 2), e_vec(2, 2), e_val(2, 1);
  Real err = 1.0e-14;

  matrix(0, 0) = 1.0;
  matrix(0, 1) = 2.0;
  matrix(1, 0) = 2.0;
  matrix(1, 1) = 4.0;

  matrix.eigen(e_val, e_vec);

  EXPECT_LT(std::abs(e_vec(0, 0) - -2 / sqrt(5)), err);
  EXPECT_LT(std::abs(e_vec(0, 1) - 1 / sqrt(5)), err);
  EXPECT_LT(std::abs(e_vec(1, 0) - 1 / sqrt(5)), err);
  EXPECT_LT(std::abs(e_vec(1, 1) - 2 / sqrt(5)), err);

  EXPECT_EQ(e_val(0, 0), 0.0);
  EXPECT_EQ(e_val(1, 0), 5.0);
}

TEST_F(ColumnMajorMatrixTest, eigenNonsym)
{
  ColumnMajorMatrix matrix(2, 2), e_vector_right(2, 2), e_vector_left(2, 2), e_values_real(2, 1),
      e_values_img(2, 1);
  Real err = 1.0e-14;

  matrix(0, 0) = 2.0;
  matrix(0, 1) = 1.0;
  matrix(1, 0) = 1.0;
  matrix(1, 1) = 2.0;

  matrix.eigenNonsym(e_values_real, e_values_img, e_vector_right, e_vector_left);

  EXPECT_LT(std::abs(e_vector_right(0, 0) - 1 / sqrt(2)), err);
  EXPECT_LT(std::abs(e_vector_right(0, 1) - -1 / sqrt(2)), err);
  EXPECT_LT(std::abs(e_vector_right(1, 0) - 1 / sqrt(2)), err);
  EXPECT_LT(std::abs(e_vector_right(1, 1) - 1 / sqrt(2)), err);

  EXPECT_EQ(e_values_real(0, 0), 3.0);
  EXPECT_EQ(e_values_real(1, 0), 1.0);

  EXPECT_EQ(e_values_img(0, 0), 0.0);
  EXPECT_EQ(e_values_img(1, 0), 0.0);
}

TEST_F(ColumnMajorMatrixTest, exp)
{
  ColumnMajorMatrix matrix1(3, 3), matrix_exp1(3, 3);
  ColumnMajorMatrix matrix2(3, 3), matrix_exp2(3, 3);
  Real err = 1.0e-10;
  Real e = 2.71828182846;
  Real e1 = (e * e * e * e - e) / 3, e2 = (2 * e + e * e * e * e) / 3;

  matrix1(0, 0) = 2.0;
  matrix1(0, 1) = 1.0;
  matrix1(0, 2) = 1.0;
  matrix1(1, 0) = 1.0;
  matrix1(1, 1) = 2.0;
  matrix1(1, 2) = 1.0;
  matrix1(2, 0) = 1.0;
  matrix1(2, 1) = 1.0;
  matrix1(2, 2) = 2.0;

  matrix1.exp(matrix_exp1);

  EXPECT_LT(std::abs(matrix_exp1(0, 0) - e2), err);
  EXPECT_LT(std::abs(matrix_exp1(0, 1) - e1), err);
  EXPECT_LT(std::abs(matrix_exp1(0, 2) - e1), err);
  EXPECT_LT(std::abs(matrix_exp1(1, 0) - e1), err);
  EXPECT_LT(std::abs(matrix_exp1(1, 1) - e2), err);
  EXPECT_LT(std::abs(matrix_exp1(1, 2) - e1), err);
  EXPECT_LT(std::abs(matrix_exp1(2, 0) - e1), err);
  EXPECT_LT(std::abs(matrix_exp1(2, 1) - e1), err);
  EXPECT_LT(std::abs(matrix_exp1(2, 2) - e2), err);

  matrix2(0, 0) = 1.0;
  matrix2(0, 1) = 1.0;
  matrix2(0, 2) = 0.0;
  matrix2(1, 0) = 0.0;
  matrix2(1, 1) = 0.0;
  matrix2(1, 2) = 2.0;
  matrix2(2, 0) = 0.0;
  matrix2(2, 1) = 0.0;
  matrix2(2, 2) = -1.0;

  matrix2.exp(matrix_exp2);

  EXPECT_LT(std::abs(matrix_exp2(0, 0) - e), err);
  EXPECT_LT(std::abs(matrix_exp2(0, 1) - (e - 1)), err);
  EXPECT_LT(std::abs(matrix_exp2(0, 2) - (-(-(e * e) + (2 * e) - 1) / e)), err);
  EXPECT_LT(std::abs(matrix_exp2(1, 0) - 0.0), err);
  EXPECT_LT(std::abs(matrix_exp2(1, 1) - 1), err);
  EXPECT_LT(std::abs(matrix_exp2(1, 2) - (2 * (e - 1) / e)), err);
  EXPECT_LT(std::abs(matrix_exp2(2, 0) - 0.0), err);
  EXPECT_LT(std::abs(matrix_exp2(2, 1) - 0.0), err);
  EXPECT_LT(std::abs(matrix_exp2(2, 2) - (1 / e)), err);
}

TEST_F(ColumnMajorMatrixTest, denseMatrixAssignment)
{
  DenseMatrix<Real> rhs(2, 1);
  ColumnMajorMatrix lhs(2, 2);
  rhs(0, 0) = 1;
  rhs(1, 0) = 2;
  try
  {
    lhs = rhs;
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("ColumnMajorMatrix and DenseMatrix should be of the same shape.") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST_F(ColumnMajorMatrixTest, denseVectorAssignment)
{
  DenseVector<Real> rhs(2);
  ColumnMajorMatrix lhs(2, 2);
  rhs(0) = 1;
  rhs(1) = 2;
  try
  {
    lhs = rhs;
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("ColumnMajorMatrix and DenseVector should be of the same shape.") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST_F(ColumnMajorMatrixTest, multTypeVecError)
{
  TypeVector<Real> rhs;
  try
  {
    ColumnMajorMatrix result = t * rhs;
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Cannot perform matvec operation! The column dimension of "
                         "the ColumnMajorMatrix does not match the TypeVector!") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST_F(ColumnMajorMatrixTest, matrixMultError)
{
  try
  {
    ColumnMajorMatrix result = a * two_mat;
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "Cannot perform matrix multiply! The shapes of the two operands are not compatible!") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST_F(ColumnMajorMatrixTest, matrixAddAssignError)
{
  TensorValue<Real> tensor(0, 0, 0, 0, 0, 0, 0, 0, 0);
  try
  {
    t += tensor;
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "Cannot perform matrix addition and assignment! The shapes of the two operands are "
            "not compatible!") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST_F(ColumnMajorMatrixTest, boundsTest)
{
  try
  {
    a(3, 4) += 2;
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Reference outside of ColumnMajorMatrix bounds!") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST_F(ColumnMajorMatrixTest, checkSquareness)
{
  ColumnMajorMatrix mat1(2, 1);
  mat1(0, 0) = 1;
  mat1(1, 0) = 2;
  try
  {
    mat1.checkSquareness();
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "ColumnMajorMatrix error: Unable to perform the operation on a non-square matrix.") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST_F(ColumnMajorMatrixTest, checkShapeEqualityTest)
{
  try
  {
    a.checkShapeEquality(t);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("ColumnMajorMatrix error: Unable to perform the operation on matrices of "
                         "different shapes.") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}
