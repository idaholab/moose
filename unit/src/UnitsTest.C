//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "Units.h"
#include <cmath>
#include <sstream>

TEST(Units, numbers)
{
  // test every unit in a bunch of cases
  std::string one_over = "1/";
  std::string one_times = "1.0*";
  for (const auto & u : MooseUnits::_unit_table)
  {
    EXPECT_DOUBLE_EQ(Real(MooseUnits(one_over + u.first) * MooseUnits(u.first)), 1.0);
    EXPECT_DOUBLE_EQ(Real(MooseUnits(one_times + u.first) / MooseUnits(u.first)), 1.0);
  }

  EXPECT_DOUBLE_EQ(Real(MooseUnits("1000.0m") / MooseUnits("km")), 1.0);
  EXPECT_DOUBLE_EQ(Real(MooseUnits("1000.0*m") / MooseUnits("km")), 1.0);
  EXPECT_DOUBLE_EQ(Real(MooseUnits("10^3*m") / MooseUnits("km")), 1.0);
  EXPECT_DOUBLE_EQ(Real(MooseUnits("1e3*m") / MooseUnits("km")), 1.0);
  EXPECT_DOUBLE_EQ(Real(MooseUnits("m/0.001") / MooseUnits("km")), 1.0);
  EXPECT_DOUBLE_EQ(Real(MooseUnits("m/0.1^3") / MooseUnits("km")), 1.0);
  EXPECT_DOUBLE_EQ(Real(MooseUnits("m/1e-3") / MooseUnits("km")), 1.0);
  EXPECT_DOUBLE_EQ(Real(MooseUnits("1234.567")), 1234.567);
}

TEST(Units, si_prefixes)
{
  // test every possible combination of units and si prefixes for correct parsing
  for (const auto & u : MooseUnits::_unit_table)
  {
    auto unit = MooseUnits(u.first);
    for (const auto & p : MooseUnits::_si_prefix)
    {
      auto prefixed = MooseUnits(p.first + u.first);
      EXPECT_DOUBLE_EQ(Real(prefixed / unit), p.second);
    }
  }
}

TEST(Units, parse)
{
  // check the parsing and plain text stringification of units
  std::vector<std::pair<std::string, std::string>> pairs = {
      {"m^3", "m^3"},
      {"N*m", "m^2*kg*s^-2"},
      {"mm", "0.001 m"},
      {"kg/(m*s^2)", "m^-1*kg*s^-2"},
      {"kg/(m*s)^2", "m^-2*kg*s^-2"},
      {"eV/at", "1.60218e-19 m^2*kg*s^-2*at^-1"},
      {"J/mol", "1.66054e-24 m^2*kg*s^-2*at^-1"},
      {"m/s*s/m", "1"},
      {"km/s*s/m", "1000"},
      {"(km/s)*(s/m)", "1000"},
      {"GPa", "1e+09 m^-1*kg*s^-2"},
      {"psi", "6894.76 m^-1*kg*s^-2"},
      {"kbar", "1e+08 m^-1*kg*s^-2"},
      {"9*degF/m", "5 m^-1*K"},
      {"mdegC/m", "0.001 m^-1*K"}};

  for (auto & p : pairs)
  {
    auto u = MooseUnits(p.first);
    std::stringstream s;
    s << u;
    EXPECT_EQ(s.str(), p.second);
  }
}

TEST(Units, conformsTo)
{
  // check the conformsTo function for a few pairs of units
  std::vector<std::pair<std::string, std::string>> pairs = {
      {"erg/in", "J/nm"}, {"erg", "eV"}, {"ft", "in"}, {"N/m^2", "bar"}, {"eV/at", "J/mol"}};

  for (auto & u : pairs)
    EXPECT_TRUE(MooseUnits(u.first).conformsTo(MooseUnits(u.second)))
        << "units " << u.first << " and " << u.second << " should conform!";
}

