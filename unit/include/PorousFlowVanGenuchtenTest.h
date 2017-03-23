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

#ifndef POROUSFLOWVANGENUCHTENTEST_H
#define POROUSFLOWVANGENUCHTENTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

// Moose includes
#include "PorousFlowVanGenuchten.h"

class PorousFlowVanGenuchtenTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(PorousFlowVanGenuchtenTest);

  CPPUNIT_TEST(satTest);
  CPPUNIT_TEST(dsatTest);
  CPPUNIT_TEST(d2satTest);
  CPPUNIT_TEST(capTest);
  CPPUNIT_TEST(dcapTest);
  CPPUNIT_TEST(d2capTest);
  CPPUNIT_TEST(relpermTest);
  CPPUNIT_TEST(drelpermTest);
  CPPUNIT_TEST(d2relpermTest);

  CPPUNIT_TEST_SUITE_END();

public:
  PorousFlowVanGenuchtenTest();

  void satTest();
  void dsatTest();
  void d2satTest();
  void capTest();
  void dcapTest();
  void d2capTest();
  void relpermTest();
  void drelpermTest();
  void d2relpermTest();

private:
  Real _ep;
};

#endif // POROUSFLOWVANGENUCHTENTEST_H
