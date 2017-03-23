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

#ifndef SIMPLEFLUIDPROPERTIESTEST_H
#define SIMPLEFLUIDPROPERTIESTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

class MooseMesh;
class FEProblem;
class SimpleFluidProperties;

class SimpleFluidPropertiesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SimpleFluidPropertiesTest);

  /**
   * Verify calculation of the fluid properties
   */
  CPPUNIT_TEST(properties);

  /**
   * Verify calculation of the derivatives by comparing with finite
   * differences
   */
  CPPUNIT_TEST(derivatives);

  CPPUNIT_TEST_SUITE_END();

public:
  void registerObjects(Factory & factory);
  void buildObjects();

  void setUp();
  void tearDown();
  void properties();
  void derivatives();

private:
  MooseApp * _app;
  Factory * _factory;
  MooseMesh * _mesh;
  FEProblem * _fe_problem;
  const SimpleFluidProperties * _fp;
};

#endif // SIMPLEFLUIDPROPERTIESTEST_H
