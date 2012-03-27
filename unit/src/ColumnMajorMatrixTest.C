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

#include "ColumnMajorMatrixTest.h"

//Moose includes
#include "ColumnMajorMatrix.h"

//libMesh include
#include "vector_value.h"
#include "tensor_value.h"

CPPUNIT_TEST_SUITE_REGISTRATION( ColumnMajorMatrixTest );

void
ColumnMajorMatrixTest::setUp()
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

  add_ref(0, 0) = 6; a_sol_ref(0, 0) = 7;
  add_ref(1, 0) = 5; a_sol_ref(1, 0) = 7;
  add_ref(2, 0) = 4; a_sol_ref(2, 0) = 7;
  add_ref(0, 1) = 1; a_sol_ref(0, 1) = 5;
  add_ref(1, 1) = 1; a_sol_ref(1, 1) = 6;
  add_ref(2, 1) = 1; a_sol_ref(2, 1) = 7;

  sub = new ColumnMajorMatrix(3, 2);
  sub_solution = new ColumnMajorMatrix(3, 2);
  ColumnMajorMatrix & sub_ref = *sub;
  ColumnMajorMatrix & s_sol_ref = *sub_solution;

  sub_ref(0, 0) = 0; s_sol_ref(0, 0) = 1;
  sub_ref(1, 0) = 1; s_sol_ref(1, 0) = 1;
  sub_ref(2, 0) = 2; s_sol_ref(2, 0) = 1;
  sub_ref(0, 1) = 1; s_sol_ref(0, 1) = 3;
  sub_ref(1, 1) = 1; s_sol_ref(1, 1) = 4;
  sub_ref(2, 1) = 1; s_sol_ref(2, 1) = 5;
}

void
ColumnMajorMatrixTest::tearDown()
{
  delete a;
  delete t;
  delete add;
  delete add_solution;
  delete sub;
  delete sub_solution;
  delete two_mat;
}

void
ColumnMajorMatrixTest::addMatrixScalar()
{
  ColumnMajorMatrix add_solution(3, 3);

  add_solution(0, 0) = 11;
  add_solution(1, 0) = 12;
  add_solution(2, 0) = 13;
  add_solution(0, 1) = 14;
  add_solution(1, 1) = 15;
  add_solution(2, 1) = 16;
  add_solution(0, 2) = 17;
  add_solution(1, 2) = 18;
  add_solution(2, 2) = 19;

  CPPUNIT_ASSERT(add_solution == *a + 10);
}

void
ColumnMajorMatrixTest::divideMatrixScalarEquals()
{
  ColumnMajorMatrix divide(2, 2);

  divide(0, 0) = 2;
  divide(1, 0) = 4;
  divide(0, 1) = 6;
  divide(1, 1) = 8;

  divide /= 2;
  CPPUNIT_ASSERT( divide(0,0) == 1 );
  CPPUNIT_ASSERT( divide(1,0) == 2 );
  CPPUNIT_ASSERT( divide(0,1) == 3 );
  CPPUNIT_ASSERT( divide(1,1) == 4 );
}

void
ColumnMajorMatrixTest::addMatrixScalarEquals()
{
  ColumnMajorMatrix add_solution(3, 3);

  add_solution(0, 0) = 11;
  add_solution(1, 0) = 12;
  add_solution(2, 0) = 13;
  add_solution(0, 1) = 14;
  add_solution(1, 1) = 15;
  add_solution(2, 1) = 16;
  add_solution(0, 2) = 17;
  add_solution(1, 2) = 18;
  add_solution(2, 2) = 19;

  // Scalar add and update
  *a += 10;
  CPPUNIT_ASSERT(add_solution == *a);
}

void
ColumnMajorMatrixTest::multMatrixScalar()
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

  CPPUNIT_ASSERT(mult_solution == *a * 2);
}

void
ColumnMajorMatrixTest::multMatrixScalarEquals()
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
  *a *= 2;

  CPPUNIT_ASSERT(mult_solution == *a);
}

void
ColumnMajorMatrixTest::multMatrixMatrix()
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

  CPPUNIT_ASSERT(mult_solution == *a * *a);
}

