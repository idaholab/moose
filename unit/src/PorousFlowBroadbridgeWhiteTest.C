//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "PorousFlowBroadbridgeWhite.h"

class PorousFlowBroadbridgeWhiteTest : public ::testing::Test
{
protected:
  Real _ep = 1e-8;
  Real _c = 1.5;
  Real _sn = .1;
  Real _ss = .95;
  Real _las = 2.0;
  Real _kn = 0.05;
  Real _ks = .8;
};

TEST_F(PorousFlowBroadbridgeWhiteTest, sat)
{
  EXPECT_NEAR(1.0, PorousFlowBroadbridgeWhite::effectiveSaturation(50, _c, _sn, _ss, _las), 1.0E-5);
  const Real eff = PorousFlowBroadbridgeWhite::effectiveSaturation(-1.0, _c, _sn, _ss, _las);
  const Real th = (eff - _sn) / (_ss - _sn);
  const Real t1 = (1.0 / _c) * std::log((_c - th) / (_c - 1.0) / th);
  const Real t2 = (th - 1.0) / th;
  EXPECT_NEAR(-1.0, _las * (t2 - t1), 1.0E-5);
}

TEST_F(PorousFlowBroadbridgeWhiteTest, dsat)
{
  EXPECT_NEAR(
      0.0, PorousFlowBroadbridgeWhite::dEffectiveSaturation(50, 0.7, 0.5, 0.6, 1.0), 1.0E-5);
  const Real fd = (PorousFlowBroadbridgeWhite::effectiveSaturation(-1.0 + _ep, _c, _sn, _ss, _las) -
                   PorousFlowBroadbridgeWhite::effectiveSaturation(-1.0, _c, _sn, _ss, _las)) /
                  _ep;
  EXPECT_NEAR(
      fd, PorousFlowBroadbridgeWhite::dEffectiveSaturation(-1.0, _c, _sn, _ss, _las), 1.0E-5);
}

TEST_F(PorousFlowBroadbridgeWhiteTest, d2sat)
{
  EXPECT_NEAR(
      0.0, PorousFlowBroadbridgeWhite::d2EffectiveSaturation(50, 0.7, 0.5, 0.6, 1.0), 1.0E-5);
  const Real fd =
      (PorousFlowBroadbridgeWhite::dEffectiveSaturation(-1.0 + _ep, _c, _sn, _ss, _las) -
       PorousFlowBroadbridgeWhite::dEffectiveSaturation(-1.0, _c, _sn, _ss, _las)) /
      _ep;
  EXPECT_NEAR(
      fd, PorousFlowBroadbridgeWhite::d2EffectiveSaturation(-1.0, _c, _sn, _ss, _las), 1.0E-5);
}

TEST_F(PorousFlowBroadbridgeWhiteTest, relperm)
{
  EXPECT_NEAR(
      _kn, PorousFlowBroadbridgeWhite::relativePermeability(0.01, _c, _sn, _ss, _kn, _ks), 1.0E-5);
  const Real sat = 0.5;
  const Real th = (sat - _sn) / (_ss - _sn);
  const Real expect = _kn + (_ks - _kn) * th * th * (_c - 1.0) / (_c - th);
  EXPECT_NEAR(expect,
              PorousFlowBroadbridgeWhite::relativePermeability(sat, _c, _sn, _ss, _kn, _ks),
              1.0E-5);
  EXPECT_NEAR(
      _ks, PorousFlowBroadbridgeWhite::relativePermeability(0.99, _c, _sn, _ss, _kn, _ks), 1.0E-5);
}

TEST_F(PorousFlowBroadbridgeWhiteTest, drelperm)
{
  EXPECT_NEAR(
      0.0, PorousFlowBroadbridgeWhite::dRelativePermeability(0.01, _c, _sn, _ss, _kn, _ks), 1.0E-5);
  const Real sat = 0.4;
  const Real fd =
      (PorousFlowBroadbridgeWhite::relativePermeability(sat + _ep, _c, _sn, _ss, _kn, _ks) -
       PorousFlowBroadbridgeWhite::relativePermeability(sat, _c, _sn, _ss, _kn, _ks)) /
      _ep;
  EXPECT_NEAR(
      fd, PorousFlowBroadbridgeWhite::dRelativePermeability(sat, _c, _sn, _ss, _kn, _ks), 1.0E-5);
  EXPECT_NEAR(
      0.0, PorousFlowBroadbridgeWhite::dRelativePermeability(0.99, _c, _sn, _ss, _kn, _ks), 1.0E-5);
}

TEST_F(PorousFlowBroadbridgeWhiteTest, d2relperm)
{
  EXPECT_NEAR(0.0,
              PorousFlowBroadbridgeWhite::d2RelativePermeability(0.01, _c, _sn, _ss, _kn, _ks),
              1.0E-5);
  const Real sat = 0.6;
  const Real fd =
      (PorousFlowBroadbridgeWhite::dRelativePermeability(sat + _ep, _c, _sn, _ss, _kn, _ks) -
       PorousFlowBroadbridgeWhite::dRelativePermeability(sat, _c, _sn, _ss, _kn, _ks)) /
      _ep;
  EXPECT_NEAR(
      fd, PorousFlowBroadbridgeWhite::d2RelativePermeability(sat, _c, _sn, _ss, _kn, _ks), 1.0E-5);
  EXPECT_NEAR(0.0,
              PorousFlowBroadbridgeWhite::d2RelativePermeability(0.99, _c, _sn, _ss, _kn, _ks),
              1.0E-5);
}
