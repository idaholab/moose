/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef COLUMNMAJORMATRIXTEST_H
#define COLUMNMAJORMATRIXTEST_H

// CPPUnit includes
#include "gtest/gtest.h"

// Moose includes
#include "ColumnMajorMatrix.h"

// libMesh include
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"

class ColumnMajorMatrixTest : public ::testing::Test
{
protected:
  void SetUp()
  {
    // Define commonly used matrices for testing
    a = new ColumnMajorMatrix(3, 3);
    ColumnMajorMatrix & a_ref = *a;

    a_ref(0, 0) = 1;
    a_ref(1, 0) = 2;
    a_ref(2, 0) = 3;
    a_ref(0, 1) = 4;
    a_ref(1, 1) = 5;
    a_ref(2, 1) = 6;
    a_ref(0, 2) = 7;
    a_ref(1, 2) = 8;
    a_ref(2, 2) = 9;

    t = new ColumnMajorMatrix(3, 2);
    ColumnMajorMatrix & t_ref = *t;

    t_ref(0, 0) = 1;
    t_ref(1, 0) = 2;
    t_ref(2, 0) = 3;
    t_ref(0, 1) = 4;
    t_ref(1, 1) = 5;
    t_ref(2, 1) = 6;

    two_mat = new ColumnMajorMatrix(2, 2);
    ColumnMajorMatrix & mat = *two_mat;

    mat(0, 0) = 1;
    mat(1, 0) = 2;
    mat(0, 1) = 3;
    mat(1, 1) = 4;

    add = new ColumnMajorMatrix(3, 2);
    add_solution = new ColumnMajorMatrix(3, 2);
    ColumnMajorMatrix & add_ref = *add;
    ColumnMajorMatrix & a_sol_ref = *add_solution;

    add_ref(0, 0) = 6;
    a_sol_ref(0, 0) = 7;
    add_ref(1, 0) = 5;
    a_sol_ref(1, 0) = 7;
    add_ref(2, 0) = 4;
    a_sol_ref(2, 0) = 7;
    add_ref(0, 1) = 1;
    a_sol_ref(0, 1) = 5;
    add_ref(1, 1) = 1;
    a_sol_ref(1, 1) = 6;
    add_ref(2, 1) = 1;
    a_sol_ref(2, 1) = 7;

    sub = new ColumnMajorMatrix(3, 2);
    sub_solution = new ColumnMajorMatrix(3, 2);
    ColumnMajorMatrix & sub_ref = *sub;
    ColumnMajorMatrix & s_sol_ref = *sub_solution;

    sub_ref(0, 0) = 0;
    s_sol_ref(0, 0) = 1;
    sub_ref(1, 0) = 1;
    s_sol_ref(1, 0) = 1;
    sub_ref(2, 0) = 2;
    s_sol_ref(2, 0) = 1;
    sub_ref(0, 1) = 1;
    s_sol_ref(0, 1) = 3;
    sub_ref(1, 1) = 1;
    s_sol_ref(1, 1) = 4;
    sub_ref(2, 1) = 1;
    s_sol_ref(2, 1) = 5;
  }

  void TearDown()
  {
    delete a;
    delete t;
    delete add;
    delete add_solution;
    delete sub;
    delete sub_solution;
    delete two_mat;
  }

  ColumnMajorMatrix *a, *t, *two_mat;
  ColumnMajorMatrix *add, *add_solution;
  ColumnMajorMatrix *sub, *sub_solution;
};

#endif // COLUMNMAJORMATRIXTEST_H
