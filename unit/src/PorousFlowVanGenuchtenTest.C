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

CPPUNIT_TEST_SUITE_REGISTRATION( PorousFlowVanGenuchtenTest );

PorousFlowVanGenuchtenTest::PorousFlowVanGenuchtenTest() :
    _ep(1.0E-8)
{
}

void
PorousFlowVanGenuchtenTest::satTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, PorousFlowVanGenuchten::seff(1.0E30, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, PorousFlowVanGenuchten::seff(1.0, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, PorousFlowVanGenuchten::seff(0.0, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, PorousFlowVanGenuchten::seff(1.0E-10, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.486841442435055, PorousFlowVanGenuchten::seff(-2.0, 0.7, 0.6), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowVanGenuchten::seff(-1.0E30, 0.7, 0.5), 1.0E-5);
}

void
PorousFlowVanGenuchtenTest::dsatTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowVanGenuchten::dseff(1.0E30, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowVanGenuchten::dseff(1.0, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowVanGenuchten::dseff(0.0, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowVanGenuchten::dseff(1.0E-10, 0.7, 0.5), 1.0E-5);
  fd = (PorousFlowVanGenuchten::seff(-2.0 + _ep, 0.7, 0.6) - PorousFlowVanGenuchten::seff(-2.0, 0.7, 0.6))/_ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowVanGenuchten::dseff(-2.0, 0.7, 0.6), 1.0E-5);
  fd = (PorousFlowVanGenuchten::seff(-1.1 + _ep, 0.9, 0.66) - PorousFlowVanGenuchten::seff(-1.1, 0.9, 0.66))/_ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowVanGenuchten::dseff(-1.1, 0.9, 0.66), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowVanGenuchten::dseff(-1.0E30, 0.7, 0.5), 1.0E-5);
}

void
PorousFlowVanGenuchtenTest::d2satTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowVanGenuchten::d2seff(1.0E30, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowVanGenuchten::d2seff(1.0, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowVanGenuchten::d2seff(0.0, 0.7, 0.5), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowVanGenuchten::d2seff(1.0E-10, 0.7, 0.5), 1.0E-5);
  fd = (PorousFlowVanGenuchten::dseff(-2.0 + _ep, 0.7, 0.6) - PorousFlowVanGenuchten::dseff(-2.0, 0.7, 0.6))/_ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowVanGenuchten::d2seff(-2.0, 0.7, 0.6), 1.0E-5);
  fd = (PorousFlowVanGenuchten::dseff(-1.1 + _ep, 2.3, 0.67) - PorousFlowVanGenuchten::dseff(-1.1, 2.3, 0.67))/_ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowVanGenuchten::d2seff(-1.1, 2.3, 0.67), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowVanGenuchten::d2seff(-1.0E30, 0.7, 0.5), 1.0E-5);
}

void
PorousFlowVanGenuchtenTest::relpermTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, PorousFlowVanGenuchten::relativePermeability(1.0E30, 0.7), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowVanGenuchten::relativePermeability(-1.0, 0.7), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0091160727, PorousFlowVanGenuchten::relativePermeability(0.3, 0.7), 1.0E-5);
}


void
PorousFlowVanGenuchtenTest::drelpermTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowVanGenuchten::drelativePermeability(1.0E30, 0.7), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowVanGenuchten::drelativePermeability(-1.0, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::relativePermeability(0.3 + _ep, 0.7) - PorousFlowVanGenuchten::relativePermeability(0.3, 0.7)) / _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowVanGenuchten::drelativePermeability(0.3, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::relativePermeability(0.8 + _ep, 0.65) - PorousFlowVanGenuchten::relativePermeability(0.8, 0.65)) / _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowVanGenuchten::drelativePermeability(0.8, 0.65), 1.0E-5);
}


void
PorousFlowVanGenuchtenTest::d2relpermTest()
{
  Real fd;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowVanGenuchten::d2relativePermeability(1.0E30, 0.7), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, PorousFlowVanGenuchten::d2relativePermeability(-1.0, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::drelativePermeability(0.3 + _ep, 0.7) - PorousFlowVanGenuchten::drelativePermeability(0.3, 0.7)) / _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowVanGenuchten::d2relativePermeability(0.3, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::drelativePermeability(0.8 + _ep, 0.65) - PorousFlowVanGenuchten::drelativePermeability(0.8, 0.65)) / _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(fd, PorousFlowVanGenuchten::d2relativePermeability(0.8, 0.65), 1.0E-5);
}

