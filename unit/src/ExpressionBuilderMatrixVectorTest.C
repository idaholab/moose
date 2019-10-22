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

  EBTerm r("r"), s("s"), t("t"), u("u"), v("v"), w("w"), x("x"), y("y"), z("z");
  EBTerm a("a"), b("b"), c("c"), d("d"), e("e"), f("f"), g("g"), h("h"), i("i");

  EBMatrix lower_letters(r, s, t, u, v, w, x, y, z);
  EBMatrix upper_letters(a, b, c, d, e, f, g, h, i);

  // Test Matrix Multiplication
  {
    EBMatrix mult_mat = lower_letters * upper_letters;
    std::cout << mult_mat[0][0] << std::endl;
    std::cout << mult_mat[0][0] << std::endl;
    std::cout << mult_mat[0][1] << std::endl;
    std::cout << mult_mat[0][2] << std::endl;
    std::cout << mult_mat[1][0] << std::endl;
    std::cout << mult_mat[1][1] << std::endl;
    std::cout << mult_mat[1][2] << std::endl;
    std::cout << mult_mat[2][0] << std::endl;
    std::cout << mult_mat[2][1] << std::endl;
    std::cout << mult_mat[2][2] << std::endl;
  }

  // Test Matrix Addition and Subtraction
  {
    EBMatrix add_mat = lower_letters * upper_letters;
    std::cout << add_mat[0][0] << std::endl;
    std::cout << add_mat[0][0] << std::endl;
    std::cout << add_mat[0][1] << std::endl;
    std::cout << add_mat[0][2] << std::endl;
    std::cout << add_mat[1][0] << std::endl;
    std::cout << add_mat[1][1] << std::endl;
    std::cout << add_mat[1][2] << std::endl;
    std::cout << add_mat[2][0] << std::endl;
    std::cout << add_mat[2][1] << std::endl;
    std::cout << add_mat[2][2] << std::endl;

    EBMatrix sub_mat = lower_letters * upper_letters;
    std::cout << sub_mat[0][0] << std::endl;
    std::cout << sub_mat[0][0] << std::endl;
    std::cout << sub_mat[0][1] << std::endl;
    std::cout << sub_mat[0][2] << std::endl;
    std::cout << sub_mat[1][0] << std::endl;
    std::cout << sub_mat[1][1] << std::endl;
    std::cout << sub_mat[1][2] << std::endl;
    std::cout << sub_mat[2][0] << std::endl;
    std::cout << sub_mat[2][1] << std::endl;
    std::cout << sub_mat[2][2] << std::endl;
  }

  // Test Scalar Multiplication of a Matrix
  {
    EBMatrix scal_mult_mat = 2 * lower_letters;
    std::cout << scal_mult_mat[0][0] << std::endl;
    std::cout << scal_mult_mat[0][2] << std::endl;
    std::cout << scal_mult_mat[0][1] << std::endl;
    std::cout << scal_mult_mat[1][0] << std::endl;
    std::cout << scal_mult_mat[1][1] << std::endl;
    std::cout << scal_mult_mat[1][2] << std::endl;
    std::cout << scal_mult_mat[2][0] << std::endl;
    std::cout << scal_mult_mat[2][1] << std::endl;
    std::cout << scal_mult_mat[2][2] << std::endl;

    scal_mult_mat = a * lower_letters;
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
    EBMatrix transp_mat = lower_letters.transpose();
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

  EBVector abc_vec(a, b, c);
  EBVector rst_vec(r, s, t);
  // Test Dot Product
  {
    std::cout << abc_vec * rst_vec << std::endl;
  }

  // Test Scalar Multiplication
  {
    EBVector scal_mult_vec = a * rst_vec;
    std::cout << scal_mult_vec[0] << std::endl;
    std::cout << scal_mult_vec[1] << std::endl;
    std::cout << scal_mult_vec[2] << std::endl;

    scal_mult_vec = 2 * rst_vec;
    std::cout << scal_mult_vec[0] << std::endl;
    std::cout << scal_mult_vec[1] << std::endl;
    std::cout << scal_mult_vec[2] << std::endl;
  }

  // Test Vector Matrix Multiplication
  {
    EBVector vec_mat_mult = rst_vec * upper_letters;
    std::cout << vec_mat_mult[0] << std::endl;
    std::cout << vec_mat_mult[1] << std::endl;
    std::cout << vec_mat_mult[2] << std::endl;

    vec_mat_mult = upper_letters * rst_vec;
    std::cout << vec_mat_mult[0] << std::endl;
    std::cout << vec_mat_mult[1] << std::endl;
    std::cout << vec_mat_mult[2] << std::endl;
  }

  // Test Vector Addition and Subtraction
  {
    EBVector add_vec = abc_vec + rst_vec;
    std::cout << add_vec[0] << std::endl;
    std::cout << add_vec[1] << std::endl;
    std::cout << add_vec[2] << std::endl;

    EBVector sub_vec = abc_vec - rst_vec;
    std::cout << sub_vec[0] << std::endl;
    std::cout << sub_vec[1] << std::endl;
    std::cout << sub_vec[2] << std::endl;
  }

  // Test Cross product
  {
    EBVector cross_prod = EBVector::cross(abc_vec, rst_vec);
    std::cout << cross_prod[0] << std::endl;
    std::cout << cross_prod[1] << std::endl;
    std::cout << cross_prod[2] << std::endl;
  }

  // Test Vector Normalization
  {
    std::cout << abc_vec.norm() << std::endl;
  }
}
