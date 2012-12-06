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



//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

class ColumnMajorMatrix;

class ColumnMajorMatrixTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( ColumnMajorMatrixTest );

  CPPUNIT_TEST( rowColConstructor );
  CPPUNIT_TEST( copyConstructor );
  CPPUNIT_TEST( tensorConstructor );
  CPPUNIT_TEST( ThreeColConstructor );
  CPPUNIT_TEST( numEntries );
  CPPUNIT_TEST( reshapeMatrix );
  CPPUNIT_TEST( accessMatrix );
  CPPUNIT_TEST( print );
  CPPUNIT_TEST( fillMatrix );
  CPPUNIT_TEST( transposeMatrix );
  CPPUNIT_TEST( setDiagMatrix );
  CPPUNIT_TEST( trMatrix );
  CPPUNIT_TEST( zeroMatrix );
  CPPUNIT_TEST( identityMatrix );
  CPPUNIT_TEST( contractionMatrix );
  CPPUNIT_TEST( normMatrix );
  CPPUNIT_TEST( tensorAssignOperator );
  CPPUNIT_TEST( matrixAssignOperator );
  CPPUNIT_TEST( multMatrixScalar );
  CPPUNIT_TEST( multMatrixVec );
  CPPUNIT_TEST( multMatrixMatrix );
  CPPUNIT_TEST( addMatrixMatrix );
  CPPUNIT_TEST( addMatrixMatrixEquals );
  CPPUNIT_TEST( subMatrixMatrixEquals );
  CPPUNIT_TEST( addMatrixScalar );
  CPPUNIT_TEST( divideMatrixScalarEquals );
  CPPUNIT_TEST( multMatrixScalarEquals );
  CPPUNIT_TEST( addMatrixScalarEquals );
  CPPUNIT_TEST( equalMatrix );
  CPPUNIT_TEST( notEqualMatrix );
  CPPUNIT_TEST( kronecker );
  CPPUNIT_TEST( inverse );
  CPPUNIT_TEST( eigen );
  CPPUNIT_TEST( eigenNonsym );
  CPPUNIT_TEST( exp );

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void rowColConstructor();
  void copyConstructor();
  void tensorConstructor();
  void ThreeColConstructor();
  void numEntries();
  void reshapeMatrix();
  void accessMatrix();
  void print();
  void fillMatrix();
  void transposeMatrix();
  void setDiagMatrix();
  void trMatrix();
  void zeroMatrix();
  void identityMatrix();
  void contractionMatrix();
  void normMatrix();
  void tensorAssignOperator();
  void matrixAssignOperator();
  void multMatrixScalar();
  void multMatrixVec();
  void multMatrixMatrix();
  void addMatrixMatrix();
  void addMatrixMatrixEquals();
  void subMatrixMatrixEquals();
  void addMatrixScalar();
  void divideMatrixScalarEquals();
  void multMatrixScalarEquals();
  void addMatrixScalarEquals();
  void equalMatrix();
  void notEqualMatrix();
  void kronecker();
  void inverse();
  void eigen();
  void eigenNonsym();
  void exp();

private:
  ColumnMajorMatrix *a, *t, *two_mat;
  ColumnMajorMatrix *add, *add_solution;
  ColumnMajorMatrix *sub, *sub_solution;
};

#endif  // COLUMNMAJORMATRIXTEST_H