TEST(Units, convert)
{
  std::vector<std::pair<std::pair<Real, std::string>, std::pair<Real, std::string>>> pairs = {
      // pure temperature units will convert with additive shifts
      {{0.0, "degC"}, {273.15, "K"}},
      {{0.0, "degC"}, {32.0, "degF"}},
      {{100.0, "degC"}, {212.0, "degF"}},
      {{100.0, "degC"}, {373.15, "K"}},
      {{100.0, "degF"}, {37.77777778, "degC"}},
      {{100.0, "degF"}, {310.927777778, "K"}},
      {{100.0, "K"}, {-279.67, "degF"}},
      // temperature units in complex unit expressions are treated as differential
      {{1.0, "K/m"}, {1.0, "degC/m"}},
      {{9.0, "degF/m"}, {5.0, "degC/m"}},
      // other conversions
      {{96.48533212, "kJ/mol"}, {1.0, "eV/at"}}};

  for (const auto & p : pairs)
  {
    const auto v1 = p.first.first;
    const auto v2 = p.second.first;
    const auto u1 = MooseUnits(p.first.second);
    const auto u2 = MooseUnits(p.second.second);

    // verify convert in both directions
    EXPECT_NEAR(u1.convert(v2, u2), v1, 1e-8);
    EXPECT_NEAR(u2.convert(v1, u1), v2, 1e-8);
  }
}

TEST(Units, isBase)
{
  // check the dimension helpers (and at the same time the parsing of complex units)
  std::vector<std::string> lengths = {"m", "mum", "mm", "cm", "ft", "km", "lbf/psi/m", "N/kg*s^2"};
  for (auto & l : lengths)
  {
    auto u = MooseUnits(l);
    EXPECT_TRUE(u.isLength()) << l << " = " << u << " should be a length!";
    EXPECT_FALSE(u.isTime()) << l << " = " << u << " should not be a time!";
    EXPECT_FALSE(u.isMass()) << l << " = " << u << " should not be a mass!";
    EXPECT_FALSE(u.isCurrent()) << l << " = " << u << " should not be a current!";
    EXPECT_FALSE(u.isTemperature()) << l << " = " << u << " should not be a temperature!";
  }

  std::vector<std::string> times = {"s", "ms", "Gs", "C/A"};
  for (auto & t : times)
  {
    auto u = MooseUnits(t);
    EXPECT_TRUE(u.isTime()) << t << " = " << u << " should be a time!";
    EXPECT_FALSE(u.isLength()) << t << " = " << u << " should not be a length!";
    EXPECT_FALSE(u.isMass()) << t << " = " << u << " should not be a mass!";
    EXPECT_FALSE(u.isCurrent()) << t << " = " << u << " should not be a current!";
    EXPECT_FALSE(u.isTemperature()) << t << " = " << u << " should not be a temperature!";
  }

  std::vector<std::string> masses = {"g", "kg", "N/m*s^2", "atm*in*s^2", "meV/m^2*s^2"};
  for (auto & m : masses)
  {
    auto u = MooseUnits(m);
    EXPECT_TRUE(u.isMass()) << m << " = " << u << " should be a mass!";
    EXPECT_FALSE(u.isTime()) << m << " = " << u << " should not be a time!";
    EXPECT_FALSE(u.isCurrent()) << m << " = " << u << " should not be a current!";
    EXPECT_FALSE(u.isLength()) << m << " = " << u << " should not be a length!";
    EXPECT_FALSE(u.isTemperature()) << m << " = " << u << " should not be a temperature!";
  }

  std::vector<std::string> currents = {"A", "W*S/A", "mA", "nA"};
  for (auto & c : currents)
  {
    auto u = MooseUnits(c);
    EXPECT_TRUE(u.isCurrent()) << c << " = " << u << " should be a current!";
    EXPECT_FALSE(u.isTime()) << c << " = " << u << " should not be a time!";
    EXPECT_FALSE(u.isMass()) << c << " = " << u << " should not be a mass!";
    EXPECT_FALSE(u.isLength()) << c << " = " << u << " should not be a length!";
    EXPECT_FALSE(u.isTemperature()) << c << " = " << u << " should not be a temperature!";
  }

  std::vector<std::string> temperatures = {
      "K", "mK", "K^3/K^2", "(K*K^4)/(K*K)^2", "degC", "degF", "mdegC"};
  for (auto & T : temperatures)
  {
    auto u = MooseUnits(T);
    EXPECT_TRUE(u.isTemperature()) << T << " = " << u << " should be a temperature!";
    EXPECT_FALSE(u.isTime()) << T << " = " << u << " should not be a time!";
    EXPECT_FALSE(u.isMass()) << T << " = " << u << " should not be a mass!";
    EXPECT_FALSE(u.isLength()) << T << " = " << u << " should not be a length!";
    EXPECT_FALSE(u.isCurrent()) << T << " = " << u << " should not be a current!";
  }
}