void
ColumnMajorMatrixTest::multMatrixVec()
{
  ColumnMajorMatrix vec(3, 1), mult_solution(3, 1);

  vec(0, 0) = 1;
  vec(1, 0) = 2;
  vec(2, 0) = 3;

  mult_solution(0, 0) = 30;
  mult_solution(1, 0) = 36;
  mult_solution(2, 0) = 42;

  CPPUNIT_ASSERT(mult_solution == *a * vec);
}

void
ColumnMajorMatrixTest::reshapeMatrix()
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

  CPPUNIT_ASSERT( mat(0,1) == 3 );
  CPPUNIT_ASSERT( mat(1,2) == 6 );

  mat.reshape(1, 6);

  CPPUNIT_ASSERT( mat(0,1) == 2 );
  CPPUNIT_ASSERT( mat(0,5) == 6 );
}

void
ColumnMajorMatrixTest::setDiagMatrix()
{
  ColumnMajorMatrix mat = *two_mat;
  mat.setDiag( 9 );

  CPPUNIT_ASSERT( mat(0,0) == 9 );
  CPPUNIT_ASSERT( mat(1,0) == 2 );
  CPPUNIT_ASSERT( mat(0,1) == 3 );
  CPPUNIT_ASSERT( mat(1,1) == 9 );
}

void
ColumnMajorMatrixTest::trMatrix()
{
  CPPUNIT_ASSERT( a->tr() == 15 );
  CPPUNIT_ASSERT( two_mat->tr() == 5 );
}

void
ColumnMajorMatrixTest::zeroMatrix()
{
  ColumnMajorMatrix mat = *two_mat;
  mat.zero();

  CPPUNIT_ASSERT( mat(0,0) == 0 );
  CPPUNIT_ASSERT( mat(1,0) == 0 );
  CPPUNIT_ASSERT( mat(0,1) == 0 );
  CPPUNIT_ASSERT( mat(1,1) == 0 );
}

void
ColumnMajorMatrixTest::identityMatrix()
{
  ColumnMajorMatrix mat = *two_mat;
  mat.identity();

  CPPUNIT_ASSERT( mat(0,0) == 1 );
  CPPUNIT_ASSERT( mat(1,0) == 0 );
  CPPUNIT_ASSERT( mat(0,1) == 0 );
  CPPUNIT_ASSERT( mat(1,1) == 1 );
}

void
ColumnMajorMatrixTest::contractionMatrix()
{
  ColumnMajorMatrix mat = *two_mat;
  ColumnMajorMatrix sec(2, 2);

  sec(0, 0) = 4;
  sec(1, 0) = 3;
  sec(0, 1) = 2;
  sec(1, 1) = 1;

  CPPUNIT_ASSERT( mat.doubleContraction( sec ) == (1*4 + 2*3 + 3*2 + 4*1) );
}

void
ColumnMajorMatrixTest::normMatrix()
{
  ColumnMajorMatrix mat(2, 2);
  mat(0, 0) = 1;
  mat(1, 0) = 2;
  mat(0, 1) = 2;
  mat(1, 1) = 4;

  CPPUNIT_ASSERT( mat.norm() == 5 );
}

void
ColumnMajorMatrixTest::transposeMatrix()
{
  ColumnMajorMatrix mat = *two_mat;
  ColumnMajorMatrix test = mat.transpose();

  CPPUNIT_ASSERT( test(0,0) == 1 );
  CPPUNIT_ASSERT( test(1,0) == 3 );
  CPPUNIT_ASSERT( test(0,1) == 2 );
  CPPUNIT_ASSERT( test(1,1) == 4 );
}

void
ColumnMajorMatrixTest::rowColConstructor()
{
  ColumnMajorMatrix mat = *two_mat;

  CPPUNIT_ASSERT( mat(0,0) == 1 );
  CPPUNIT_ASSERT( mat(1,0) == 2 );
  CPPUNIT_ASSERT( mat(0,1) == 3 );
  CPPUNIT_ASSERT( mat(1,1) == 4 );
}

