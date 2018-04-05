//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include <array>

#include "Setup.h"

TEST(FunctionalExpansionsTest, zernikeConstructor)
{
  const unsigned int order = 5;
  expansion_type = "orthonormal";
  generation_type = "standard";
  Zernike zernike({FunctionalBasisInterface::_domain_options = "x",
                   FunctionalBasisInterface::_domain_options = "y"},
                  {order},
                  expansion_type,
                  generation_type);
  EXPECT_EQ(zernike.getOrder(0), order);
}

TEST(FunctionalExpansionsTest, zernikeSeriesEvaluationXY)
{
  const unsigned int order = 4;
  const Point location(-0.90922108754014, 0.262698547343, 0.156796889218);
  expansion_type = "standard";
  generation_type = "orthonormal";

  Zernike zernike({FunctionalBasisInterface::_domain_options = "x",
                   FunctionalBasisInterface::_domain_options = "y"},
                  {order},
                  expansion_type,
                  generation_type);
  zernike.setLocation(location);
  const std::array<Real, 15> truth = {{0.318309886183791,
                                       -0.300628811510287,
                                       -1.117940202314423,
                                       0.791881706198328,
                                       0.623918544603900,
                                       1.365900806059890,
                                       -1.357069098439213,
                                       -0.288633095470502,
                                       -1.073332058640328,
                                       -1.349767446988918,
                                       1.887803726543957,
                                       0.404825692079851,
                                       0.223343150486739,
                                       0.698275682841869,
                                       1.080889714660983}};

  auto & answer = zernike.getAllGeneration();
  for (std::size_t i = 0; i < zernike.getNumberOfTerms(); ++i)
    EXPECT_NEAR(answer[i], truth[i], tol);
}

TEST(FunctionalExpansionsTest, zernikeSeriesEvaluationXZ)
{
  const unsigned int order = 4;
  const Point location(-0.90922108754014, 0.262698547343, 0.156796889218);
  expansion_type = "standard";
  generation_type = "orthonormal";

  Zernike zernike({FunctionalBasisInterface::_domain_options = "x",
                   FunctionalBasisInterface::_domain_options = "z"},
                  {order},
                  expansion_type,
                  generation_type);
  zernike.setLocation(location);
  const std::array<Real, 15> truth = {{0.318309886183791,
                                       -0.180774038043348,
                                       -1.143454732567233,
                                       0.487041727962393,
                                       0.623918544603900,
                                       1.501849527692451,
                                       -0.867504286868072,
                                       -0.173560777222340,
                                       -1.097828505968007,
                                       -1.706149176114187,
                                       1.276644449771070,
                                       0.248985426801565,
                                       0.223343150486739,
                                       0.767775375651402,
                                       1.761335979897610}};

  auto & answer = zernike.getAllGeneration();
  for (std::size_t i = 0; i < zernike.getNumberOfTerms(); ++i)
    EXPECT_NEAR(answer[i], truth[i], tol);
}

TEST(FunctionalExpansionsTest, zernikeSeriesEvaluationYZ)
{
  const unsigned int order = 4;
  const Point location(-0.90922108754014, 0.262698547343, 0.156796889218);
  expansion_type = "standard";
  generation_type = "orthonormal";

  Zernike zernike({FunctionalBasisInterface::_domain_options = "y",
                   FunctionalBasisInterface::_domain_options = "z"},
                  {order},
                  expansion_type,
                  generation_type);
  zernike.setLocation(location);
  const std::array<Real, 15> truth = {{0.318309886183791,
                                       0.052230505695590,
                                       0.330374978445085,
                                       0.040657672622662,
                                       -0.823129261009826,
                                       0.125372638358686,
                                       0.020923587239414,
                                       -0.187295294511344,
                                       -1.184703806003468,
                                       0.041151106306071,
                                       0.008896570968308,
                                       -0.184582980412102,
                                       0.978025517529447,
                                       -0.569182979683797,
                                       0.012274247968574}};

  auto & answer = zernike.getAllGeneration();
  for (std::size_t i = 0; i < zernike.getNumberOfTerms(); ++i)
    EXPECT_NEAR(answer[i], truth[i], tol);
}

