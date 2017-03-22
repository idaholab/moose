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
#include "PorousFlowBroadbridgeWhiteTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(PorousFlowBroadbridgeWhiteTest);

PorousFlowBroadbridgeWhiteTest::PorousFlowBroadbridgeWhiteTest()
  : _ep(1.0E-8), _c(1.5), _sn(0.1), _ss(0.95), _las(2.0), _kn(0.05), _ks(0.8)
{
}

void
PorousFlowBroadbridgeWhiteTest::satTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      1.0, PorousFlowBroadbridgeWhite::effectiveSaturation(50, _c, _sn, _ss, _las), 1.0E-5);
  const Real eff = PorousFlowBroadbridgeWhite::effectiveSaturation(-1.0, _c, _sn, _ss, _las);
  const Real th = (eff - _sn) / (_ss - _sn);
  const Real t1 = (1.0 / _c) * std::log((_c - th) / (_c - 1.0) / th);
  const Real t2 = (th - 1.0) / th;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, _las * (t2 - t1), 1.0E-5);
}

void
PorousFlowBroadbridgeWhiteTest::dsatTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowBroadbridgeWhite::dEffectiveSaturation(50, 0.7, 0.5, 0.6, 1.0), 1.0E-5);
  const Real fd = (PorousFlowBroadbridgeWhite::effectiveSaturation(-1.0 + _ep, _c, _sn, _ss, _las) -
                   PorousFlowBroadbridgeWhite::effectiveSaturation(-1.0, _c, _sn, _ss, _las)) /
                  _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      fd, PorousFlowBroadbridgeWhite::dEffectiveSaturation(-1.0, _c, _sn, _ss, _las), 1.0E-5);
}

void
PorousFlowBroadbridgeWhiteTest::d2satTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowBroadbridgeWhite::d2EffectiveSaturation(50, 0.7, 0.5, 0.6, 1.0), 1.0E-5);
  const Real fd =
      (PorousFlowBroadbridgeWhite::dEffectiveSaturation(-1.0 + _ep, _c, _sn, _ss, _las) -
       PorousFlowBroadbridgeWhite::dEffectiveSaturation(-1.0, _c, _sn, _ss, _las)) /
      _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      fd, PorousFlowBroadbridgeWhite::d2EffectiveSaturation(-1.0, _c, _sn, _ss, _las), 1.0E-5);
}

void
PorousFlowBroadbridgeWhiteTest::relpermTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      _kn, PorousFlowBroadbridgeWhite::relativePermeability(0.01, _c, _sn, _ss, _kn, _ks), 1.0E-5);
  const Real sat = 0.5;
  const Real th = (sat - _sn) / (_ss - _sn);
  const Real expect = _kn + (_ks - _kn) * th * th * (_c - 1.0) / (_c - th);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      expect,
      PorousFlowBroadbridgeWhite::relativePermeability(sat, _c, _sn, _ss, _kn, _ks),
      1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      _ks, PorousFlowBroadbridgeWhite::relativePermeability(0.99, _c, _sn, _ss, _kn, _ks), 1.0E-5);
}

void
PorousFlowBroadbridgeWhiteTest::drelpermTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowBroadbridgeWhite::dRelativePermeability(0.01, _c, _sn, _ss, _kn, _ks), 1.0E-5);
  const Real sat = 0.4;
  const Real fd =
      (PorousFlowBroadbridgeWhite::relativePermeability(sat + _ep, _c, _sn, _ss, _kn, _ks) -
       PorousFlowBroadbridgeWhite::relativePermeability(sat, _c, _sn, _ss, _kn, _ks)) /
      _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      fd, PorousFlowBroadbridgeWhite::dRelativePermeability(sat, _c, _sn, _ss, _kn, _ks), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0, PorousFlowBroadbridgeWhite::dRelativePermeability(0.99, _c, _sn, _ss, _kn, _ks), 1.0E-5);
}

void
PorousFlowBroadbridgeWhiteTest::d2relpermTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0,
      PorousFlowBroadbridgeWhite::d2RelativePermeability(0.01, _c, _sn, _ss, _kn, _ks),
      1.0E-5);
  const Real sat = 0.6;
  const Real fd =
      (PorousFlowBroadbridgeWhite::dRelativePermeability(sat + _ep, _c, _sn, _ss, _kn, _ks) -
       PorousFlowBroadbridgeWhite::dRelativePermeability(sat, _c, _sn, _ss, _kn, _ks)) /
      _ep;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      fd, PorousFlowBroadbridgeWhite::d2RelativePermeability(sat, _c, _sn, _ss, _kn, _ks), 1.0E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      0.0,
      PorousFlowBroadbridgeWhite::d2RelativePermeability(0.99, _c, _sn, _ss, _kn, _ks),
      1.0E-5);
}
