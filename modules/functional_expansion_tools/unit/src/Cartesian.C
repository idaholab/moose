#include "gtest/gtest.h"

#include "Cartesian.h"
#include "Legendre.h"

const double tol = 1e-13;

MooseEnum SingleSeriesTypes1D("Legendre");


TEST(FunctionalExpansionsTest, legendreConstructor)
{
  const unsigned int order = 5;
  Legendre legendre({FBI::_domain_options = "x"}, {order});
  EXPECT_EQ(legendre.getOrder(0), order);
}

TEST(FunctionalExpansionsTest, legendreSeriesEvaluation)
{
  const unsigned int order = 15;
  Real location = -0.90922108754014;
  std::array<Real, order> truth = {{ 0.50000000000000,
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
  Legendre legendre({FBI::_domain_options = "x"}, {order});

  legendre.setLocation(Point(location));
  auto& answer = legendre.getAllOrthonormal();
  for (std::size_t i = 0; i < order; ++i)
    EXPECT_NEAR(answer[i], truth[i], tol);
}

TEST(FunctionalExpansionsTest, cartesianConstructor)
{
  std::vector<MooseEnum> domains;
  std::vector<std::size_t> orders;
  std::vector<MooseEnum> series;

  domains.push_back(FBI::_domain_options = "x");
  orders = {19};
  series.push_back(SingleSeriesTypes1D = "Legendre");
  Cartesian legendreOne(domains, orders, series);
  EXPECT_EQ(legendreOne.getNumberOfTerms(), orders[0] + 1);

  domains.push_back(FBI::_domain_options = "y");
  orders = {{13, 15}};
  series.push_back(SingleSeriesTypes1D = "Legendre");
  Cartesian legendreTwo(domains, orders, series);
  EXPECT_EQ(legendreTwo.getNumberOfTerms(), (orders[0] + 1) * (orders[1] + 1));

  domains.push_back(FBI::_domain_options = "z");
  orders = {{14, 21, 22}};
  series.push_back(SingleSeriesTypes1D = "Legendre");
  Cartesian legendreThree(domains, orders, series);
  EXPECT_EQ(legendreThree.getNumberOfTerms(), (orders[0] + 1) * (orders[1] + 1) * (orders[2] + 1));
}

TEST(FunctionalExpansionsTest, Cartesian3D)
{
  std::vector<MooseEnum> domains;
  std::vector<std::size_t> orders;
  std::vector<MooseEnum> series;

  domains.push_back(FBI::_domain_options = "x");
  orders.push_back(14);
  series.push_back(SingleSeriesTypes1D = "Legendre");

  domains.push_back(FBI::_domain_options = "y");
  orders.push_back(21);
  series.push_back(SingleSeriesTypes1D = "Legendre");

  domains.push_back(FBI::_domain_options = "z");
  orders.push_back(22);
  series.push_back(SingleSeriesTypes1D = "Legendre");

  Cartesian legendre3D(domains, orders, series);


  const unsigned int number_of_locations = 3;
  std::vector<Point> locations;
  std::array<Real, number_of_locations> standard_truth, orthogonal_truth;

  locations.push_back(Point(-0.14854612627465, 0.60364074055275, 0.76978431165674));
  standard_truth[0] =     1.32257143058688;
  orthogonal_truth[0] =  -2.33043696271172;

  locations.push_back(Point(0.93801805187856, 0.74175118177279, 0.74211345600994));
  standard_truth[1] =     3.68047786932034;
  orthogonal_truth[1] =  74.48747654183713;

  locations.push_back(Point(0.35423736896098, -0.83921049062126, -0.02231845586669));
  standard_truth[2] =     0.17515811557416;
  orthogonal_truth[2] = -14.48091828923379;


  for (std::size_t i = 0; i < number_of_locations; ++i)
  {
    legendre3D.setLocation(locations[i]);
    EXPECT_NEAR(legendre3D.getStandardSeriesSum(), standard_truth[i], tol);
    EXPECT_NEAR(legendre3D.getOrthonormalSeriesSum(), orthogonal_truth[i], tol);
  }
}

TEST(FunctionalExpansionsTest, functionalBasisInterface)
{
  std::vector<MooseEnum> domains;
  std::vector<std::size_t> orders;
  std::vector<MooseEnum> series;

  domains.push_back(FBI::_domain_options = "x");
  orders.push_back(4);
  series.push_back(SingleSeriesTypes1D = "Legendre");

  domains.push_back(FBI::_domain_options = "y");
  orders.push_back(5);
  series.push_back(SingleSeriesTypes1D = "Legendre");

  domains.push_back(FBI::_domain_options = "z");
  orders.push_back(3);
  series.push_back(SingleSeriesTypes1D = "Legendre");

  Cartesian legendre3D(domains, orders, series);

  const Point location(-0.38541903411291,
                        0.61369802505416,
                       -0.04539307255549);
  const Real truth = 0.26458908225718;
  FunctionalBasisInterface & interface = (FunctionalBasisInterface &)legendre3D;

  interface.setLocation(location);
  EXPECT_NEAR(interface.getStandardSeriesSum(), truth, tol);
}
