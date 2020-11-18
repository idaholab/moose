//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "PorousFlowVanGenuchten.h"

const double eps = 1.0E-8;

const PorousFlowVanGenuchten::LowCapillaryPressureExtension
    no_low_ext(PorousFlowVanGenuchten::LowCapillaryPressureExtension::NONE, -1.0, 123.0, 0.0);
const Real low_ext_Pc = 1.5;
const Real low_ext_S = PorousFlowVanGenuchten::saturationHys(low_ext_Pc, 0.1, 0.3, 10.0, 1.9);
const Real low_ext_dPc =
    PorousFlowVanGenuchten::dcapillaryPressureHys(low_ext_S, 0.1, 0.3, 10.0, 1.9);
const PorousFlowVanGenuchten::LowCapillaryPressureExtension
    low_ext_exp(PorousFlowVanGenuchten::LowCapillaryPressureExtension::EXPONENTIAL,
                low_ext_S,
                low_ext_Pc,
                low_ext_dPc);
const PorousFlowVanGenuchten::LowCapillaryPressureExtension
    low_ext_quad(PorousFlowVanGenuchten::LowCapillaryPressureExtension::QUADRATIC,
                 low_ext_S,
                 low_ext_Pc,
                 low_ext_dPc);
const PorousFlowVanGenuchten::LowCapillaryPressureExtension
    low_ext_none(PorousFlowVanGenuchten::LowCapillaryPressureExtension::NONE,
                 low_ext_S,
                 low_ext_Pc,
                 low_ext_dPc);
const PorousFlowVanGenuchten::HighCapillaryPressureExtension
    no_high_ext(PorousFlowVanGenuchten::HighCapillaryPressureExtension::NONE, 2.0, 0.0, 0.0);
const Real high_ext_S = 0.9 * (1.0 - 0.3); // here 0.9 is the so-called ratio
const Real high_ext_Pc =
    PorousFlowVanGenuchten::capillaryPressureHys(high_ext_S, 0.1, 0.3, 10.0, 1.9);
const Real high_ext_dPc =
    PorousFlowVanGenuchten::dcapillaryPressureHys(high_ext_S, 0.1, 0.3, 10.0, 1.9);
const PorousFlowVanGenuchten::HighCapillaryPressureExtension
    high_ext_power(PorousFlowVanGenuchten::HighCapillaryPressureExtension::POWER,
                   high_ext_S,
                   high_ext_Pc,
                   high_ext_dPc);
const PorousFlowVanGenuchten::HighCapillaryPressureExtension high_ext_none(
    PorousFlowVanGenuchten::HighCapillaryPressureExtension::NONE, 0.8, high_ext_Pc, high_ext_dPc);
const PorousFlowVanGenuchten::HighCapillaryPressureExtension
    high_ext_none2(PorousFlowVanGenuchten::HighCapillaryPressureExtension::NONE,
                   high_ext_S,
                   high_ext_Pc,
                   high_ext_dPc);

