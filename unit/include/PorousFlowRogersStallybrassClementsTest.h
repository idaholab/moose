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

#ifndef POROUSFLOWROGERSSTALLYBRASSCLEMENTSTEST_H
#define POROUSFLOWROGERSSTALLYBRASSCLEMENTSTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

// Moose includes
#include "PorousFlowRogersStallybrassClements.h"

class PorousFlowRogersStallybrassClementsTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(PorousFlowRogersStallybrassClementsTest);

  CPPUNIT_TEST(satTest);
  CPPUNIT_TEST(dsatTest);
  CPPUNIT_TEST(d2satTest);

  CPPUNIT_TEST_SUITE_END();

public:
  PorousFlowRogersStallybrassClementsTest();

  void satTest();
  void dsatTest();
  void d2satTest();

private:
  Real _ep;
};

#endif // POROUSFLOWROGERSSTALLYBRASSCLEMENTSTEST_H
