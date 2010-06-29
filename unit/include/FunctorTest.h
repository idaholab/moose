#ifndef FUNCTORTEST_H
#define FUNCTORTEST_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

class Functor;

class FunctorTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE( FunctorTest );

  CPPUNIT_TEST( basicConstructor );
  CPPUNIT_TEST( advancedConstructor );
  CPPUNIT_TEST( testVariables );
  CPPUNIT_TEST( testConstants );

  CPPUNIT_TEST_SUITE_END();
  
public:
  void basicConstructor();
  void advancedConstructor();
  void testVariables();
  void testConstants();

private:
};

#endif  // FUNCTORTEST_H
