#ifndef COLUMNMAJORMATRIXTEST_H
#define COLUMNMAJORMATRIXTEST_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

//Moose includes
#include "ColumnMajorMatrix.h"

class ColumnMajorMatrixTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE( ColumnMajorMatrixTest );
  CPPUNIT_TEST( addMatrixScalar );
  CPPUNIT_TEST( multMatrixScalar );
  CPPUNIT_TEST_SUITE_END();
  
public:
  void setUp();
  void tearDown();
  
  void addMatrixScalar();
  void multMatrixScalar();

private:
  ColumnMajorMatrix *a; 
};

  

#endif