TEST(FunctionalExpansionsTest, zernikeSeriesSqrtMuEvaluation)
{
  const unsigned int order = 4;
  const Point location(-0.90922108754014, 0.262698547343, 0.156796889218);
  expansion_type = "standard";
  generation_type = "sqrt_mu";

  Zernike zernike({FunctionalBasisInterface::_domain_options = "y",
                   FunctionalBasisInterface::_domain_options = "z"},
                  {order},
                  expansion_type,
                  generation_type);
  zernike.setLocation(location);
  const std::array<Real, 15> truth = {{0.318309886183791,
                                       0.052230505695590,
                                       0.330374978445085,
                                       0.040657672622662,
                                       -0.823129261009826,
                                       0.125372638358686,
                                       0.020923587239414,
                                       -0.187295294511344,
                                       -1.184703806003468,
                                       0.041151106306071,
                                       0.008896570968308,
                                       -0.184582980412102,
                                       0.978025517529447,
                                       -0.569182979683797,
                                       0.012274247968574}};

  auto & answer = zernike.getAllGeneration();
  EXPECT_NEAR(answer[0], truth[0] * std::sqrt(M_PI), tol);
  size_t i = 1;
  for (size_t n = 1; n < order + 1; ++n)
  {
    for (size_t m = 0; m < n + 1; ++m)
    {
      if (m != 0 && n / m == 2 && n % m == 0)
        EXPECT_NEAR(answer[i], truth[i] * std::sqrt(M_PI / (n + 1)), tol);
      else
        EXPECT_NEAR(answer[i], truth[i] * std::sqrt(M_PI / (2 * n + 2)), tol);
      ++i;
    }
  }
}

TEST(FunctionalExpansionsTest, zernikeSeriesStandardEvaluation)
{
  const unsigned int order = 4;
  const Point location(-0.90922108754014, 0.262698547343, 0.156796889218);
  expansion_type = "standard";
  generation_type = "standard";

  Zernike zernike({FunctionalBasisInterface::_domain_options = "y",
                   FunctionalBasisInterface::_domain_options = "z"},
                  {order},
                  expansion_type,
                  generation_type);
  zernike.setLocation(location);
  const std::array<Real, 15> truth = {{0.318309886183791,
                                       0.052230505695590,
                                       0.330374978445085,
                                       0.040657672622662,
                                       -0.823129261009826,
                                       0.125372638358686,
                                       0.020923587239414,
                                       -0.187295294511344,
                                       -1.184703806003468,
                                       0.041151106306071,
                                       0.008896570968308,
                                       -0.184582980412102,
                                       0.978025517529447,
                                       -0.569182979683797,
                                       0.012274247968574}};

  auto & answer = zernike.getAllGeneration();
  EXPECT_NEAR(answer[0], truth[0] * M_PI, tol);
  size_t i = 1;
  for (size_t n = 1; n < order + 1; ++n)
  {
    for (size_t m = 0; m < n + 1; ++m)
    {
      if (m != 0 && n / m == 2 && n % m == 0)
        EXPECT_NEAR(answer[i], truth[i] * M_PI / (n + 1), tol);
      else
        EXPECT_NEAR(answer[i], truth[i] * M_PI / (2 * n + 2), tol);
      ++i;
    }
  }
}

TEST(FunctionalExpansionsTest, cylindricalDuoConstructorAxialX)
{
  const std::vector<MooseEnum> domains = {FunctionalBasisInterface::_domain_options = "x",
                                          FunctionalBasisInterface::_domain_options = "y",
                                          FunctionalBasisInterface::_domain_options = "z"};
  const std::vector<std::size_t> orders = {5, 18};
  const std::vector<MooseEnum> series = {single_series_types_1D = "Legendre",
                                         single_series_types_2D = "Zernike"};
  expansion_type = "standard";
  generation_type = "orthonormal";

  CylindricalDuo cylindrical(domains, orders, series, name, expansion_type, generation_type);
  EXPECT_EQ(cylindrical.getNumberOfTerms(),
            (orders[0] + 1) * ((orders[1] + 1) * (orders[1] + 2)) / 2);
}

