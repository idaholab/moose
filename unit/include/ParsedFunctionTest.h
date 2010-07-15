#ifndef USERFUNCTIONTEST_H
#define USERFUNCTIONTEST_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

class ParsedFunctionTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE( ParsedFunctionTest );

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

#endif  // USERFUNCTIONTEST_H