void
ColumnMajorMatrixTest::copyConstructor()
{
  ColumnMajorMatrix mat = *two_mat;
  ColumnMajorMatrix test = mat;
  test(1, 1) = 9;

  CPPUNIT_ASSERT( test(0,0) == 1 );
  CPPUNIT_ASSERT( test(1,0) == 2 );
  CPPUNIT_ASSERT( test(0,1) == 3 );
  CPPUNIT_ASSERT( test(1,1) == 9 );
  CPPUNIT_ASSERT( mat(1,1) == 4 );

  CPPUNIT_ASSERT( test.numEntries() == 4 );
}

void
ColumnMajorMatrixTest::tensorConstructor()
{
  TensorValue<Real> tensor( 1, 4, 7,
                            2, 5, 8,
                            3, 6, 9 );
  ColumnMajorMatrix test( tensor );

  CPPUNIT_ASSERT( test == *a );
  CPPUNIT_ASSERT( test.numEntries() == 9 );
}

void
ColumnMajorMatrixTest::ThreeColConstructor()
{
  VectorValue<Real> col1( 1, 2, 3 );
  VectorValue<Real> col2( 4, 5, 6 );
  VectorValue<Real> col3( 7, 8, 9 );

  ColumnMajorMatrix test( col1, col2, col3 );

  CPPUNIT_ASSERT( test == *a );
  CPPUNIT_ASSERT( test.numEntries() == 9 );
}

void
ColumnMajorMatrixTest::numEntries()
{
  //numEntries is tested in other functions, like after different
  //constructors to make sure the number of entries copied over correctly
  ColumnMajorMatrix mat = *two_mat;
  ColumnMajorMatrix mat2(3, 4);

  CPPUNIT_ASSERT( mat.numEntries() == 4 );
  CPPUNIT_ASSERT( mat2.numEntries() == 12 );
}

void
ColumnMajorMatrixTest::accessMatrix()
{
  //tests operator()
  ColumnMajorMatrix mat = *two_mat;

  CPPUNIT_ASSERT( mat(0,0) == 1 );
  CPPUNIT_ASSERT( mat(1,0) == 2 );
  CPPUNIT_ASSERT( mat(0,1) == 3 );
  CPPUNIT_ASSERT( mat(1,1) == 4 );
}

void
ColumnMajorMatrixTest::print()
{
  //TODO?
}