TEST(FunctionalExpansionsTest, cylindricalDuoConstructorAxialY)
{
  const std::vector<MooseEnum> domains = {FunctionalBasisInterface::_domain_options = "y",
                                          FunctionalBasisInterface::_domain_options = "x",
                                          FunctionalBasisInterface::_domain_options = "z"};
  const std::vector<std::size_t> orders = {23, 8};
  const std::vector<MooseEnum> series = {single_series_types_1D = "Legendre",
                                         single_series_types_2D = "Zernike"};
  expansion_type = "standard";
  generation_type = "orthonormal";

  CylindricalDuo cylindrical(domains, orders, series, name, expansion_type, generation_type);
  EXPECT_EQ(cylindrical.getNumberOfTerms(),
            (orders[0] + 1) * ((orders[1] + 1) * (orders[1] + 2)) / 2);
}

TEST(FunctionalExpansionsTest, cylindricalDuoConstructorAxialZ)
{
  const std::vector<MooseEnum> domains = {FunctionalBasisInterface::_domain_options = "z",
                                          FunctionalBasisInterface::_domain_options = "x",
                                          FunctionalBasisInterface::_domain_options = "y"};
  const std::vector<std::size_t> orders = {21, 23};
  const std::vector<MooseEnum> series = {single_series_types_1D = "Legendre",
                                         single_series_types_2D = "Zernike"};
  expansion_type = "standard";
  generation_type = "orthonormal";

  CylindricalDuo cylindrical(domains, orders, series, name, expansion_type, generation_type);
  EXPECT_EQ(cylindrical.getNumberOfTerms(),
            (orders[0] + 1) * ((orders[1] + 1) * (orders[1] + 2)) / 2);
}

TEST(FunctionalExpansionsTest, cylindricalDuoEvaluator)
{
  const std::vector<MooseEnum> domains = {FunctionalBasisInterface::_domain_options = "x",
                                          FunctionalBasisInterface::_domain_options = "y",
                                          FunctionalBasisInterface::_domain_options = "z"};
  const std::vector<std::size_t> orders = {15, 17};
  const std::vector<MooseEnum> series = {single_series_types_1D = "Legendre",
                                         single_series_types_2D = "Zernike"};
  expansion_type = "standard";
  generation_type = "orthonormal";

  CylindricalDuo cylindrical(domains, orders, series, name, expansion_type, generation_type);

  const std::vector<Point> locations = {
      Point(-0.14854612627465, 0.60364074055275, 0.76978431165674),
      Point(0.93801805187856, 0.74175118177279, 0.45207131996044),
      Point(0.35423736896098, -0.83921049062126, -0.02231845586669)};
  const std::vector<Real> standard_truth = {
      0.42889689399543629, 4.3724388003439207, 0.82275646257084989};
  const std::vector<Real> orthogonal_truth = {
      -10.386457517518826, -161.7959192066881, -3.9949571266605481};

  for (std::size_t i = 0; i < locations.size(); ++i)
  {
    cylindrical.setLocation(locations[i]);
    EXPECT_NEAR(cylindrical.getExpansionSeriesSum(), standard_truth[i], tol);
    EXPECT_NEAR(cylindrical.getGenerationSeriesSum(), orthogonal_truth[i], tol);
  }
}

TEST(FunctionalExpansionsTest, functionalBasisInterfaceCylindrical)
{
  const std::vector<MooseEnum> domains = {FunctionalBasisInterface::_domain_options = "x",
                                          FunctionalBasisInterface::_domain_options = "y",
                                          FunctionalBasisInterface::_domain_options = "z"};
  const std::vector<std::size_t> orders = {4, 5};
  const std::vector<MooseEnum> series = {single_series_types_1D = "Legendre",
                                         single_series_types_2D = "Zernike"};
  expansion_type = "standard";
  generation_type = "orthonormal";

  CylindricalDuo cylindrical(domains, orders, series, name, expansion_type, generation_type);

  const Point location(-0.38541903411291, 0.61369802505416, -0.04539307255549);
  const Real truth = 0.10414963426362565;
  FunctionalBasisInterface & interface = (FunctionalBasisInterface &)cylindrical;

  interface.setLocation(location);
  EXPECT_NEAR(interface.getExpansionSeriesSum(), truth, tol);
}
