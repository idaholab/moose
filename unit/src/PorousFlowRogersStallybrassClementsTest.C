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
#include "PorousFlowRogersStallybrassClementsTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(PorousFlowRogersStallybrassClementsTest);

PorousFlowRogersStallybrassClementsTest::PorousFlowRogersStallybrassClementsTest() : _ep(1.0E-8) {}

void
PorousFlowRogersStallybrassClementsTest::satTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowRogersStallybrassClements::effectiveSaturation(50, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      std::pow(2.0, -0.5),
      PorousFlowRogersStallybrassClements::effectiveSaturation(1.1, 1.1, 4.4),
      1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      std::pow(1.0 + std::exp(1.0), -0.5),
      PorousFlowRogersStallybrassClements::effectiveSaturation(5.5, 1.1, 4.4),
      1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      1.0, PorousFlowRogersStallybrassClements::effectiveSaturation(-50, 0.7, 0.5), 1.0E-5);
}

void
PorousFlowRogersStallybrassClementsTest::dsatTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowRogersStallybrassClements::dEffectiveSaturation(50, 0.7, 0.5), 1.0E-5);
  fd = (PorousFlowRogersStallybrassClements::effectiveSaturation(1.1 + _ep, 1.1, 4.4) -
        PorousFlowRogersStallybrassClements::effectiveSaturation(1.1, 1.1, 4.4)) /
       _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      fd, PorousFlowRogersStallybrassClements::dEffectiveSaturation(1.1, 1.1, 4.4), 1.0E-5);
  fd = (PorousFlowRogersStallybrassClements::effectiveSaturation(5.5 + _ep, 1.1, 4.4) -
        PorousFlowRogersStallybrassClements::effectiveSaturation(5.5, 1.1, 4.4)) /
       _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      fd, PorousFlowRogersStallybrassClements::dEffectiveSaturation(5.5, 1.1, 4.4), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowRogersStallybrassClements::dEffectiveSaturation(-50, 0.7, 0.5), 1.0E-5);
}

void
PorousFlowRogersStallybrassClementsTest::d2satTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowRogersStallybrassClements::d2EffectiveSaturation(50, 0.7, 0.5), 1.0E-5);
  fd = (PorousFlowRogersStallybrassClements::dEffectiveSaturation(1.1 + _ep, 1.1, 4.4) -
        PorousFlowRogersStallybrassClements::dEffectiveSaturation(1.1, 1.1, 4.4)) /
       _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      fd, PorousFlowRogersStallybrassClements::d2EffectiveSaturation(1.1, 1.1, 4.4), 1.0E-5);
  fd = (PorousFlowRogersStallybrassClements::dEffectiveSaturation(5.5 + _ep, 1.1, 4.4) -
        PorousFlowRogersStallybrassClements::dEffectiveSaturation(5.5, 1.1, 4.4)) /
       _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      fd, PorousFlowRogersStallybrassClements::d2EffectiveSaturation(5.5, 1.1, 4.4), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowRogersStallybrassClements::d2EffectiveSaturation(-50, 0.7, 0.5), 1.0E-5);
}
