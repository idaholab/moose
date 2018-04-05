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

TEST(FunctionalExpansionsTest, legendreConstructor)
{
  const unsigned int order = 5;

  Legendre legendre({FunctionalBasisInterface::_domain_options = "x"},
                    {order},
                    expansion_type = "orthonormal",
                    generation_type = "standard");
  EXPECT_EQ(legendre.getOrder(0), order);
}

TEST(FunctionalExpansionsTest, legendreSeriesOrthonormalEvaluation)
{
  const unsigned int order = 15;
  Real location = -0.90922108754014;
  const std::array<Real, order> truth = {{0.50000000000000,
                                          -1.36383163131021,
                                          1.85006119760378,
                                          -1.80341832197563,
                                          1.19175581122701,
                                          -0.11669847057321,
                                          -1.20462734483853,
                                          2.48341349094950,
                                          -3.41981864606651,
                                          3.76808851494207,
                                          -3.39261995754146,
                                          2.30300489952095,
                                          -0.66011244776270,
                                          -1.24901920248131,
                                          3.06342136027001}};

  Legendre legendre({FunctionalBasisInterface::_domain_options = "x"},
                    {order},
                    expansion_type = "standard",
                    generation_type = "orthonormal");

  legendre.setLocation(Point(location));
  auto & answer = legendre.getAllGeneration();
  for (std::size_t i = 0; i < order; ++i)
    EXPECT_NEAR(answer[i], truth[i], tol);
}

TEST(FunctionalExpansionsTest, legendreSeriesSqrtMuEvaluation)
{
  const unsigned int order = 15;
  Real location = -0.90922108754014;
  const std::array<Real, order> truth = {{0.50000000000000,
                                          -1.36383163131021,
                                          1.85006119760378,
                                          -1.80341832197563,
                                          1.19175581122701,
                                          -0.11669847057321,
                                          -1.20462734483853,
                                          2.48341349094950,
                                          -3.41981864606651,
                                          3.76808851494207,
                                          -3.39261995754146,
                                          2.30300489952095,
                                          -0.66011244776270,
                                          -1.24901920248131,
                                          3.06342136027001}};

  Legendre legendre({FunctionalBasisInterface::_domain_options = "x"},
                    {order},
                    expansion_type = "standard",
                    generation_type = "sqrt_mu");

  legendre.setLocation(Point(location));
  auto & answer = legendre.getAllGeneration();
  for (std::size_t i = 0; i < order; ++i)
    EXPECT_NEAR(answer[i], truth[i] / std::sqrt(i + 0.5), tol);
}

TEST(FunctionalExpansionsTest, legendreSeriesStandardEvaluation)
{
  const unsigned int order = 15;
  Real location = -0.90922108754014;
  const std::array<Real, order> truth = {{0.50000000000000,
                                          -1.36383163131021,
                                          1.85006119760378,
                                          -1.80341832197563,
                                          1.19175581122701,
                                          -0.11669847057321,
                                          -1.20462734483853,
                                          2.48341349094950,
                                          -3.41981864606651,
                                          3.76808851494207,
                                          -3.39261995754146,
                                          2.30300489952095,
                                          -0.66011244776270,
                                          -1.24901920248131,
                                          3.06342136027001}};

  Legendre legendre({FunctionalBasisInterface::_domain_options = "x"},
                    {order},
                    expansion_type = "standard",
                    generation_type = "standard");

  legendre.setLocation(Point(location));
  auto & answer = legendre.getAllGeneration();
  for (std::size_t i = 0; i < order; ++i)
    EXPECT_NEAR(answer[i], truth[i] / (i + 0.5), tol);
}

TEST(FunctionalExpansionsTest, CartesianConstructor)
{
  std::vector<MooseEnum> domains;
  std::vector<std::size_t> orders;
  std::vector<MooseEnum> series;
  expansion_type = "standard";
  generation_type = "orthonormal";

  domains.push_back(FunctionalBasisInterface::_domain_options = "x");
  orders = {19};
  series.push_back(single_series_types_1D = "Legendre");
  Cartesian legendreOne(domains, orders, series, name, expansion_type, generation_type);
  EXPECT_EQ(legendreOne.getNumberOfTerms(), orders[0] + 1);

  domains.push_back(FunctionalBasisInterface::_domain_options = "y");
  orders = {{13, 15}};
  series.push_back(single_series_types_1D = "Legendre");
  Cartesian legendreTwo(domains, orders, series, name, expansion_type, generation_type);
  EXPECT_EQ(legendreTwo.getNumberOfTerms(), (orders[0] + 1) * (orders[1] + 1));

  domains.push_back(FunctionalBasisInterface::_domain_options = "z");
  orders = {{14, 21, 22}};
  series.push_back(single_series_types_1D = "Legendre");
  Cartesian legendreThree(domains, orders, series, name, expansion_type, generation_type);
  EXPECT_EQ(legendreThree.getNumberOfTerms(), (orders[0] + 1) * (orders[1] + 1) * (orders[2] + 1));
}

TEST(FunctionalExpansionsTest, Cartesian3D)
{
  const std::vector<MooseEnum> domains = {FunctionalBasisInterface::_domain_options = "x",
                                          FunctionalBasisInterface::_domain_options = "y",
                                          FunctionalBasisInterface::_domain_options = "z"};
  const std::vector<std::size_t> orders = {14, 21, 22};
  const std::vector<MooseEnum> series = {single_series_types_1D = "Legendre",
                                         single_series_types_1D = "Legendre",
                                         single_series_types_1D = "Legendre"};

  Cartesian legendre3D(
      domains, orders, series, name, expansion_type = "standard", generation_type = "orthonormal");

  const std::vector<Point> locations = {
      Point(-0.14854612627465, 0.60364074055275, 0.76978431165674),
      Point(0.93801805187856, 0.74175118177279, 0.74211345600994),
      Point(0.35423736896098, -0.83921049062126, -0.02231845586669)};
  const std::vector<Real> standard_truth = {1.32257143058688, 3.68047786932034, 0.17515811557416};
  const std::vector<Real> orthogonal_truth = {
      -2.33043696271172, 74.48747654183713, -14.48091828923379};

  for (std::size_t i = 0; i < locations.size(); ++i)
  {
    legendre3D.setLocation(locations[i]);
    EXPECT_NEAR(legendre3D.getExpansionSeriesSum(), standard_truth[i], tol);
    EXPECT_NEAR(legendre3D.getGenerationSeriesSum(), orthogonal_truth[i], tol);
  }
}

TEST(FunctionalExpansionsTest, functionalBasisInterfaceCartesian)
{
  const std::vector<MooseEnum> domains = {FunctionalBasisInterface::_domain_options = "x",
                                          FunctionalBasisInterface::_domain_options = "y",
                                          FunctionalBasisInterface::_domain_options = "z"};
  const std::vector<std::size_t> orders = {4, 5, 3};
  const std::vector<MooseEnum> series = {single_series_types_1D = "Legendre",
                                         single_series_types_1D = "Legendre",
                                         single_series_types_1D = "Legendre"};

  Cartesian legendre3D(
      domains, orders, series, name, expansion_type = "standard", generation_type = "orthonormal");

  const Point location(-0.38541903411291, 0.61369802505416, -0.04539307255549);
  const Real truth = 0.26458908225718;
  FunctionalBasisInterface & interface = (FunctionalBasisInterface &)legendre3D;

  interface.setLocation(location);
  EXPECT_NEAR(interface.getExpansionSeriesSum(), truth, tol);
}
