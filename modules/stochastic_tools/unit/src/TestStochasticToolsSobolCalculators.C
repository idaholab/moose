//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include <vector>

#include "gtest/gtest.h"
#include "SobolCalculators.h"
#include "MooseRandom.h"
#include "libmesh/communicator.h"
#include "libmesh/parallel_object.h"

using namespace StochasticTools;

Real
g_function(const std::vector<Real> & q_vector, const DenseMatrix<Real> & data, std::size_t row)
{
  Real y = 1;
  for (std::size_t c = 0; c < data.n(); ++c)
    y *= (std::abs(4 * data(row, c) - 2) + q_vector[c]) / (1 + q_vector[c]);
  return y;
}

void
evaluate(const std::vector<Real> & q_vector,
         const DenseMatrix<Real> & data,
         std::vector<Real> & out)
{
  for (std::size_t i = 0; i < data.m(); ++i)
    out.push_back(g_function(q_vector, data, i));
}

Real
model_association_index(const std::vector<Real> & data, std::size_t count)
{
  return 1 - std::accumulate(data.begin(), data.begin() + count, 0.);
}

std::vector<Real>
sobolidx(const std::vector<Real> & q_vector, std::size_t K, bool resample = true)
{
  MooseRandom generator;
  generator.seed(0, 1980);
  generator.seed(1, 1949);

  const std::size_t n = q_vector.size();

  DenseMatrix<Real> M1(K, n); // "sample" matrix
  DenseMatrix<Real> M2(K, n); // "re-sample" matrix

  for (std::size_t r = 0; r < K; ++r)
    for (std::size_t c = 0; c < n; ++c)
    {
      M1(r, c) = generator.rand(0);
      M2(r, c) = generator.rand(1);
    }

  std::vector<DenseMatrix<Real>> Ni(n, DenseMatrix<Real>(K, n));
  std::vector<DenseMatrix<Real>> Nni(n, DenseMatrix<Real>(K, n));
  for (std::size_t j = 0; j < n; ++j)
    for (std::size_t r = 0; r < K; ++r)
      for (std::size_t c = 0; c < n; ++c)
      {
        Ni[j](r, c) = j == c ? M1(r, c) : M2(r, c);
        Nni[j](r, c) = j == c ? M2(r, c) : M1(r, c);
      }

  // Output data
  std::vector<Real> data;

  // a0
  evaluate(q_vector, M2, data);

  // ai
  for (std::size_t i = 0; i < n; ++i)
    evaluate(q_vector, Ni[i], data);

  // ani
  if (resample)
    for (std::size_t i = 0; i < n; ++i)
      evaluate(q_vector, Nni[i], data);

  // aK
  evaluate(q_vector, M1, data);

  return data;
}

