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
#include "PorousFlowVanGenuchtenTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(PorousFlowVanGenuchtenTest);

PorousFlowVanGenuchtenTest::PorousFlowVanGenuchtenTest() : _ep(1.0E-8) {}

void
PorousFlowVanGenuchtenTest::satTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      1.0, PorousFlowVanGenuchten::effectiveSaturation(1.0E30, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      1.0, PorousFlowVanGenuchten::effectiveSaturation(1.0, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      1.0, PorousFlowVanGenuchten::effectiveSaturation(0.0, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      1.0, PorousFlowVanGenuchten::effectiveSaturation(1.0E-10, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.486841442435055, PorousFlowVanGenuchten::effectiveSaturation(-2.0, 0.7, 0.6), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::effectiveSaturation(-1.0E30, 0.7, 0.5), 1.0E-5);
}

void
PorousFlowVanGenuchtenTest::dsatTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::dEffectiveSaturation(1.0E30, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::dEffectiveSaturation(1.0, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::dEffectiveSaturation(0.0, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::dEffectiveSaturation(1.0E-10, 0.7, 0.5), 1.0E-5);
  fd = (PorousFlowVanGenuchten::effectiveSaturation(-2.0 + _ep, 0.7, 0.6) -
        PorousFlowVanGenuchten::effectiveSaturation(-2.0, 0.7, 0.6)) /
       _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      fd, PorousFlowVanGenuchten::dEffectiveSaturation(-2.0, 0.7, 0.6), 1.0E-5);
  fd = (PorousFlowVanGenuchten::effectiveSaturation(-1.1 + _ep, 0.9, 0.66) -
        PorousFlowVanGenuchten::effectiveSaturation(-1.1, 0.9, 0.66)) /
       _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      fd, PorousFlowVanGenuchten::dEffectiveSaturation(-1.1, 0.9, 0.66), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::dEffectiveSaturation(-1.0E30, 0.7, 0.5), 1.0E-5);
}

void
PorousFlowVanGenuchtenTest::d2satTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::d2EffectiveSaturation(1.0E30, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::d2EffectiveSaturation(1.0, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::d2EffectiveSaturation(0.0, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::d2EffectiveSaturation(1.0E-10, 0.7, 0.5), 1.0E-5);
  fd = (PorousFlowVanGenuchten::dEffectiveSaturation(-2.0 + _ep, 0.7, 0.6) -
        PorousFlowVanGenuchten::dEffectiveSaturation(-2.0, 0.7, 0.6)) /
       _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      fd, PorousFlowVanGenuchten::d2EffectiveSaturation(-2.0, 0.7, 0.6), 1.0E-5);
  fd = (PorousFlowVanGenuchten::dEffectiveSaturation(-1.1 + _ep, 2.3, 0.67) -
        PorousFlowVanGenuchten::dEffectiveSaturation(-1.1, 2.3, 0.67)) /
       _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      fd, PorousFlowVanGenuchten::d2EffectiveSaturation(-1.1, 2.3, 0.67), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::d2EffectiveSaturation(-1.0E30, 0.7, 0.5), 1.0E-5);
}

void
PorousFlowVanGenuchtenTest::capTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::capillaryPressure(1.1, 0.55, 1.0, -1.0E30), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-4.06172392297447,
                               PorousFlowVanGenuchten::capillaryPressure(0.3, 0.55, 1.6, -1.0E30),
                               1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      -2.9, PorousFlowVanGenuchten::capillaryPressure(0.001, 0.55, 1.6, -2.9), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      -1000.0, PorousFlowVanGenuchten::capillaryPressure(0.0, 0.55, 1.6, -1000.0), 1.0E-5);
}

void
PorousFlowVanGenuchtenTest::dcapTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::dCapillaryPressure(0.0, 0.55, 1.0, -1.0E30), 1.0E-5);
  fd = (PorousFlowVanGenuchten::capillaryPressure(0.3 + _ep, 0.55, 1.6, -1.0E30) -
        PorousFlowVanGenuchten::capillaryPressure(0.3, 0.55, 1.6, -1.0E30)) /
       _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      fd, PorousFlowVanGenuchten::dCapillaryPressure(0.3, 0.55, 1.6, -1.0E30), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::dCapillaryPressure(1.0, 0.55, 1.0, -1.0E30), 1.0E-5);
}

void
PorousFlowVanGenuchtenTest::d2capTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::d2CapillaryPressure(0.0, 0.55, 1.0, -1.0E30), 1.0E-5);
  fd = (PorousFlowVanGenuchten::dCapillaryPressure(0.3 + _ep, 0.55, 1.6, -1.0E30) -
        PorousFlowVanGenuchten::dCapillaryPressure(0.3, 0.55, 1.6, -1.0E30)) /
       _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      fd, PorousFlowVanGenuchten::d2CapillaryPressure(0.3, 0.55, 1.6, -1.0E30), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::d2CapillaryPressure(1.0, 0.55, 1.6, -2.9), 1.0E-5);
}

void
PorousFlowVanGenuchtenTest::relpermTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      1.0, PorousFlowVanGenuchten::relativePermeability(1.0E30, 0.7), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::relativePermeability(-1.0, 0.7), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0091160727, PorousFlowVanGenuchten::relativePermeability(0.3, 0.7), 1.0E-5);
}

void
PorousFlowVanGenuchtenTest::drelpermTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::dRelativePermeability(1.0E30, 0.7), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::dRelativePermeability(-1.0, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::relativePermeability(0.3 + _ep, 0.7) -
        PorousFlowVanGenuchten::relativePermeability(0.3, 0.7)) /
       _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowVanGenuchten::dRelativePermeability(0.3, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::relativePermeability(0.8 + _ep, 0.65) -
        PorousFlowVanGenuchten::relativePermeability(0.8, 0.65)) /
       _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      fd, PorousFlowVanGenuchten::dRelativePermeability(0.8, 0.65), 1.0E-5);
}

void
PorousFlowVanGenuchtenTest::d2relpermTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::d2RelativePermeability(1.0E30, 0.7), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowVanGenuchten::d2RelativePermeability(-1.0, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::dRelativePermeability(0.3 + _ep, 0.7) -
        PorousFlowVanGenuchten::dRelativePermeability(0.3, 0.7)) /
       _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      fd, PorousFlowVanGenuchten::d2RelativePermeability(0.3, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::dRelativePermeability(0.8 + _ep, 0.65) -
        PorousFlowVanGenuchten::dRelativePermeability(0.8, 0.65)) /
       _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      fd, PorousFlowVanGenuchten::d2RelativePermeability(0.8, 0.65), 1.0E-5);
}
