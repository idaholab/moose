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
#include "PorousFlowFLACrelpermTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(PorousFlowFLACrelpermTest);

PorousFlowFLACrelpermTest::PorousFlowFLACrelpermTest() : _ep(1.0E-8) {}

void
PorousFlowFLACrelpermTest::relpermTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      1.0, PorousFlowFLACrelperm::relativePermeability(1.0E30, 2.7), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowFLACrelperm::relativePermeability(-1.0, 2.7), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.111976072427008, PorousFlowFLACrelperm::relativePermeability(0.3, 2.7), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.208087549965399, PorousFlowFLACrelperm::relativePermeability(0.8, 12.7), 1.0E-5);
}

void
PorousFlowFLACrelpermTest::drelpermTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowFLACrelperm::dRelativePermeability(1.0E30, 2.7), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowFLACrelperm::dRelativePermeability(-1.0, 2.7), 1.0E-5);
  fd = (PorousFlowFLACrelperm::relativePermeability(0.3 + _ep, 2.7) -
        PorousFlowFLACrelperm::relativePermeability(0.3, 2.7)) /
       _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowFLACrelperm::dRelativePermeability(0.3, 2.7), 1.0E-5);
  fd = (PorousFlowFLACrelperm::relativePermeability(0.8 + _ep, 0.65) -
        PorousFlowFLACrelperm::relativePermeability(0.8, 0.65)) /
       _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowFLACrelperm::dRelativePermeability(0.8, 0.65), 1.0E-5);
}

void
PorousFlowFLACrelpermTest::d2relpermTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowFLACrelperm::d2RelativePermeability(1.0E30, 2.7), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowFLACrelperm::d2RelativePermeability(-1.0, 2.7), 1.0E-5);
  fd = (PorousFlowFLACrelperm::dRelativePermeability(0.3 + _ep, 2.7) -
        PorousFlowFLACrelperm::dRelativePermeability(0.3, 2.7)) /
       _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowFLACrelperm::d2RelativePermeability(0.3, 2.7), 1.0E-5);
  fd = (PorousFlowFLACrelperm::dRelativePermeability(0.8 + _ep, 0.65) -
        PorousFlowFLACrelperm::dRelativePermeability(0.8, 0.65)) /
       _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      fd, PorousFlowFLACrelperm::d2RelativePermeability(0.8, 0.65), 1.0E-5);
}
