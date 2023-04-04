//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "SimulatedAnnealingTests.h"
#include "MooseRandom.h"

// header for the global variables and the like
#include "SimulatedAnnealingAlgorithm.h"

void
ItinerantPeddler::distance(Real & objective,
                           const std::vector<Real> & /*rparams*/,
                           const std::vector<int> & iparams,
                           void * ctx)
{
  auto * itped = static_cast<ItinerantPeddler *>(ctx);

  if (iparams.size() != itped->_customer_locations.size())
    ::mooseError("Size of customer locations and visit order must be the same");

  objective = itped->_customer_locations[iparams[0]];
  for (unsigned int j = 1; j < iparams.size(); ++j)
    objective += std::abs(itped->_customer_locations[iparams[j]] -
                          itped->_customer_locations[iparams[j - 1]]);
}

ItinerantPeddler::ItinerantPeddler(const std::vector<Real> & cust_loc)
  : _customer_locations(cust_loc)
{
}

// traveling salesman test
TEST(SimulatedAnnealingTests, TravelingSalesman)
{
  MooseRandom::seed(1);

  // the customer locations, but note that loc[0] is home at x = 0
  unsigned int num_customers = 5;
  std::vector<Real> c_loc(num_customers);
  std::vector<int> init_visit_order(num_customers);
  for (unsigned int j = 0; j < num_customers; ++j)
  {
    c_loc[j] = MooseRandom::rand();
    init_visit_order[j] = j;
  }

  // the correct answer can be obtained by sorting
  std::vector<int> ref_visit_order = init_visit_order;
  std::sort(ref_visit_order.begin(), ref_visit_order.end(), [&](const int & a, const int & b) {
    return (c_loc[a] < c_loc[b]);
  });

  // for (unsigned int i = 0; i != ref_visit_order.size(); ++i)
  //  std::cout << ref_visit_order[i] << " " << c_loc[ref_visit_order[i]] << std::endl;

  // instance of the itinerant peddler object
  ItinerantPeddler itped(c_loc);
  SimulatedAnnealingAlgorithm alg;
  alg.setObjectiveRoutine(itped.distance, &itped);
  alg.maxIt() = 10000;
  alg.setInitialSolution({}, init_visit_order);
  alg.solve();
  // for (auto & p : alg.intSolution())
  //  std::cout << p << std::endl;

  for (unsigned int i = 0; i != ref_visit_order.size(); ++i)
    EXPECT_EQ(ref_visit_order[i], alg.intSolution()[i]);
}

void
comb_func(Real & objective,
          const std::vector<Real> & x,
          const std::vector<int> & /*iparams*/,
          void * /*ctx*/)
{
  objective = 10.0 * sin(x[0]) - 0.05 * (x[0] + 2.0) + pow(x[0] - 1.0, 2) + 20.0 +
              1.78E-6 * pow(x[1], 8) + 1.86E-5 * pow(x[1], 7) - 3.75E-4 * pow(x[1], 6) -
              3.61E-3 * pow(x[1], 5) + 2.55E-2 * pow(x[1], 4) + 2.06E-1 * pow(x[1], 3) -
              4.85E-1 * pow(x[1], 2) - 3.11E0 * x[1] + 1.38E0 + 20.E0;
}

TEST(SimulatedAnnealingTests, ContinuousMinimization)
{
  MooseRandom::seed(1);

  SimulatedAnnealingAlgorithm alg;
  alg.setObjectiveRoutine(comb_func, nullptr);
  alg.maxIt() = 20000;
  alg.setInitialSolution({1.0, 0.0}, {});
  alg.setLowerLimits({-1.2, 2.8});
  alg.setUpperLimits({-1.1, 2.9});
  alg.realNeighborSelection() = SimulatedAnnealingAlgorithm::BoxSampling;
  alg.solve();

  EXPECT_NEAR(alg.realSolution()[0], -1.1260671421437776E+00, 0.01);
  EXPECT_NEAR(alg.realSolution()[1], 2.8556531452530787E+00, 0.01);
  //std::cout << alg.realSolution()[0] << " " << alg.realSolution()[1] << std::endl;
}
