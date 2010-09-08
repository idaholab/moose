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