TEST(StochasticTools, Sobol_Saltelli2002)
{
  // Compute SOBOL vectors of g_function
  std::vector<Real> q_vector = {0, 0.5, 3, 9, 99, 99};
  std::vector<Real> data = sobolidx(q_vector, 100000);

  // Compute SOBOL indices
  Parallel::Communicator comm;
  ParallelObject po(comm);
  const SobolCalculator calc(po, q_vector.size(), true);
  std::vector<Real> sobol = calc.compute(data, false);

  // This tests the C++ against the results from the TestSobolCalculator.py script, these values
  // also agree with what is reported by "Making best use of model evaluations to compute
  // sensitivity indices;" Saltelli, 2002.
  //
  // These calculations are based on random number generation and agree well for non-zero terms, the
  // absolute error is higher for the near-zero terms.

  // First-order
  EXPECT_NEAR(sobol[0], 0.585817378, 1e-2); // S_1
  EXPECT_NEAR(sobol[1], 0.261775886, 1e-2); // S_2
  EXPECT_NEAR(sobol[2], 0.036806879, 1e-2); // S_3
  EXPECT_NEAR(sobol[3], 0.005850546, 1e-2); // S_4
  EXPECT_NEAR(sobol[4], 0.000059171, 1e-2); // S_5
  EXPECT_NEAR(sobol[5], 0.000058227, 1e-2); // S_6

  // Total-effect
  EXPECT_NEAR(sobol[6], 0.686102012, 1e-2);   // ST_1
  EXPECT_NEAR(sobol[7], 0.349098045, 1e-2);   // ST_2
  EXPECT_NEAR(sobol[8], 0.044801021, 1e-2);   // ST_3
  EXPECT_NEAR(sobol[9], 0.002512902, 1e-2);   // ST_4
  EXPECT_NEAR(sobol[10], -0.007565606, 1e-2); // ST_5
  EXPECT_NEAR(sobol[11], -0.007483272, 1e-2); // ST_6

  // Second-order
  EXPECT_NEAR(sobol[12], 0.081273974, 1e-2);  // S_21;
  EXPECT_NEAR(sobol[13], 0.006920649, 1e-2);  // S_31;
  EXPECT_NEAR(sobol[14], 0.006375308, 1e-1);  // S_32;
  EXPECT_NEAR(sobol[15], -0.004536444, 1e-1); // S_41;
  EXPECT_NEAR(sobol[16], -0.001670717, 1e-2); // S_42;
  EXPECT_NEAR(sobol[17], -0.000131402, 1e-2); // S_43;
  EXPECT_NEAR(sobol[18], -0.006650540, 1e-2); // S_51;
  EXPECT_NEAR(sobol[19], -0.001642939, 1e-2); // S_52;
  EXPECT_NEAR(sobol[20], -0.000117414, 1e-2); // S_53;
  EXPECT_NEAR(sobol[21], 0.000029383, 1e-2);  // S_54;
  EXPECT_NEAR(sobol[22], -0.006641290, 1e-2); // S_61;
  EXPECT_NEAR(sobol[23], -0.001713718, 1e-2); // S_62;
  EXPECT_NEAR(sobol[24], -0.000123219, 1e-2); // S_63;
  EXPECT_NEAR(sobol[25], 0.000027036, 1e-2);  // S_64;
  EXPECT_NEAR(sobol[26], -0.000000386, 1e-2); // S_65;
}

