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
#include "PorousFlowEffectiveSaturationVGTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( PorousFlowEffectiveSaturationVGTest );

PorousFlowEffectiveSaturationVGTest::PorousFlowEffectiveSaturationVGTest() :
    _ep(1.0E-8)
{
}

void
PorousFlowEffectiveSaturationVGTest::satTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, PorousFlowEffectiveSaturationVG::seff(1.0E30, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, PorousFlowEffectiveSaturationVG::seff(1.0, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, PorousFlowEffectiveSaturationVG::seff(0.0, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, PorousFlowEffectiveSaturationVG::seff(1.0E-10, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.486841442435055, PorousFlowEffectiveSaturationVG::seff(-2.0, 0.7, 0.6), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowEffectiveSaturationVG::seff(-1.0E30, 0.7, 0.5), 1.0E-5);
}

void
PorousFlowEffectiveSaturationVGTest::dsatTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowEffectiveSaturationVG::dseff(1.0E30, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowEffectiveSaturationVG::dseff(1.0, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowEffectiveSaturationVG::dseff(0.0, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowEffectiveSaturationVG::dseff(1.0E-10, 0.7, 0.5), 1.0E-5);
  fd = (PorousFlowEffectiveSaturationVG::seff(-2.0 + _ep, 0.7, 0.6) - PorousFlowEffectiveSaturationVG::seff(-2.0, 0.7, 0.6))/_ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowEffectiveSaturationVG::dseff(-2.0, 0.7, 0.6), 1.0E-5);
  fd = (PorousFlowEffectiveSaturationVG::seff(-1.1 + _ep, 0.9, 0.66) - PorousFlowEffectiveSaturationVG::seff(-1.1, 0.9, 0.66))/_ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowEffectiveSaturationVG::dseff(-1.1, 0.9, 0.66), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowEffectiveSaturationVG::dseff(-1.0E30, 0.7, 0.5), 1.0E-5);
}

void
PorousFlowEffectiveSaturationVGTest::d2satTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowEffectiveSaturationVG::d2seff(1.0E30, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowEffectiveSaturationVG::d2seff(1.0, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowEffectiveSaturationVG::d2seff(0.0, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowEffectiveSaturationVG::d2seff(1.0E-10, 0.7, 0.5), 1.0E-5);
  fd = (PorousFlowEffectiveSaturationVG::dseff(-2.0 + _ep, 0.7, 0.6) - PorousFlowEffectiveSaturationVG::dseff(-2.0, 0.7, 0.6))/_ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowEffectiveSaturationVG::d2seff(-2.0, 0.7, 0.6), 1.0E-5);
  fd = (PorousFlowEffectiveSaturationVG::dseff(-1.1 + _ep, 2.3, 0.67) - PorousFlowEffectiveSaturationVG::dseff(-1.1, 2.3, 0.67))/_ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowEffectiveSaturationVG::d2seff(-1.1, 2.3, 0.67), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowEffectiveSaturationVG::d2seff(-1.0E30, 0.7, 0.5), 1.0E-5);
}