void
ColumnMajorMatrixTest::fillMatrix()
{
  TensorValue<Real> tensor( 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  ColumnMajorMatrix & a_ref = *a;
  a_ref.fill( tensor );

  CPPUNIT_ASSERT( tensor(0, 0) == a_ref(0, 0) );
  CPPUNIT_ASSERT( tensor(1, 0) == a_ref(1, 0) );
  CPPUNIT_ASSERT( tensor(2, 0) == a_ref(2, 0) );
  CPPUNIT_ASSERT( tensor(0, 1) == a_ref(0, 1) );
  CPPUNIT_ASSERT( tensor(1, 1) == a_ref(1, 1) );
  CPPUNIT_ASSERT( tensor(2, 1) == a_ref(2, 1) );
  CPPUNIT_ASSERT( tensor(0, 2) == a_ref(0, 2) );
  CPPUNIT_ASSERT( tensor(1, 2) == a_ref(1, 2) );
  CPPUNIT_ASSERT( tensor(2, 2) == a_ref(2, 2) );
}

void
ColumnMajorMatrixTest::tensorAssignOperator()
{
  ColumnMajorMatrix test(3, 3);
  TensorValue<Real> tensor( 1, 4, 7,
                            2, 5, 8,
                            3, 6, 9 );
  test = tensor;

  CPPUNIT_ASSERT( test == *a );
  CPPUNIT_ASSERT( test.numEntries() == 9 );
}

void
ColumnMajorMatrixTest::matrixAssignOperator()
{
  ColumnMajorMatrix mat = *two_mat;
  ColumnMajorMatrix test(1, 2);

  test = mat;
  test(1, 1) = 9;

  CPPUNIT_ASSERT( test(0,0) == 1 );
  CPPUNIT_ASSERT( test(1,0) == 2 );
  CPPUNIT_ASSERT( test(0,1) == 3 );
  CPPUNIT_ASSERT( test(1,1) == 9 );
  CPPUNIT_ASSERT( mat(1,1) == 4 );

  CPPUNIT_ASSERT( test.numEntries() == 4 );
}

void
ColumnMajorMatrixTest::addMatrixMatrix()
{
  CPPUNIT_ASSERT( *t + *add == *add_solution );
}

void
ColumnMajorMatrixTest::addMatrixMatrixEquals()
{
  ColumnMajorMatrix mat = *t;
  mat += *add;

  CPPUNIT_ASSERT( mat == *add_solution );
}

void
ColumnMajorMatrixTest::subMatrixMatrixEquals()
{
  ColumnMajorMatrix mat = *t;
  mat -= *sub;

  CPPUNIT_ASSERT( mat == *sub_solution );
}

void
ColumnMajorMatrixTest::equalMatrix()
{
  ColumnMajorMatrix mat(1, 1);
  ColumnMajorMatrix mat1(1, 1);

  mat(0, 0) = 2;
  mat1(0, 0) = 2;

  CPPUNIT_ASSERT( !(*a == *t) );
  CPPUNIT_ASSERT( mat == mat1 );
}

void
ColumnMajorMatrixTest::notEqualMatrix()
{
  ColumnMajorMatrix mat(1, 1);
  ColumnMajorMatrix mat1(1, 1);

  mat(0, 0) = 2;
  mat1(0, 0) = 2;

  CPPUNIT_ASSERT( *a != *t );
  CPPUNIT_ASSERT( !(mat != mat1) );
}

void
ColumnMajorMatrixTest::kronecker()
{
  ColumnMajorMatrix rhs(2, 2);
  rhs(0, 0) = 1;
  rhs(0, 1) = 2;
  rhs(1, 0) = 3;
  rhs(1, 1) = 4;

  ColumnMajorMatrix lhs(2, 2);
  lhs(0, 0) = 0;
  lhs(0, 1) = 5;
  lhs(1, 0) = 6;
  lhs(1, 1) = 7;

  ColumnMajorMatrix ans = rhs.kronecker(lhs);

  CPPUNIT_ASSERT( ans(0,0) == 0 );
  CPPUNIT_ASSERT( ans(0,3) == 10 );
  CPPUNIT_ASSERT( ans(1,0) == 6 );
  CPPUNIT_ASSERT( ans(1,1) == 7 );
  CPPUNIT_ASSERT( ans(1,3) == 14 );
  CPPUNIT_ASSERT( ans(2,1) == 15 );
  CPPUNIT_ASSERT( ans(2,2) == 0 );
  CPPUNIT_ASSERT( ans(3,0) == 18 );
  CPPUNIT_ASSERT( ans(3,1) == 21 );
  CPPUNIT_ASSERT( ans(3,2) == 24 );
  CPPUNIT_ASSERT( ans(3,3) == 28 );
}

void
ColumnMajorMatrixTest::inverse()
{
  ColumnMajorMatrix matrix(3,3), matrix_inverse(3,3);

  matrix(0,0) = 1.0;
  matrix(0,1) = 3.0;
  matrix(0,2) = 3.0;
  
  matrix(1,0) = 1.0;
  matrix(1,1) = 4.0;
  matrix(1,2) = 3.0;

  matrix(2,0) = 1.0;
  matrix(2,1) = 3.0;
  matrix(2,2) = 4.0;

  matrix.inverse(matrix_inverse);


  CPPUNIT_ASSERT( matrix_inverse(0,0) == 7.0 );
  CPPUNIT_ASSERT( matrix_inverse(0,1) == -3.0 );
  CPPUNIT_ASSERT( matrix_inverse(0,2) == -3.0 );
  
  CPPUNIT_ASSERT( matrix_inverse(1,0) == -1.0 );
  CPPUNIT_ASSERT( matrix_inverse(1,1) == 1.0 );
  CPPUNIT_ASSERT( matrix_inverse(1,2) == 0.0 );
  
  CPPUNIT_ASSERT( matrix_inverse(2,0) == -1.0 );
  CPPUNIT_ASSERT( matrix_inverse(2,1) == 0.0 );
  CPPUNIT_ASSERT( matrix_inverse(2,2) == 1.0 );
  
}


