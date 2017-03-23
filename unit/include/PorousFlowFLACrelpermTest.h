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

#ifndef POROUSFLOWFLACRELPERMTEST_H
#define POROUSFLOWFLACRELPERMTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

// Moose includes
#include "PorousFlowFLACrelperm.h"

class PorousFlowFLACrelpermTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(PorousFlowFLACrelpermTest);

  CPPUNIT_TEST(relpermTest);
  CPPUNIT_TEST(drelpermTest);
  CPPUNIT_TEST(d2relpermTest);

  CPPUNIT_TEST_SUITE_END();

public:
  PorousFlowFLACrelpermTest();

  void relpermTest();
  void drelpermTest();
  void d2relpermTest();

private:
  Real _ep;
};

#endif // POROUSFLOWFLACRELPERMTEST_H
