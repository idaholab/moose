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

#ifndef METHANEFLUIDPROPERTIESTEST_H
#define METHANEFLUIDPROPERTIESTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

class MooseMesh;
class FEProblem;
class MethaneFluidProperties;

class MethaneFluidPropertiesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MethaneFluidPropertiesTest);

  /**
   * Verify calculation of Henry's constant using data from
   * Guidelines on the Henry's constant and vapour liquid distribution constant
   * for gases in H20 and D20 at high temperatures, IAPWS (2004).
   */
  CPPUNIT_TEST(henry);

  /**
   * Verify calculation of thermophysical properties of methane using
   * verification data provided in
   * Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas Tables with
   * Computer Equations
   */
  CPPUNIT_TEST(properties);

  /**
   * Verify calculation of the derivatives of all properties by comparing with finite
   * differences
   */
  CPPUNIT_TEST(derivatives);

  CPPUNIT_TEST_SUITE_END();

public:
  void registerObjects(Factory & factory);
  void buildObjects();

  void setUp();
  void tearDown();

  void henry();
  void properties();
  void derivatives();

private:
  MooseApp * _app;
  Factory * _factory;
  MooseMesh * _mesh;
  FEProblem * _fe_problem;
  const MethaneFluidProperties * _fp;
};

#endif // METHANEFLUIDPROPERTIESTEST_H
