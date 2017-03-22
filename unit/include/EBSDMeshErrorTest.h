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

#ifndef EBSDMESHERRORTEST_H
#define EBSDMESHERRORTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

// Forward declarations
class Factory;
class MooseApp;

class EBSDMeshErrorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(EBSDMeshErrorTest);

  CPPUNIT_TEST(geometrySpecifiedError);
  CPPUNIT_TEST(fileDoesNotExist);
  CPPUNIT_TEST(headerError);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void fileDoesNotExist();
  void headerError();
  void geometrySpecifiedError();

private:
  MooseApp * _app;
  Factory * _factory;

  template <typename T>
  void testParam(unsigned int nparam, const char ** param_list, std::string name);

  void headerErrorHelper(const char * filename, const char * error);
};

#endif // EBSDMESHERRORTEST_H
