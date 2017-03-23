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

#ifndef POROUSFLOWBROADBRIDGEWHITETEST_H
#define POROUSFLOWBROADBRIDGEWHITETEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

// Moose includes
#include "PorousFlowBroadbridgeWhite.h"

class PorousFlowBroadbridgeWhiteTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(PorousFlowBroadbridgeWhiteTest);

  CPPUNIT_TEST(satTest);
  CPPUNIT_TEST(dsatTest);
  CPPUNIT_TEST(d2satTest);
  CPPUNIT_TEST(relpermTest);
  CPPUNIT_TEST(drelpermTest);
  CPPUNIT_TEST(d2relpermTest);

  CPPUNIT_TEST_SUITE_END();

public:
  PorousFlowBroadbridgeWhiteTest();

  void satTest();
  void dsatTest();
  void d2satTest();
  void relpermTest();
  void drelpermTest();
  void d2relpermTest();

private:
  Real _ep;
  Real _c;
  Real _sn;
  Real _ss;
  Real _las;
  Real _kn;
  Real _ks;
};

#endif // POROUSFLOWBROADBRIDGEWHITETEST_H
