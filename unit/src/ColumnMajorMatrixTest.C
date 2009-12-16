#include "ColumnMajorMatrixTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( ColumnMajorMatrixTest );

void
ColumnMajorMatrixTest::setUp()
{
  a = new ColumnMajorMatrix(3, 3);
  ColumnMajorMatrix &a_ref = *a;
  
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
  ColumnMajorMatrix &t_ref = *t;

  t_ref(0, 0) = 1;
  t_ref(1, 0) = 2;
  t_ref(2, 0) = 3;
  t_ref(0, 1) = 4;
  t_ref(1, 1) = 5;
  t_ref(2, 1) = 6;
}

void
ColumnMajorMatrixTest::tearDown()
{
  delete a;
  delete t;
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
