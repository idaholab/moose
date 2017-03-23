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

#ifndef IDEALGASFLUIDPROPERTIESTEST_H
#define IDEALGASFLUIDPROPERTIESTEST_H

// CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

class MooseMesh;
class FEProblem;
class IdealGasFluidProperties;

class IdealGasFluidPropertiesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(IdealGasFluidPropertiesTest);

  CPPUNIT_TEST(testAll);

  CPPUNIT_TEST_SUITE_END();

public:
  void registerObjects(Factory & factory);
  void buildObjects();

  void setUp();
  void tearDown();
  // test
  void testAll();

protected:
  MooseApp * _app;
  Factory * _factory;
  MooseMesh * _mesh;
  FEProblem * _fe_problem;
  const IdealGasFluidProperties * _fp;
};

#endif /* IDEALGASFLUIDPROPERTIESTEST_H */
