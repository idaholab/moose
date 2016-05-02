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
#include "PorousFlowEffectiveSaturationRSCTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( PorousFlowEffectiveSaturationRSCTest );

PorousFlowEffectiveSaturationRSCTest::PorousFlowEffectiveSaturationRSCTest() :
    _ep(1.0E-8)
{
}

void
PorousFlowEffectiveSaturationRSCTest::satTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowEffectiveSaturationRSC::seff(50, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(std::pow(2.0, -0.5), PorousFlowEffectiveSaturationRSC::seff(1.1, 1.1, 4.4), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(std::pow(1.0 + std::exp(1.0), -0.5), PorousFlowEffectiveSaturationRSC::seff(5.5, 1.1, 4.4), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, PorousFlowEffectiveSaturationRSC::seff(-50, 0.7, 0.5), 1.0E-5);
}

void
PorousFlowEffectiveSaturationRSCTest::dsatTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowEffectiveSaturationRSC::dseff(50, 0.7, 0.5), 1.0E-5);
  fd = (PorousFlowEffectiveSaturationRSC::seff(1.1 + _ep, 1.1, 4.4) - PorousFlowEffectiveSaturationRSC::seff(1.1, 1.1, 4.4))/_ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowEffectiveSaturationRSC::dseff(1.1, 1.1, 4.4), 1.0E-5);
  fd = (PorousFlowEffectiveSaturationRSC::seff(5.5 + _ep, 1.1, 4.4) - PorousFlowEffectiveSaturationRSC::seff(5.5, 1.1, 4.4))/_ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowEffectiveSaturationRSC::dseff(5.5, 1.1, 4.4), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowEffectiveSaturationRSC::dseff(-50, 0.7, 0.5), 1.0E-5);
}

void
PorousFlowEffectiveSaturationRSCTest::d2satTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowEffectiveSaturationRSC::d2seff(50, 0.7, 0.5), 1.0E-5);
  fd = (PorousFlowEffectiveSaturationRSC::dseff(1.1 + _ep, 1.1, 4.4) - PorousFlowEffectiveSaturationRSC::dseff(1.1, 1.1, 4.4))/_ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowEffectiveSaturationRSC::d2seff(1.1, 1.1, 4.4), 1.0E-5);
  fd = (PorousFlowEffectiveSaturationRSC::dseff(5.5 + _ep, 1.1, 4.4) - PorousFlowEffectiveSaturationRSC::dseff(5.5, 1.1, 4.4))/_ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowEffectiveSaturationRSC::d2seff(5.5, 1.1, 4.4), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowEffectiveSaturationRSC::d2seff(-50, 0.7, 0.5), 1.0E-5);
}


