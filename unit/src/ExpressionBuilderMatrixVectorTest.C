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
    EXPECT_EQ(std::string(mult_mat[0][0],"0+a11*b11+a12*b21+b13*b31");
    EXPECT_EQ(std::string(mult_mat[0][1],"0+a11*b12+a12*b22+a13*b32");
    EXPECT_EQ(std::string(mult_mat[0][2],"0+a11*b13+a12*b23+a13*b33");
    EXPECT_EQ(std::string(mult_mat[1][0],"0+a21*b11+a22*b21+a23*b31");
    EXPECT_EQ(std::string(mult_mat[1][1],"0+a21*b12+a22*b22+a23*b32");
    EXPECT_EQ(std::string(mult_mat[1][2],"0+a21*b13+a22*b23+a23*b33");
    EXPECT_EQ(std::string(mult_mat[2][0],"0+a31*b11+a32*b21+a33*b31");
    EXPECT_EQ(std::string(mult_mat[2][1],"0+a31*b12+a32*b22+a33*b32");
    EXPECT_EQ(std::string(mult_mat[2][2],"0+a31*b13+a32*b23+a33*b33");
  }

  // Test Matrix Addition and Subtraction
  {
    EBMatrix add_mat = a_matrix + b_matrix;
    std::cout << add_mat[0][0] << std::endl;
    EXPECT_EQ(std::string(add_mat[0][0],"0+a11+b11");
    EXPECT_EQ(std::string(add_mat[0][1],"0+a12+b12");
    EXPECT_EQ(std::string(add_mat[0][2],"0+a13+b13");
    EXPECT_EQ(std::string(add_mat[1][0],"0+a21+b21");
    EXPECT_EQ(std::string(add_mat[1][1],"0+a22+b22");
    EXPECT_EQ(std::string(add_mat[1][2],"0+a23+b23");
    EXPECT_EQ(std::string(add_mat[2][0],"0+a31+b31");
    EXPECT_EQ(std::string(add_mat[2][1],"0+a32+b32");
    EXPECT_EQ(std::string(add_mat[2][2],"0+a33+b33");

    EBMatrix sub_mat = a_matrix - b_matrix;
    std::cout << sub_mat[0][0] << std::endl;
    EXPECT_EQ(std::string(sub_mat[0][0],"0+a11-b11");
    EXPECT_EQ(std::string(sub_mat[0][1],"0+a12-b12");
    EXPECT_EQ(std::string(sub_mat[0][2],"0+a13-b13");
    EXPECT_EQ(std::string(sub_mat[1][0],"0+a21-b21");
    EXPECT_EQ(std::string(sub_mat[1][1],"0+a22-b22");
    EXPECT_EQ(std::string(sub_mat[1][2],"0+a23-b23");
    EXPECT_EQ(std::string(sub_mat[2][0],"0+a31-b31");
    EXPECT_EQ(std::string(sub_mat[2][1],"0+a32-b32");
    EXPECT_EQ(std::string(sub_mat[2][2],"0+a33-b33");
  }

  // Test Scalar Multiplication of a Matrix
  {
    EBMatrix scal_mult_mat = 2 * b_matrix;
    std::cout << scal_mult_mat[0][0] << std::endl;
    std::cout << scal_mult_mat[0][1] << std::endl;
    std::cout << scal_mult_mat[0][2] << std::endl;
    std::cout << scal_mult_mat[1][0] << std::endl;
    std::cout << scal_mult_mat[1][1] << std::endl;
    std::cout << scal_mult_mat[1][2] << std::endl;
    std::cout << scal_mult_mat[2][0] << std::endl;
    std::cout << scal_mult_mat[2][1] << std::endl;
    std::cout << scal_mult_mat[2][2] << std::endl;

    scal_mult_mat = a1 * b_matrix;
    std::cout << scal_mult_mat[0][0] << std::endl;
    std::cout << scal_mult_mat[0][1] << std::endl;
    std::cout << scal_mult_mat[0][2] << std::endl;
    std::cout << scal_mult_mat[1][0] << std::endl;
    std::cout << scal_mult_mat[1][1] << std::endl;
    std::cout << scal_mult_mat[1][2] << std::endl;
    std::cout << scal_mult_mat[2][0] << std::endl;
    std::cout << scal_mult_mat[2][1] << std::endl;
    std::cout << scal_mult_mat[2][2] << std::endl;
  }

  // Test Matrix transpose
  {
    EBMatrix transp_mat = a_matrix.transpose();
    std::cout << transp_mat[0][0] << std::endl;
    std::cout << transp_mat[0][1] << std::endl;
    std::cout << transp_mat[0][2] << std::endl;
    std::cout << transp_mat[1][0] << std::endl;
    std::cout << transp_mat[1][1] << std::endl;
    std::cout << transp_mat[1][2] << std::endl;
    std::cout << transp_mat[2][0] << std::endl;
    std::cout << transp_mat[2][1] << std::endl;
    std::cout << transp_mat[2][2] << std::endl;
  }

  // Testing Vector operations

  EBVector a_vec(a1, a2, a3);
  EBVector b_vec(b1, b2, b3);
  // Test Dot Product
  {
    std::cout << a_vec * b_vec << std::endl;
  }

  // Test Scalar Multiplication
  {
    EBVector scal_mult_vec = a1 * b_vec;
    std::cout << scal_mult_vec[0] << std::endl;
    std::cout << scal_mult_vec[1] << std::endl;
    std::cout << scal_mult_vec[2] << std::endl;

    scal_mult_vec = 2 * b_vec;
    std::cout << scal_mult_vec[0] << std::endl;
    std::cout << scal_mult_vec[1] << std::endl;
    std::cout << scal_mult_vec[2] << std::endl;
  }

  // Test Vector Matrix Multiplication
  {
    EBVector vec_mat_mult = a_vec * b_matrix;
    std::cout << vec_mat_mult[0] << std::endl;
    std::cout << vec_mat_mult[1] << std::endl;
    std::cout << vec_mat_mult[2] << std::endl;

    vec_mat_mult = b_matrix * a_vec;
    std::cout << vec_mat_mult[0] << std::endl;
    std::cout << vec_mat_mult[1] << std::endl;
    std::cout << vec_mat_mult[2] << std::endl;
  }

  // Test Vector Addition and Subtraction
  {
    EBVector add_vec = a_vec + b_vec;
    std::cout << add_vec[0] << std::endl;
    std::cout << add_vec[1] << std::endl;
    std::cout << add_vec[2] << std::endl;

    EBVector sub_vec = a_vec - b_vec;
    std::cout << sub_vec[0] << std::endl;
    std::cout << sub_vec[1] << std::endl;
    std::cout << sub_vec[2] << std::endl;
  }

  // Test Cross product
  {
    EBVector cross_prod = EBVector::cross(a_vec, b_vec);
    std::cout << cross_prod[0] << std::endl;
    std::cout << cross_prod[1] << std::endl;
    std::cout << cross_prod[2] << std::endl;
  }

  // Test Vector Normalization
  {
    std::cout << a_vec.norm() << std::endl;
  }
}