TEST(StochasticTools, Sobol_Analytical)
{
  // Computes results to compare to analytic values presented by Saltelly and Sobol (1995):
  // "About the use of rank transformation in sensitivity analysis of model output."

  // Compute SOBOL indices
  Parallel::Communicator comm;
  ParallelObject po(comm);

  {
    // Compute SOBOL vectors of g_function
    std::vector<Real> q_vector = {0, 0, 0, 0};
    std::vector<Real> data = sobolidx(q_vector, 1000000);
    const SobolCalculator calc(po, q_vector.size(), true);
    std::vector<Real> sobol = calc.compute(data, false);

    // p. 235
    EXPECT_EQ(sobol.size(), (std::size_t)14);
    EXPECT_NEAR(sobol[0], 0.154, 1e-2);
    EXPECT_NEAR(sobol[1], 0.154, 1e-2);
    EXPECT_NEAR(sobol[2], 0.154, 1e-2);
    EXPECT_NEAR(sobol[3], 0.154, 1e-2);

    EXPECT_NEAR(sobol[4], 0.360, 1e-2);
    EXPECT_NEAR(sobol[5], 0.360, 1e-2);
    EXPECT_NEAR(sobol[6], 0.360, 1e-2);
    EXPECT_NEAR(sobol[7], 0.360, 1e-2);

    EXPECT_NEAR(sobol[8], 0.051, 1e-2);
    EXPECT_NEAR(sobol[9], 0.051, 1e-2);
    EXPECT_NEAR(sobol[10], 0.051, 1e-2);
    EXPECT_NEAR(sobol[11], 0.051, 1e-2);
    EXPECT_NEAR(sobol[12], 0.051, 1e-2);
    EXPECT_NEAR(sobol[13], 0.051, 1e-2);
  }

  {
    // Compute SOBOL vectors of g_function
    std::vector<Real> q_vector = {0, 0, 0, 0};
    std::vector<Real> data = sobolidx(q_vector, 1000000, false);
    const SobolCalculator calc(po, q_vector.size(), false);
    std::vector<Real> sobol = calc.compute(data, false);

    // p. 235
    EXPECT_EQ(sobol.size(), (std::size_t)8);
    EXPECT_NEAR(sobol[0], 0.154, 1e-2);
    EXPECT_NEAR(sobol[1], 0.154, 1e-2);
    EXPECT_NEAR(sobol[2], 0.154, 1e-2);
    EXPECT_NEAR(sobol[3], 0.154, 1e-2);

    EXPECT_NEAR(sobol[4], 0.360, 1e-2);
    EXPECT_NEAR(sobol[5], 0.360, 1e-2);
    EXPECT_NEAR(sobol[6], 0.360, 1e-2);
    EXPECT_NEAR(sobol[7], 0.360, 1e-2);
  }

  {
    std::vector<Real> q_vector = {0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<Real> data = sobolidx(q_vector, 1000000);
    const SobolCalculator calc(po, q_vector.size(), true);
    std::vector<Real> sobol = calc.compute(data, false);

    // p. 235
    EXPECT_EQ(sobol.size(), (std::size_t)44);
    EXPECT_NEAR(sobol[0], 0.037, 1e-2);
    EXPECT_NEAR(sobol[1], 0.037, 1e-2);
    EXPECT_NEAR(sobol[2], 0.037, 1e-2);
    EXPECT_NEAR(sobol[3], 0.037, 1e-2);
    EXPECT_NEAR(sobol[4], 0.037, 1e-2);
    EXPECT_NEAR(sobol[5], 0.037, 1e-2);
    EXPECT_NEAR(sobol[6], 0.037, 1e-2);
    EXPECT_NEAR(sobol[7], 0.037, 1e-2);

    EXPECT_NEAR(sobol[16], 0.012, 1e-2);
    EXPECT_NEAR(sobol[17], 0.012, 1e-2);
    EXPECT_NEAR(sobol[18], 0.012, 1e-2);
    EXPECT_NEAR(sobol[19], 0.012, 1e-2);
    EXPECT_NEAR(sobol[20], 0.012, 1e-2);
    EXPECT_NEAR(sobol[21], 0.012, 1e-2);
    EXPECT_NEAR(sobol[22], 0.012, 1e-2);
    EXPECT_NEAR(sobol[23], 0.012, 1e-2);
    EXPECT_NEAR(sobol[24], 0.012, 1e-2);
    EXPECT_NEAR(sobol[25], 0.012, 1e-2);
    EXPECT_NEAR(sobol[26], 0.012, 1e-2);
    EXPECT_NEAR(sobol[27], 0.012, 1e-2);
    EXPECT_NEAR(sobol[28], 0.012, 1e-2);
    EXPECT_NEAR(sobol[29], 0.012, 1e-2);
    EXPECT_NEAR(sobol[30], 0.012, 1e-2);
    EXPECT_NEAR(sobol[31], 0.012, 1e-2);
    EXPECT_NEAR(sobol[32], 0.012, 1e-2);
    EXPECT_NEAR(sobol[33], 0.012, 1e-2);
    EXPECT_NEAR(sobol[34], 0.012, 1e-2);
    EXPECT_NEAR(sobol[35], 0.012, 1e-2);
    EXPECT_NEAR(sobol[36], 0.012, 1e-2);
    EXPECT_NEAR(sobol[37], 0.012, 1e-2);
    EXPECT_NEAR(sobol[38], 0.012, 1e-2);
    EXPECT_NEAR(sobol[39], 0.012, 1e-2);
    EXPECT_NEAR(sobol[40], 0.012, 1e-2);
    EXPECT_NEAR(sobol[41], 0.012, 1e-2);
    EXPECT_NEAR(sobol[42], 0.012, 1e-2);
    EXPECT_NEAR(sobol[43], 0.012, 1e-2);
  }

  {
    std::vector<Real> q_vector = {0, 0, 3, 9, 9, 9, 9, 9};
    std::vector<Real> data = sobolidx(q_vector, 1000000);
    const SobolCalculator calc(po, q_vector.size(), true);
    std::vector<Real> sobol = calc.compute(data, false);

    // p. 235
    EXPECT_EQ(sobol.size(), (std::size_t)44);
    EXPECT_NEAR(sobol[8], 0.550, 1e-2);
    EXPECT_NEAR(sobol[9], 0.550, 1e-2);
    EXPECT_NEAR(sobol[10], 0.050, 1e-2);
    EXPECT_NEAR(sobol[11], 0.007, 1e-2);
    EXPECT_NEAR(sobol[12], 0.007, 1e-2);
    EXPECT_NEAR(sobol[13], 0.007, 1e-2);
    EXPECT_NEAR(sobol[14], 0.007, 1e-2);
    EXPECT_NEAR(sobol[15], 0.007, 1e-2);
  }
}