TEST(PorousFlowVanGenuchten, sat)
{
  EXPECT_NEAR(1.0, PorousFlowVanGenuchten::effectiveSaturation(1.0E30, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(1.0, PorousFlowVanGenuchten::effectiveSaturation(1.0, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(1.0, PorousFlowVanGenuchten::effectiveSaturation(0.0, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(1.0, PorousFlowVanGenuchten::effectiveSaturation(1.0E-10, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(
      0.486841442435055, PorousFlowVanGenuchten::effectiveSaturation(-2.0, 0.7, 0.6), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::effectiveSaturation(-1.0E30, 0.7, 0.5), 1.0E-5);
}

TEST(PorousFlowVanGenuchten, dsat)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dEffectiveSaturation(1.0E30, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dEffectiveSaturation(1.0, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dEffectiveSaturation(0.0, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dEffectiveSaturation(1.0E-10, 0.7, 0.5), 1.0E-5);
  fd = (PorousFlowVanGenuchten::effectiveSaturation(-2.0 + eps, 0.7, 0.6) -
        PorousFlowVanGenuchten::effectiveSaturation(-2.0, 0.7, 0.6)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::dEffectiveSaturation(-2.0, 0.7, 0.6), 1.0E-5);
  fd = (PorousFlowVanGenuchten::effectiveSaturation(-1.1 + eps, 0.9, 0.66) -
        PorousFlowVanGenuchten::effectiveSaturation(-1.1, 0.9, 0.66)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::dEffectiveSaturation(-1.1, 0.9, 0.66), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dEffectiveSaturation(-1.0E30, 0.7, 0.5), 1.0E-5);
}

TEST(PorousFlowVanGenuchten, d2sat)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2EffectiveSaturation(1.0E30, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2EffectiveSaturation(1.0, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2EffectiveSaturation(0.0, 0.7, 0.5), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2EffectiveSaturation(1.0E-10, 0.7, 0.5), 1.0E-5);
  fd = (PorousFlowVanGenuchten::dEffectiveSaturation(-2.0 + eps, 0.7, 0.6) -
        PorousFlowVanGenuchten::dEffectiveSaturation(-2.0, 0.7, 0.6)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::d2EffectiveSaturation(-2.0, 0.7, 0.6), 1.0E-5);
  fd = (PorousFlowVanGenuchten::dEffectiveSaturation(-1.1 + eps, 2.3, 0.67) -
        PorousFlowVanGenuchten::dEffectiveSaturation(-1.1, 2.3, 0.67)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::d2EffectiveSaturation(-1.1, 2.3, 0.67), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2EffectiveSaturation(-1.0E30, 0.7, 0.5), 1.0E-5);
}

TEST(PorousFlowVanGenuchten, cap)
{
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::capillaryPressure(1.1, 1.0, 0.55, 1.0E30), 1.0E-5);
  EXPECT_NEAR(4.06172392297447,
              PorousFlowVanGenuchten::capillaryPressure(0.3, 0.625, 0.55, 1.0E30),
              1.0E-5);
  EXPECT_NEAR(2.9, PorousFlowVanGenuchten::capillaryPressure(0.001, 0.625, 0.55, 2.9), 1.0E-5);
  EXPECT_NEAR(1000.0, PorousFlowVanGenuchten::capillaryPressure(0.0, 0.625, 0.55, 1000.0), 1.0E-5);
}

TEST(PorousFlowVanGenuchten, dcap)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dCapillaryPressure(0.0, 1.0, 0.55, 1.0E30), 1.0E-5);
  fd = (PorousFlowVanGenuchten::capillaryPressure(0.3 + eps, 0.625, 0.55, 1.0E30) -
        PorousFlowVanGenuchten::capillaryPressure(0.3, 0.625, 0.55, 1.0E30)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::dCapillaryPressure(0.3, 0.625, 0.55, 1.0E30), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dCapillaryPressure(1.0, 1.0, 0.55, 1.0E30), 1.0E-5);
}

TEST(PorousFlowVanGenuchten, d2cap)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2CapillaryPressure(0.0, 1.0, 0.55, 1.0E30), 1.0E-5);
  fd = (PorousFlowVanGenuchten::dCapillaryPressure(0.3 + eps, 0.625, 0.55, 1.0E30) -
        PorousFlowVanGenuchten::dCapillaryPressure(0.3, 0.625, 0.55, 1.0E30)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::d2CapillaryPressure(0.3, 0.625, 0.55, 1.0E30), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2CapillaryPressure(1.0, 0.625, 0.55, 1.0E30), 1.0E-5);
}

TEST(PorousFlowVanGenuchten, relperm)
{
  EXPECT_NEAR(1.0, PorousFlowVanGenuchten::relativePermeability(1.0E30, 0.7), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::relativePermeability(-1.0, 0.7), 1.0E-5);
  EXPECT_NEAR(0.0091160727, PorousFlowVanGenuchten::relativePermeability(0.3, 0.7), 1.0E-5);
}

TEST(PorousFlowVanGenuchten, drelperm)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dRelativePermeability(1.0E30, 0.7), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dRelativePermeability(-1.0, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::relativePermeability(0.3 + eps, 0.7) -
        PorousFlowVanGenuchten::relativePermeability(0.3, 0.7)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::dRelativePermeability(0.3, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::relativePermeability(0.8 + eps, 0.65) -
        PorousFlowVanGenuchten::relativePermeability(0.8, 0.65)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::dRelativePermeability(0.8, 0.65), 1.0E-5);
}

TEST(PorousFlowVanGenuchten, d2relperm)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2RelativePermeability(1.0E30, 0.7), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2RelativePermeability(-1.0, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::dRelativePermeability(0.3 + eps, 0.7) -
        PorousFlowVanGenuchten::dRelativePermeability(0.3, 0.7)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::d2RelativePermeability(0.3, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::dRelativePermeability(0.8 + eps, 0.65) -
        PorousFlowVanGenuchten::dRelativePermeability(0.8, 0.65)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::d2RelativePermeability(0.8, 0.65), 1.0E-5);
}

TEST(PorousFlowVanGenuchten, relpermNW)
{
  EXPECT_NEAR(1.0, PorousFlowVanGenuchten::relativePermeabilityNW(1.0E30, 0.7), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::relativePermeabilityNW(-1.0, 0.7), 1.0E-5);
  EXPECT_NEAR(0.664138097, PorousFlowVanGenuchten::relativePermeabilityNW(0.5, 0.3), 1.0E-5);
  EXPECT_NEAR(0.559879012, PorousFlowVanGenuchten::relativePermeabilityNW(0.7, 0.8), 1.0E-5);
}

TEST(PorousFlowVanGenuchten, drelpermNW)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dRelativePermeabilityNW(1.0E30, 0.7), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dRelativePermeabilityNW(-1.0, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::relativePermeabilityNW(0.3 + eps, 0.7) -
        PorousFlowVanGenuchten::relativePermeabilityNW(0.3, 0.7)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::dRelativePermeabilityNW(0.3, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::relativePermeabilityNW(0.8 + eps, 0.65) -
        PorousFlowVanGenuchten::relativePermeabilityNW(0.8, 0.65)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::dRelativePermeabilityNW(0.8, 0.65), 1.0E-5);
}

TEST(PorousFlowVanGenuchten, d2relpermNW)
{
  Real fd;
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2RelativePermeabilityNW(1.0E30, 0.7), 1.0E-5);
  EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2RelativePermeabilityNW(-1.0, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::dRelativePermeabilityNW(0.3 + eps, 0.7) -
        PorousFlowVanGenuchten::dRelativePermeabilityNW(0.3, 0.7)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::d2RelativePermeabilityNW(0.3, 0.7), 1.0E-5);
  fd = (PorousFlowVanGenuchten::dRelativePermeabilityNW(0.8 + eps, 0.65) -
        PorousFlowVanGenuchten::dRelativePermeabilityNW(0.8, 0.65)) /
       eps;
  EXPECT_NEAR(fd, PorousFlowVanGenuchten::d2RelativePermeabilityNW(0.8, 0.65), 1.0E-5);
}

/// Test the hysteretic capillary pressure
TEST(PorousFlowVanGenuchten, caphys)
{
  // first define some extensions that do not actually induce any extension, so the non-extended Pc
  // can be checked
  const std::vector<Real> no_ext_sats{1.1, 0.75, 0.2, 0.4, 0.6};
  const std::vector<Real> expected_pcs{
      0.0, 0.0, 0.7233512030263158, 0.18806139488299345, 0.06716785873826017};
  for (unsigned i = 0; i < no_ext_sats.size(); ++i)
  {
    EXPECT_NEAR(expected_pcs[i],
                PorousFlowVanGenuchten::capillaryPressureHys(
                    no_ext_sats[i], 0.1, 0.3, 10.0, 1.9, no_low_ext, no_high_ext),
                1.0E-5);
    EXPECT_NEAR(expected_pcs[i],
                PorousFlowVanGenuchten::capillaryPressureHys(no_ext_sats[i], 0.1, 0.3, 10.0, 1.9),
                1.0E-5);
  }
  EXPECT_NEAR(123.0,
              PorousFlowVanGenuchten::capillaryPressureHys(
                  -1.0, 0.1, 0.3, 10.0, 1.9, no_low_ext, no_high_ext),
              1.0E-5);
  EXPECT_NEAR(123.0,
              PorousFlowVanGenuchten::capillaryPressureHys(
                  0.05, 0.1, 0.3, 10.0, 1.9, no_low_ext, no_high_ext),
              1.0E-5);
  EXPECT_NEAR(std::numeric_limits<Real>::max(),
              PorousFlowVanGenuchten::capillaryPressureHys(-1.0, 0.1, 0.3, 10.0, 1.9),
              1.0E-5);
  EXPECT_NEAR(std::numeric_limits<Real>::max(),
              PorousFlowVanGenuchten::capillaryPressureHys(0.05, 0.1, 0.3, 10.0, 1.9),
              1.0E-5);

  // check the lower extension values
  EXPECT_NEAR(0.15229665668299805, low_ext_exp.S, 1.0E-5);
  EXPECT_NEAR(-32.05516428734052, low_ext_exp.dPc, 1.0E-5);

  // check the high extension
  EXPECT_NEAR(0.05300654102157442, high_ext_power.Pc, 1.0E-5);
  EXPECT_NEAR(-0.48230528124844047, high_ext_power.dPc, 1.0E-5);

  // using low_ext_exp and high_ext_power
  const std::vector<Real> sats{0.01, 0.1, 0.15, 0.2, 0.5, 0.63, 0.8, 0.99, 1.0};
  std::vector<Real> pc;
  pc = {31.385947046636815,
        4.5861935551774735,
        1.5754562501536364,
        0.7233512030263158,
        0.11727884570711045,
        0.05300654102157442,
        0.006681337544884095,
        2.7847615834816514e-07,
        0.0};
  for (unsigned i = 0; i < sats.size(); ++i)
    EXPECT_NEAR(pc[i],
                PorousFlowVanGenuchten::capillaryPressureHys(
                    sats[i], 0.1, 0.3, 10.0, 1.9, low_ext_exp, high_ext_power),
                1.0E-5);

  // using low_ext_quad and high_ext_power
  pc = {3.9304232526771696,
        2.8885549236001244,
        1.5730646091089062,
        0.7233512030263158,
        0.11727884570711045,
        0.05300654102157442,
        0.006681337544884095,
        2.7847615834816514e-07,
        0.0};
  for (unsigned i = 0; i < sats.size(); ++i)
    EXPECT_NEAR(pc[i],
                PorousFlowVanGenuchten::capillaryPressureHys(
                    sats[i], 0.1, 0.3, 10.0, 1.9, low_ext_quad, high_ext_power),
                1.0E-5);

  // using low_ext_none and high_ext_power
  pc = {1.5,
        1.5,
        1.5,
        0.7233512030263158,
        0.11727884570711045,
        0.05300654102157442,
        0.006681337544884095,
        2.7847615834816514e-07,
        0.0};
  for (unsigned i = 0; i < sats.size(); ++i)
    EXPECT_NEAR(pc[i],
                PorousFlowVanGenuchten::capillaryPressureHys(
                    sats[i], 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_power),
                1.0E-5);

  // using low_ext_none and high_ext_none
  pc = {1.5, 1.5, 1.5, 0.7233512030263158, 0.11727884570711045, 0.05300654102157442, 0.0, 0.0, 0.0};
  for (unsigned i = 0; i < sats.size(); ++i)
    EXPECT_NEAR(pc[i],
                PorousFlowVanGenuchten::capillaryPressureHys(
                    sats[i], 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_none),
                1.0E-5);
}

/// Test the hysteretic saturation
TEST(PorousFlowVanGenuchten, sathys)
{
  // first define some extensions that do not actually induce any extension, so the non-extended Pc
  // can be checked
  const std::vector<Real> expected_sats{1.0, 0.2, 0.4, 0.6};
  const std::vector<Real> no_ext_pcs{
      -1.1, 0.7233512030263158, 0.18806139488299345, 0.06716785873826017};
  for (unsigned i = 0; i < no_ext_pcs.size(); ++i)
  {
    EXPECT_NEAR(expected_sats[i],
                PorousFlowVanGenuchten::saturationHys(
                    no_ext_pcs[i], 0.1, 0.3, 10.0, 1.9, no_low_ext, no_high_ext),
                1.0E-5);
    EXPECT_NEAR(expected_sats[i],
                PorousFlowVanGenuchten::saturationHys(no_ext_pcs[i], 0.1, 0.3, 10.0, 1.9),
                1.0E-5);
  }
  EXPECT_NEAR(
      -1.0,
      PorousFlowVanGenuchten::saturationHys(1234.0, 0.1, 0.3, 10.0, 1.9, no_low_ext, no_high_ext),
      1.0E-5);
  EXPECT_NEAR(
      0.0,
      PorousFlowVanGenuchten::saturationHys(std::numeric_limits<Real>::max(), 0.1, 0.3, 10.0, 1.9),
      1.0E-5);

  // now with extensions
  std::vector<Real> sats{0.01, 0.1, 0.15, 0.2, 0.5, 0.63, 0.8, 0.99, 1.0};
  std::vector<Real> pc;
  pc = {31.385947046636815,
        4.5861935551774735,
        1.5754562501536364,
        0.7233512030263158,
        0.11727884570711045,
        0.05300654102157442,
        0.006681337544884095,
        2.7847615834816514e-07,
        0.0};
  for (unsigned i = 0; i < sats.size(); ++i)
    EXPECT_NEAR(sats[i],
                PorousFlowVanGenuchten::saturationHys(
                    pc[i], 0.1, 0.3, 10.0, 1.9, low_ext_exp, high_ext_power),
                1.0E-5);

  // different lower extension
  pc = {3.9304232526771696,
        2.8885549236001244,
        1.5730646091089062,
        0.7233512030263158,
        0.11727884570711045,
        0.05300654102157442,
        0.006681337544884095,
        2.7847615834816514e-07,
        0.0};
  for (unsigned i = 0; i < sats.size(); ++i)
    EXPECT_NEAR(sats[i],
                PorousFlowVanGenuchten::saturationHys(
                    pc[i], 0.1, 0.3, 10.0, 1.9, low_ext_quad, high_ext_power),
                1.0E-5);

  // different low extension
  sats = {low_ext_none.S, low_ext_none.S, 0.2, 0.5, 0.63, 0.8, 0.99, 1.0};
  pc = {2.5,
        1.5,
        0.7233512030263158,
        0.11727884570711045,
        0.05300654102157442,
        0.006681337544884095,
        2.7847615834816514e-07,
        0.0};
  for (unsigned i = 0; i < sats.size(); ++i)
    EXPECT_NEAR(sats[i],
                PorousFlowVanGenuchten::saturationHys(
                    pc[i], 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_power),
                1.0E-5);

  // different high extension
  pc = {2.5, 1.5, 0.7233512030263158, 0.11727884570711045, 0.05300654102157442, 1E-10};
  sats = {low_ext_none.S, low_ext_none.S, 0.2, 0.5, 0.63, 0.8};
  for (unsigned i = 0; i < sats.size(); ++i)
    EXPECT_NEAR(sats[i],
                PorousFlowVanGenuchten::saturationHys(
                    pc[i], 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_none),
                1.0E-5);
}

/// Test the first derivative of the hysteretic capillary pressure
TEST(PorousFlowVanGenuchten, dcaphys)
{
  const Real eps = 1.0E-9;

  // first define some extensions that do not actually induce any extension, so the non-extended
  // Pc can be checked
  std::vector<Real> sats{1.1, 0.75, -1.0, 0.05};
  for (const auto & sat : sats)
  {
    EXPECT_NEAR(0.0,
                PorousFlowVanGenuchten::dcapillaryPressureHys(
                    sat, 0.1, 0.3, 10.0, 1.9, no_low_ext, no_high_ext),
                1.0E-5);
    EXPECT_NEAR(
        0.0, PorousFlowVanGenuchten::dcapillaryPressureHys(sat, 0.1, 0.3, 10.0, 1.9), 1.0E-5);
  }

  sats = {0.2, 0.4, 0.6};
  for (const auto & sat : sats)
  {
    const Real fd = 0.5 *
                    (PorousFlowVanGenuchten::capillaryPressureHys(sat + eps, 0.1, 0.3, 10.0, 1.9) -
                     PorousFlowVanGenuchten::capillaryPressureHys(sat - eps, 0.1, 0.3, 10.0, 1.9)) /
                    eps;
    EXPECT_NEAR(fd,
                PorousFlowVanGenuchten::dcapillaryPressureHys(
                    sat, 0.1, 0.3, 10.0, 1.9, no_low_ext, no_high_ext),
                1.0E-5);
    EXPECT_NEAR(
        fd, PorousFlowVanGenuchten::dcapillaryPressureHys(sat, 0.1, 0.3, 10.0, 1.9), 1.0E-5);
  };

  // now with extensions
  sats = {0.01, 0.1, 0.15, 0.2, 0.5, 0.63, 0.8, 0.99, 1.0};

  for (const auto & sat : sats)
  {
    const Real fd = 0.5 *
                    (PorousFlowVanGenuchten::capillaryPressureHys(
                         sat + eps, 0.1, 0.3, 10.0, 1.9, low_ext_exp, high_ext_power) -
                     PorousFlowVanGenuchten::capillaryPressureHys(
                         sat - eps, 0.1, 0.3, 10.0, 1.9, low_ext_exp, high_ext_power)) /
                    eps;
    EXPECT_NEAR(fd,
                PorousFlowVanGenuchten::dcapillaryPressureHys(
                    sat, 0.1, 0.3, 10.0, 1.9, low_ext_exp, high_ext_power),
                1.0E-5);
  };

  // different low extension
  for (const auto & sat : sats)
  {
    const Real fd = 0.5 *
                    (PorousFlowVanGenuchten::capillaryPressureHys(
                         sat + eps, 0.1, 0.3, 10.0, 1.9, low_ext_quad, high_ext_power) -
                     PorousFlowVanGenuchten::capillaryPressureHys(
                         sat - eps, 0.1, 0.3, 10.0, 1.9, low_ext_quad, high_ext_power)) /
                    eps;
    EXPECT_NEAR(fd,
                PorousFlowVanGenuchten::dcapillaryPressureHys(
                    sat, 0.1, 0.3, 10.0, 1.9, low_ext_quad, high_ext_power),
                1.0E-5);
  };

  // different lower extension
  for (const auto & sat : sats)
  {
    const Real fd = 0.5 *
                    (PorousFlowVanGenuchten::capillaryPressureHys(
                         sat + eps, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_power) -
                     PorousFlowVanGenuchten::capillaryPressureHys(
                         sat - eps, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_power)) /
                    eps;
    EXPECT_NEAR(fd,
                PorousFlowVanGenuchten::dcapillaryPressureHys(
                    sat, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_power),
                1.0E-5);
  };

  // different upper extension (cannot evaluate derivative at s = 0.63)
  sats = {0.01, 0.1, 0.15, 0.2, 0.5, 0.8, 0.99, 1.0};
  for (const auto & sat : sats)
  {
    const Real fd = 0.5 *
                    (PorousFlowVanGenuchten::capillaryPressureHys(
                         sat + eps, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_none2) -
                     PorousFlowVanGenuchten::capillaryPressureHys(
                         sat - eps, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_none2)) /
                    eps;
    EXPECT_NEAR(fd,
                PorousFlowVanGenuchten::dcapillaryPressureHys(
                    sat, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_none2),
                1.0E-5);
  };
}

/// Test the second derivative of the hysteretic capillary pressure
TEST(PorousFlowVanGenuchten, d2caphys)
{
  const Real eps = 1.0E-9;

  // first define some extensions that do not actually induce any extension, so the non-extended
  // Pc can be checked
  std::vector<Real> sats{1.1, 0.75, -1.0, 0.05};
  for (const auto & sat : sats)
  {
    EXPECT_NEAR(0.0,
                PorousFlowVanGenuchten::d2capillaryPressureHys(
                    sat, 0.1, 0.3, 10.0, 1.9, no_low_ext, no_high_ext),
                1.0E-5);
    EXPECT_NEAR(
        0.0, PorousFlowVanGenuchten::d2capillaryPressureHys(sat, 0.1, 0.3, 10.0, 1.9), 1.0E-5);
  }

  sats = {0.2, 0.4, 0.6};
  for (const auto & sat : sats)
  {
    const Real fd =
        0.5 *
        (PorousFlowVanGenuchten::dcapillaryPressureHys(sat + eps, 0.1, 0.3, 10.0, 1.9) -
         PorousFlowVanGenuchten::dcapillaryPressureHys(sat - eps, 0.1, 0.3, 10.0, 1.9)) /
        eps;
    EXPECT_NEAR(fd,
                PorousFlowVanGenuchten::d2capillaryPressureHys(
                    sat, 0.1, 0.3, 10.0, 1.9, no_low_ext, no_high_ext),
                1.0E-5);
    EXPECT_NEAR(
        fd, PorousFlowVanGenuchten::d2capillaryPressureHys(sat, 0.1, 0.3, 10.0, 1.9), 1.0E-5);
  };

  // now with extensions
  sats = {0.01, 0.1, 0.15, 0.2, 0.5, 0.6, 0.7, 0.8, 0.99, 1.0};

  for (const auto & sat : sats)
  {
    const Real fd = 0.5 *
                    (PorousFlowVanGenuchten::dcapillaryPressureHys(
                         sat + eps, 0.1, 0.3, 10.0, 1.9, low_ext_exp, high_ext_power) -
                     PorousFlowVanGenuchten::dcapillaryPressureHys(
                         sat - eps, 0.1, 0.3, 10.0, 1.9, low_ext_exp, high_ext_power)) /
                    eps;
    if (std::abs(fd) > 10)
      EXPECT_NEAR(1.0,
                  fd / PorousFlowVanGenuchten::d2capillaryPressureHys(
                           sat, 0.1, 0.3, 10.0, 1.9, low_ext_exp, high_ext_power),
                  1.0E-5);
    else
      EXPECT_NEAR(fd,
                  PorousFlowVanGenuchten::d2capillaryPressureHys(
                      sat, 0.1, 0.3, 10.0, 1.9, low_ext_exp, high_ext_power),
                  1.0E-5);
  };

  // different low extension
  for (const auto & sat : sats)
  {
    const Real fd = 0.5 *
                    (PorousFlowVanGenuchten::dcapillaryPressureHys(
                         sat + eps, 0.1, 0.3, 10.0, 1.9, low_ext_quad, high_ext_power) -
                     PorousFlowVanGenuchten::dcapillaryPressureHys(
                         sat - eps, 0.1, 0.3, 10.0, 1.9, low_ext_quad, high_ext_power)) /
                    eps;
    if (std::abs(fd) > 10)
      EXPECT_NEAR(1.0,
                  fd / PorousFlowVanGenuchten::d2capillaryPressureHys(
                           sat, 0.1, 0.3, 10.0, 1.9, low_ext_quad, high_ext_power),
                  1.0E-5);
    else
      EXPECT_NEAR(fd,
                  PorousFlowVanGenuchten::d2capillaryPressureHys(
                      sat, 0.1, 0.3, 10.0, 1.9, low_ext_quad, high_ext_power),
                  1.0E-5);
  };

  // different lower extension
  for (const auto & sat : sats)
  {
    const Real fd = 0.5 *
                    (PorousFlowVanGenuchten::dcapillaryPressureHys(
                         sat + eps, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_power) -
                     PorousFlowVanGenuchten::dcapillaryPressureHys(
                         sat - eps, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_power)) /
                    eps;
    if (std::abs(fd) > 10)
      EXPECT_NEAR(1.0,
                  fd / PorousFlowVanGenuchten::d2capillaryPressureHys(
                           sat, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_power),
                  1.0E-5);
    else
      EXPECT_NEAR(fd,
                  PorousFlowVanGenuchten::d2capillaryPressureHys(
                      sat, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_power),
                  1.0E-5);
  };

  // different upper extension (cannot evaluate derivative at s = 0.63)
  sats = {0.01, 0.1, 0.15, 0.2, 0.5, 0.8, 0.99, 1.0};
  for (const auto & sat : sats)
  {
    const Real fd = 0.5 *
                    (PorousFlowVanGenuchten::dcapillaryPressureHys(
                         sat + eps, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_none2) -
                     PorousFlowVanGenuchten::dcapillaryPressureHys(
                         sat - eps, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_none2)) /
                    eps;
    if (std::abs(fd) > 10)
      EXPECT_NEAR(1.0,
                  fd / PorousFlowVanGenuchten::d2capillaryPressureHys(
                           sat, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_none2),
                  1.0E-5);
    else
      EXPECT_NEAR(fd,
                  PorousFlowVanGenuchten::d2capillaryPressureHys(
                      sat, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_none2),
                  1.0E-5);
  };
}

/// Test the derivative of the hysteretic saturation
TEST(PorousFlowVanGenuchten, dsathys)
{
  const Real eps = 1E-9;

  // first define some extensions that do not actually induce any extension, so the non-extended Pc
  // can be checked
  std::vector<Real> pcs{-1.1, 1234.0};
  for (const auto & pc : pcs)
  {
    EXPECT_NEAR(
        0.0,
        PorousFlowVanGenuchten::dsaturationHys(pc, 0.1, 0.3, 10.0, 1.9, no_low_ext, no_high_ext),
        1.0E-5);
    EXPECT_NEAR(0.0, PorousFlowVanGenuchten::dsaturationHys(pc, 0.1, 0.3, 10.0, 1.9), 1.0E-5);
  }

  pcs = {0.7, 0.2, 0.07};
  for (const auto & pc : pcs)
  {
    const Real fd = 0.5 *
                    (PorousFlowVanGenuchten::saturationHys(
                         pc + eps, 0.1, 0.3, 10.0, 1.9, no_low_ext, no_high_ext) -
                     PorousFlowVanGenuchten::saturationHys(
                         pc - eps, 0.1, 0.3, 10.0, 1.9, no_low_ext, no_high_ext)) /
                    eps;
    EXPECT_NEAR(
        fd,
        PorousFlowVanGenuchten::dsaturationHys(pc, 0.1, 0.3, 10.0, 1.9, no_low_ext, no_high_ext),
        1.0E-5);
    EXPECT_NEAR(fd, PorousFlowVanGenuchten::dsaturationHys(pc, 0.1, 0.3, 10.0, 1.9), 1.0E-5);
  }

  // now with low and high extensions
  pcs = {31.385947046636815,
         4.5861935551774735,
         1.5754562501536364,
         0.7233512030263158,
         0.11727884570711045,
         0.05300654102157442,
         0.006681337544884095};
  for (const auto & pc : pcs)
  {
    const Real fd = 0.5 *
                    (PorousFlowVanGenuchten::saturationHys(
                         pc + eps, 0.1, 0.3, 10.0, 1.9, low_ext_exp, high_ext_power) -
                     PorousFlowVanGenuchten::saturationHys(
                         pc - eps, 0.1, 0.3, 10.0, 1.9, low_ext_exp, high_ext_power)) /
                    eps;
    EXPECT_NEAR(fd,
                PorousFlowVanGenuchten::dsaturationHys(
                    pc, 0.1, 0.3, 10.0, 1.9, low_ext_exp, high_ext_power),
                1.0E-5);
  }

  // different lower extension
  pcs = {3.9304232526771696,
         2.8885549236001244,
         1.5730646091089062,
         0.7233512030263158,
         0.11727884570711045,
         0.05300654102157442,
         0.006681337544884095};
  for (const auto & pc : pcs)
  {
    const Real fd = 0.5 *
                    (PorousFlowVanGenuchten::saturationHys(
                         pc + eps, 0.1, 0.3, 10.0, 1.9, low_ext_quad, high_ext_power) -
                     PorousFlowVanGenuchten::saturationHys(
                         pc - eps, 0.1, 0.3, 10.0, 1.9, low_ext_quad, high_ext_power)) /
                    eps;
    EXPECT_NEAR(fd,
                PorousFlowVanGenuchten::dsaturationHys(
                    pc, 0.1, 0.3, 10.0, 1.9, low_ext_quad, high_ext_power),
                1.0E-5);
  }

  // different lower extension
  pcs = {2.5, 0.7233512030263158, 0.11727884570711045, 0.05300654102157442, 0.006681337544884095};
  for (const auto & pc : pcs)
  {
    const Real fd = 0.5 *
                    (PorousFlowVanGenuchten::saturationHys(
                         pc + eps, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_power) -
                     PorousFlowVanGenuchten::saturationHys(
                         pc - eps, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_power)) /
                    eps;
    EXPECT_NEAR(fd,
                PorousFlowVanGenuchten::dsaturationHys(
                    pc, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_power),
                1.0E-5);
  }

  // different upper extension
  pcs = {2.5, 0.7233512030263158, 0.11727884570711045, 0.05, 0.04};
  for (const auto & pc : pcs)
  {
    const Real fd = 0.5 *
                    (PorousFlowVanGenuchten::saturationHys(
                         pc + eps, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_none) -
                     PorousFlowVanGenuchten::saturationHys(
                         pc - eps, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_none)) /
                    eps;
    EXPECT_NEAR(fd,
                PorousFlowVanGenuchten::dsaturationHys(
                    pc, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_none),
                1.0E-5);
  }
}

/// Test the second derivative of the hysteretic saturation
TEST(PorousFlowVanGenuchten, d2sathys)
{
  const Real eps = 1E-9;

  // first define some extensions that do not actually induce any extension, so the non-extended Pc
  // can be checked
  std::vector<Real> pcs{-1.1, 1234.0};
  for (const auto & pc : pcs)
  {
    EXPECT_NEAR(
        0.0,
        PorousFlowVanGenuchten::d2saturationHys(pc, 0.1, 0.3, 10.0, 1.9, no_low_ext, no_high_ext),
        1.0E-5);
    EXPECT_NEAR(0.0, PorousFlowVanGenuchten::d2saturationHys(pc, 0.1, 0.3, 10.0, 1.9), 1.0E-5);
  }

  pcs = {0.7, 0.2, 0.07};
  for (const auto & pc : pcs)
  {
    const Real fd = 0.5 *
                    (PorousFlowVanGenuchten::dsaturationHys(
                         pc + eps, 0.1, 0.3, 10.0, 1.9, no_low_ext, no_high_ext) -
                     PorousFlowVanGenuchten::dsaturationHys(
                         pc - eps, 0.1, 0.3, 10.0, 1.9, no_low_ext, no_high_ext)) /
                    eps;
    EXPECT_NEAR(
        fd,
        PorousFlowVanGenuchten::d2saturationHys(pc, 0.1, 0.3, 10.0, 1.9, no_low_ext, no_high_ext),
        1.0E-5);
    EXPECT_NEAR(fd, PorousFlowVanGenuchten::d2saturationHys(pc, 0.1, 0.3, 10.0, 1.9), 1.0E-5);
  }

  // now with low and high extensions
  pcs = {31.385947046636815,
         4.5861935551774735,
         1.5754562501536364,
         0.7233512030263158,
         0.11727884570711045,
         0.06300654102157442,
         0.006681337544884095};
  for (const auto & pc : pcs)
  {
    const Real fd = 0.5 *
                    (PorousFlowVanGenuchten::dsaturationHys(
                         pc + eps, 0.1, 0.3, 10.0, 1.9, low_ext_exp, high_ext_power) -
                     PorousFlowVanGenuchten::dsaturationHys(
                         pc - eps, 0.1, 0.3, 10.0, 1.9, low_ext_exp, high_ext_power)) /
                    eps;
    EXPECT_NEAR(fd,
                PorousFlowVanGenuchten::d2saturationHys(
                    pc, 0.1, 0.3, 10.0, 1.9, low_ext_exp, high_ext_power),
                1.0E-5);
  }

  // different lower extension
  pcs = {3.9304232526771696,
         2.8885549236001244,
         1.5730646091089062,
         0.7233512030263158,
         0.11727884570711045,
         0.06300654102157442,
         0.006681337544884095};
  for (const auto & pc : pcs)
  {
    const Real fd = 0.5 *
                    (PorousFlowVanGenuchten::dsaturationHys(
                         pc + eps, 0.1, 0.3, 10.0, 1.9, low_ext_quad, high_ext_power) -
                     PorousFlowVanGenuchten::dsaturationHys(
                         pc - eps, 0.1, 0.3, 10.0, 1.9, low_ext_quad, high_ext_power)) /
                    eps;
    EXPECT_NEAR(fd,
                PorousFlowVanGenuchten::d2saturationHys(
                    pc, 0.1, 0.3, 10.0, 1.9, low_ext_quad, high_ext_power),
                1.0E-5);
  }

  // different lower extension
  pcs = {2.5, 0.7233512030263158, 0.11727884570711045, 0.06300654102157442, 0.006681337544884095};
  for (const auto & pc : pcs)
  {
    const Real fd = 0.5 *
                    (PorousFlowVanGenuchten::dsaturationHys(
                         pc + eps, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_power) -
                     PorousFlowVanGenuchten::dsaturationHys(
                         pc - eps, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_power)) /
                    eps;
    EXPECT_NEAR(fd,
                PorousFlowVanGenuchten::d2saturationHys(
                    pc, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_power),
                1.0E-5);
  }

  // different upper extension
  pcs = {2.5, 0.7233512030263158, 0.11727884570711045, 0.05, 0.04};
  for (const auto & pc : pcs)
  {
    const Real fd = 0.5 *
                    (PorousFlowVanGenuchten::dsaturationHys(
                         pc + eps, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_none) -
                     PorousFlowVanGenuchten::dsaturationHys(
                         pc - eps, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_none)) /
                    eps;
    EXPECT_NEAR(fd,
                PorousFlowVanGenuchten::d2saturationHys(
                    pc, 0.1, 0.3, 10.0, 1.9, low_ext_none, high_ext_none),
                1.0E-5);
  }
}
