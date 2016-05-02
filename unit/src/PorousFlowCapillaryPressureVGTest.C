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
#include "PorousFlowCapillaryPressureVGTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( PorousFlowCapillaryPressureVGTest );

PorousFlowCapillaryPressureVGTest::PorousFlowCapillaryPressureVGTest() :
    _ep(1.0E-8)
{
}

void
PorousFlowCapillaryPressureVGTest::effTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.4, PorousFlowCapillaryPressureVG::effectiveSaturation(0.3, 0.1, 0.6), 1.0E-5);
}

void
PorousFlowCapillaryPressureVGTest::capTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowCapillaryPressureVG::capillaryPressure(0.7, 0.55, 0.1, 0.6, 1.0, 1.0E30), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowCapillaryPressureVG::capillaryPressure(0.6, 0.55, 0.1, 0.6, 1.0, 1.0E30), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(3.08152363443454, PorousFlowCapillaryPressureVG::capillaryPressure(0.3, 0.55, 0.1, 0.6, 1.6, 1.0E30), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(2.9, PorousFlowCapillaryPressureVG::capillaryPressure(0.3, 0.55, 0.1, 0.6, 1.6, 2.9), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1000.0, PorousFlowCapillaryPressureVG::capillaryPressure(0.1001, 0.55, 0.1, 0.6, 1.6, 1000.0), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1000.0, PorousFlowCapillaryPressureVG::capillaryPressure(0.1, 0.55, 0.1, 0.6, 1.6, 1000.0), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1000.0, PorousFlowCapillaryPressureVG::capillaryPressure(0.0, 0.55, 0.1, 0.6, 1.6, 1000.0), 1.0E-5);
}

void
PorousFlowCapillaryPressureVGTest::dcapTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowCapillaryPressureVG::dCapillaryPressure(0.7, 0.55, 0.1, 0.6, 1.0, 1.0E30), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowCapillaryPressureVG::dCapillaryPressure(0.6, 0.55, 0.1, 0.6, 1.0, 1.0E30), 1.0E-5);
  fd = (PorousFlowCapillaryPressureVG::capillaryPressure(0.3 + _ep, 0.55, 0.1, 0.6, 1.6, 1.0E30) - PorousFlowCapillaryPressureVG::capillaryPressure(0.3, 0.55, 0.1, 0.6, 1.6, 1.0E30))/_ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowCapillaryPressureVG::dCapillaryPressure(0.3, 0.55, 0.1, 0.6, 1.6, 1.0E30), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowCapillaryPressureVG::dCapillaryPressure(0.3, 0.55, 0.1, 0.6, 1.6, 2.9), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowCapillaryPressureVG::dCapillaryPressure(0.1001, 0.55, 0.1, 0.6, 1.6, 1000.0), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowCapillaryPressureVG::dCapillaryPressure(0.1, 0.55, 0.1, 0.6, 1.6, 1000.0), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowCapillaryPressureVG::dCapillaryPressure(0.0, 0.55, 0.1, 0.6, 1.6, 1000.0), 1.0E-5);
}

void
PorousFlowCapillaryPressureVGTest::d2capTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowCapillaryPressureVG::d2CapillaryPressure(0.7, 0.55, 0.1, 0.6, 1.0, 1.0E30), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowCapillaryPressureVG::d2CapillaryPressure(0.6, 0.55, 0.1, 0.6, 1.0, 1.0E30), 1.0E-5);
  fd = (PorousFlowCapillaryPressureVG::dCapillaryPressure(0.3 + _ep, 0.55, 0.1, 0.6, 1.6, 1.0E30) - PorousFlowCapillaryPressureVG::dCapillaryPressure(0.3, 0.55, 0.1, 0.6, 1.6, 1.0E30))/_ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowCapillaryPressureVG::d2CapillaryPressure(0.3, 0.55, 0.1, 0.6, 1.6, 1.0E30), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowCapillaryPressureVG::d2CapillaryPressure(0.3, 0.55, 0.1, 0.6, 1.6, 2.9), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowCapillaryPressureVG::d2CapillaryPressure(0.1001, 0.55, 0.1, 0.6, 1.6, 1000.0), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowCapillaryPressureVG::d2CapillaryPressure(0.1, 0.55, 0.1, 0.6, 1.6, 1000.0), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowCapillaryPressureVG::d2CapillaryPressure(0.0, 0.55, 0.1, 0.6, 1.6, 1000.0), 1.0E-5);
}


