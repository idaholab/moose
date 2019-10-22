//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "ExpressionBuilder.h"

class ExpressionBuilderMatrixVectorTest : public ::testing::Test, public ExpressionBuilder
{
};

TEST_F(ExpressionBuilderMatrixVectorTest, test)
{
  // Testing Matrix Operatrions

  EBTerm a1("a11"), a2("a12"), a3("a13"), a4("a21"), a5("a22"), a6("a23"), a7("a31"), a8("a32"),
      a9("a33");
  EBTerm b1("b11"), b2("b12"), b3("b13"), b4("b21"), b5("b22"), b6("b23"), b7("b31"), b8("b32"),
      b9("b33");

  EBMatrix a_matrix(a1, a2, a3, a4, a5, a6, a7, a8, a9);
  EBMatrix b_matrix(b1, b2, b3, b4, b5, b6, b7, b8, b9);

  // Test Matrix Multiplication
  {
    EBMatrix mult_mat = a_matrix * b_matrix;
    EXPECT_EQ(std::string(mult_mat[0][0]), "0+a11*b11+a12*b21+a13*b31");
    EXPECT_EQ(std::string(mult_mat[0][1]), "0+a11*b12+a12*b22+a13*b32");
    EXPECT_EQ(std::string(mult_mat[0][2]), "0+a11*b13+a12*b23+a13*b33");
    EXPECT_EQ(std::string(mult_mat[1][0]), "0+a21*b11+a22*b21+a23*b31");
    EXPECT_EQ(std::string(mult_mat[1][1]), "0+a21*b12+a22*b22+a23*b32");
    EXPECT_EQ(std::string(mult_mat[1][2]), "0+a21*b13+a22*b23+a23*b33");
    EXPECT_EQ(std::string(mult_mat[2][0]), "0+a31*b11+a32*b21+a33*b31");
    EXPECT_EQ(std::string(mult_mat[2][1]), "0+a31*b12+a32*b22+a33*b32");
    EXPECT_EQ(std::string(mult_mat[2][2]), "0+a31*b13+a32*b23+a33*b33");
  }

  // Test Matrix Addition and Subtraction
  {
    EBMatrix add_mat = a_matrix + b_matrix;
    EXPECT_EQ(std::string(add_mat[0][0]), "b11+a11");
    EXPECT_EQ(std::string(add_mat[0][1]), "b12+a12");
    EXPECT_EQ(std::string(add_mat[0][2]), "b13+a13");
    EXPECT_EQ(std::string(add_mat[1][0]), "b21+a21");
    EXPECT_EQ(std::string(add_mat[1][1]), "b22+a22");
    EXPECT_EQ(std::string(add_mat[1][2]), "b23+a23");
    EXPECT_EQ(std::string(add_mat[2][0]), "b31+a31");
    EXPECT_EQ(std::string(add_mat[2][1]), "b32+a32");
    EXPECT_EQ(std::string(add_mat[2][2]), "b33+a33");

    EBMatrix sub_mat = a_matrix - b_matrix;
    EXPECT_EQ(std::string(sub_mat[0][0]), "-1*b11+a11");
    EXPECT_EQ(std::string(sub_mat[0][1]), "-1*b12+a12");
    EXPECT_EQ(std::string(sub_mat[0][2]), "-1*b13+a13");
    EXPECT_EQ(std::string(sub_mat[1][0]), "-1*b21+a21");
    EXPECT_EQ(std::string(sub_mat[1][1]), "-1*b22+a22");
    EXPECT_EQ(std::string(sub_mat[1][2]), "-1*b23+a23");
    EXPECT_EQ(std::string(sub_mat[2][0]), "-1*b31+a31");
    EXPECT_EQ(std::string(sub_mat[2][1]), "-1*b32+a32");
    EXPECT_EQ(std::string(sub_mat[2][2]), "-1*b33+a33");
  }

  // Test Scalar Multiplication of a Matrix
  {
    EBMatrix scal_mult_mat = 2 * b_matrix;
    EXPECT_EQ(std::string(scal_mult_mat[0][0]), "2*b11");
    EXPECT_EQ(std::string(scal_mult_mat[0][1]), "2*b12");
    EXPECT_EQ(std::string(scal_mult_mat[0][2]), "2*b13");
    EXPECT_EQ(std::string(scal_mult_mat[1][0]), "2*b21");
    EXPECT_EQ(std::string(scal_mult_mat[1][1]), "2*b22");
    EXPECT_EQ(std::string(scal_mult_mat[1][2]), "2*b23");
    EXPECT_EQ(std::string(scal_mult_mat[2][0]), "2*b31");
    EXPECT_EQ(std::string(scal_mult_mat[2][1]), "2*b32");
    EXPECT_EQ(std::string(scal_mult_mat[2][2]), "2*b33");

    scal_mult_mat = a1 * b_matrix;
    EXPECT_EQ(std::string(scal_mult_mat[0][0]), "a11*b11");
    EXPECT_EQ(std::string(scal_mult_mat[0][1]), "a11*b12");
    EXPECT_EQ(std::string(scal_mult_mat[0][2]), "a11*b13");
    EXPECT_EQ(std::string(scal_mult_mat[1][0]), "a11*b21");
    EXPECT_EQ(std::string(scal_mult_mat[1][1]), "a11*b22");
    EXPECT_EQ(std::string(scal_mult_mat[1][2]), "a11*b23");
    EXPECT_EQ(std::string(scal_mult_mat[2][0]), "a11*b31");
    EXPECT_EQ(std::string(scal_mult_mat[2][1]), "a11*b32");
    EXPECT_EQ(std::string(scal_mult_mat[2][2]), "a11*b33");
  }

  // Test Matrix transpose
  {
    EBMatrix transp_mat = a_matrix.transpose();
    EXPECT_EQ(std::string(transp_mat[0][0]), "a11");
    EXPECT_EQ(std::string(transp_mat[0][1]), "a21");
    EXPECT_EQ(std::string(transp_mat[0][2]), "a31");
    EXPECT_EQ(std::string(transp_mat[1][0]), "a12");
    EXPECT_EQ(std::string(transp_mat[1][1]), "a22");
    EXPECT_EQ(std::string(transp_mat[1][2]), "a32");
    EXPECT_EQ(std::string(transp_mat[2][0]), "a13");
    EXPECT_EQ(std::string(transp_mat[2][1]), "a23");
    EXPECT_EQ(std::string(transp_mat[2][2]), "a33");
  }

  // Testing Vector operations

  EBVector a_vec(a1, a2, a3);
  EBVector b_vec(b1, b2, b3);
  // Test Dot Product
  {
    EXPECT_EQ(std::string(a_vec * b_vec), "0+a11*b11+a12*b12+a13*b13");
  }

  // Test Scalar Multiplication
  {
    EBVector scal_mult_vec = a1 * b_vec;
    EXPECT_EQ(std::string(scal_mult_vec[0]), "b11*a11");
    EXPECT_EQ(std::string(scal_mult_vec[1]), "b12*a11");
    EXPECT_EQ(std::string(scal_mult_vec[2]), "b13*a11");

    scal_mult_vec = 2 * b_vec;
    EXPECT_EQ(std::string(scal_mult_vec[0]), "2*b11");
    EXPECT_EQ(std::string(scal_mult_vec[1]), "2*b12");
    EXPECT_EQ(std::string(scal_mult_vec[2]), "2*b13");
  }

  // Test Vector Matrix Multiplication
  {
    EBVector vec_mat_mult = a_vec * b_matrix;
    EXPECT_EQ(std::string(vec_mat_mult[0]), "0+a11*b11+a12*b21+a13*b31");
    EXPECT_EQ(std::string(vec_mat_mult[1]), "0+a11*b12+a12*b22+a13*b32");
    EXPECT_EQ(std::string(vec_mat_mult[2]), "0+a11*b13+a12*b23+a13*b33");

    vec_mat_mult = b_matrix * a_vec;
    EXPECT_EQ(std::string(vec_mat_mult[0]), "0+b11*a11+b12*a12+b13*a13");
    EXPECT_EQ(std::string(vec_mat_mult[1]), "0+b21*a11+b22*a12+b23*a13");
    EXPECT_EQ(std::string(vec_mat_mult[2]), "0+b31*a11+b32*a12+b33*a13");
  }

  // Test Vector Addition and Subtraction
  {
    EBVector add_vec = a_vec + b_vec;
    EXPECT_EQ(std::string(add_vec[0]), "a11+b11");
    EXPECT_EQ(std::string(add_vec[1]), "a12+b12");
    EXPECT_EQ(std::string(add_vec[2]), "a13+b13");

    EBVector sub_vec = a_vec - b_vec;
    EXPECT_EQ(std::string(sub_vec[0]), "a11+-1*b11"); // a11 + (-1)*b11
    EXPECT_EQ(std::string(sub_vec[1]), "a12+-1*b12");
    EXPECT_EQ(std::string(sub_vec[2]), "a13+-1*b13");
  }

  // Test Cross product
  {
    EBVector cross_prod = EBVector::cross(a_vec, b_vec);
    EXPECT_EQ(std::string(cross_prod[0]), "a12*b13-a13*b12");
    EXPECT_EQ(std::string(cross_prod[1]), "a11*b13-a13*b11");
    EXPECT_EQ(std::string(cross_prod[2]), "a11*b12-a12*b11");
  }

  // Test Vector Normalization
  {
    EXPECT_EQ(std::string(a_vec.norm()), "sqrt(0+a11*a11+a12*a12+a13*a13)");
  }
}
