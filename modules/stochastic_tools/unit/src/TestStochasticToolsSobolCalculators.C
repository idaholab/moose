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
#include "BootstrapCalculators.h"
#include "StochasticToolsUtils.h"
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

std::vector<std::vector<Real>>
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

  return reshapeVector(data, K, false);
}

TEST(StochasticTools, Sobol_Saltelli2002)
{
  // This tests the C++ against the results from the TestSobolCalculator.py script, these values
  // also agree with what is reported by "Making best use of model evaluations to compute
  // sensitivity indices;" Saltelli, 2002.
  //
  // These calculations are based on random number generation and agree well for non-zero terms, the
  // absolute error is higher for the near-zero terms.
  {
    // Compute SOBOL vectors of g_function
    std::vector<Real> q_vector = {0, 0.5, 3, 9, 99, 99};
    std::vector<std::vector<Real>> data = sobolidx(q_vector, 100000);

    // Compute SOBOL indices
    Parallel::Communicator comm;
    ParallelObject po(comm);
    SobolCalculator<std::vector<Real>, Real> calc(po, "SOBOL", true);
    std::vector<Real> sobol = calc.compute(data, false);

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

  // Perform bootstrapping of Sobol indices
  {
    // Construct Sobol calculator
    Parallel::Communicator comm;
    ParallelObject po(comm);
    SobolCalculator<std::vector<Real>, Real> calc(po, "SOBOL", true);

    // Construct bootstrap calculator
    MooseEnum boot("percentile", "percentile");
    auto boot_calc = makeBootstrapCalculator(boot, po, {0.05, 0.95}, 10000, 1993, calc);

    // Compute SOBOL vectors of g_function
    std::vector<Real> q_vector = {0, 0.5, 3, 9, 99, 99};
    std::vector<std::vector<Real>> data = sobolidx(q_vector, 1024);

    // Compute mean sobol indices
    const std::vector<Real> sobol = calc.compute(data, false);
    // Compute confidence intervals
    const std::vector<std::vector<Real>> sobol_ci = boot_calc->compute(data, false);

    // S_1
    EXPECT_NEAR(sobol[0], 0.595124392, 1e-2);       // Mean
    EXPECT_NEAR(sobol_ci[0][0], 0.518401934, 1e-2); // 5% CI
    EXPECT_NEAR(sobol_ci[1][0], 0.694457878, 1e-2); // 95% CI
    // S_2
    EXPECT_NEAR(sobol[1], 0.251630423, 1e-2);       // Mean
    EXPECT_NEAR(sobol_ci[0][1], 0.212776416, 1e-2); // 5% CI
    EXPECT_NEAR(sobol_ci[1][1], 0.302403418, 1e-2); // 95% CI
    // S_3
    EXPECT_NEAR(sobol[2], 0.033508970, 1e-2);       // Mean
    EXPECT_NEAR(sobol_ci[0][2], 0.028092482, 1e-2); // 5% CI
    EXPECT_NEAR(sobol_ci[1][2], 0.040577359, 1e-2); // 95% CI
    // S_4
    EXPECT_NEAR(sobol[3], 0.005244906, 1e-2);       // Mean
    EXPECT_NEAR(sobol_ci[0][3], 0.004349295, 1e-2); // 5% CI
    EXPECT_NEAR(sobol_ci[1][3], 0.006388217, 1e-2); // 95% CI
    // S_5
    EXPECT_NEAR(sobol[4], 0.000055256, 1e-2);       // Mean
    EXPECT_NEAR(sobol_ci[0][4], 0.000046050, 1e-2); // 5% CI
    EXPECT_NEAR(sobol_ci[1][4], 0.000067359, 1e-2); // 95% CI
    // S_6
    EXPECT_NEAR(sobol[5], 0.000057292, 1e-2);       // Mean
    EXPECT_NEAR(sobol_ci[0][5], 0.000047778, 1e-2); // 5% CI
    EXPECT_NEAR(sobol_ci[1][5], 0.000069655, 1e-2); // 95% CI

    // ST_1
    EXPECT_NEAR(sobol[6], 0.719603333, 1e-2);       // Mean
    EXPECT_NEAR(sobol_ci[0][6], 0.664919501, 1e-2); // 5% CI
    EXPECT_NEAR(sobol_ci[1][6], 0.761403817, 1e-2); // 95% CI
    // ST_2
    EXPECT_NEAR(sobol[7], 0.391713495, 1e-1);       // Mean
    EXPECT_NEAR(sobol_ci[0][7], 0.284662598, 1e-1); // 5% CI
    EXPECT_NEAR(sobol_ci[1][7], 0.472968290, 1e-1); // 95% CI
    // ST_3
    EXPECT_NEAR(sobol[8], 0.053499226, 1e-1);        // Mean
    EXPECT_NEAR(sobol_ci[0][8], -0.088307896, 1e-1); // 5% CI
    EXPECT_NEAR(sobol_ci[1][8], 0.162827882, 1e-1);  // 95% CI
    // ST_4
    EXPECT_NEAR(sobol[9], 0.030497977, 1e-1);        // Mean
    EXPECT_NEAR(sobol_ci[0][9], -0.113473882, 1e-1); // 5% CI
    EXPECT_NEAR(sobol_ci[1][9], 0.140750264, 1e-1);  // 95% CI
    // ST_5
    EXPECT_NEAR(sobol[10], 0.022088362, 1e-1);        // Mean
    EXPECT_NEAR(sobol_ci[0][10], -0.123542952, 1e-1); // 5% CI
    EXPECT_NEAR(sobol_ci[1][10], 0.134539788, 1e-1);  // 95% CI
    // ST_6
    EXPECT_NEAR(sobol[11], 0.020756481, 1e-1);        // Mean
    EXPECT_NEAR(sobol_ci[0][11], -0.124872259, 1e-1); // 5% CI
    EXPECT_NEAR(sobol_ci[1][11], 0.133335504, 1e-1);  // 95% CI

    // S_21
    EXPECT_NEAR(sobol[12], 0.086623495, 1e-1);        // Mean
    EXPECT_NEAR(sobol_ci[0][12], -0.106884750, 1e-1); // 5% CI
    EXPECT_NEAR(sobol_ci[1][12], 0.292478615, 1e-1);  // 95% CI
    // S_31
    EXPECT_NEAR(sobol[13], 0.051499920, 1e-2);        // Mean
    EXPECT_NEAR(sobol_ci[0][13], -0.098917644, 1e-2); // 5% CI
    EXPECT_NEAR(sobol_ci[1][13], 0.208866009, 1e-2);  // 95% CI
    // S_32
    EXPECT_NEAR(sobol[14], -0.013129111, 1e-1);       // Mean
    EXPECT_NEAR(sobol_ci[0][14], -0.063644133, 1e-1); // 5% CI
    EXPECT_NEAR(sobol_ci[1][14], 0.034202843, 2e-1);  // 95% CI
    // S_41
    EXPECT_NEAR(sobol[15], 0.028748451, 1e-2);        // Mean
    EXPECT_NEAR(sobol_ci[0][15], -0.114335284, 1e-2); // 5% CI
    EXPECT_NEAR(sobol_ci[1][15], 0.180976397, 1e-2);  // 95% CI
    // S_42
    EXPECT_NEAR(sobol[16], -0.020848158, 1e-1);       // Mean
    EXPECT_NEAR(sobol_ci[0][16], -0.066521659, 1e-1); // 5% CI
    EXPECT_NEAR(sobol_ci[1][16], 0.021192582, 2e-1);  // 95% CI
    // S_43
    EXPECT_NEAR(sobol[17], 0.003324699, 1e-1);        // Mean
    EXPECT_NEAR(sobol_ci[0][17], -0.000940639, 1e-1); // 5% CI
    EXPECT_NEAR(sobol_ci[1][17], 0.008226253, 1e-1);  // 95% CI
    // S_51
    EXPECT_NEAR(sobol[18], 0.024066263, 1e-1);        // Mean
    EXPECT_NEAR(sobol_ci[0][18], -0.118856731, 1e-1); // 5% CI
    EXPECT_NEAR(sobol_ci[1][18], 0.173828068, 2e-1);  // 95% CI
    // S_52
    EXPECT_NEAR(sobol[19], -0.019734912, 1e-2);       // Mean
    EXPECT_NEAR(sobol_ci[0][19], -0.064960642, 1e-2); // 5% CI
    EXPECT_NEAR(sobol_ci[1][19], 0.021734891, 1e-2);  // 95% CI
    // S_53
    EXPECT_NEAR(sobol[20], 0.002819696, 1e-1);        // Mean
    EXPECT_NEAR(sobol_ci[0][20], -0.000486192, 1e-1); // 5% CI
    EXPECT_NEAR(sobol_ci[1][20], 0.006827348, 1e-1);  // 95% CI
    // S_54
    EXPECT_NEAR(sobol[21], 0.000065586, 1e-2);        // Mean
    EXPECT_NEAR(sobol_ci[0][21], -0.000178457, 1e-2); // 5% CI
    EXPECT_NEAR(sobol_ci[1][21], 0.000310071, 1e-2);  // 95% CI
    // S_61
    EXPECT_NEAR(sobol[22], 0.025259517, 1e-1);        // Mean
    EXPECT_NEAR(sobol_ci[0][22], -0.117947309, 2e-1); // 5% CI
    EXPECT_NEAR(sobol_ci[1][22], 0.175236100, 2e-1);  // 95% CI
    // S_62
    EXPECT_NEAR(sobol[23], -0.019851829, 1e-1);       // Mean
    EXPECT_NEAR(sobol_ci[0][23], -0.065106309, 1e-1); // 5% CI
    EXPECT_NEAR(sobol_ci[1][23], 0.021603718, 1e-1);  // 95% CI
    // S_63
    EXPECT_NEAR(sobol[24], 0.002859528, 1e-2);        // Mean
    EXPECT_NEAR(sobol_ci[0][24], -0.000404042, 1e-2); // 5% CI
    EXPECT_NEAR(sobol_ci[1][24], 0.006833422, 1e-2);  // 95% CI
    // S_64
    EXPECT_NEAR(sobol[25], 0.000184449, 1e-2);        // Mean
    EXPECT_NEAR(sobol_ci[0][25], -0.000042626, 1e-2); // 5% CI
    EXPECT_NEAR(sobol_ci[1][25], 0.000422993, 1e-2);  // 95% CI
    // S_65
    EXPECT_NEAR(sobol[26], -0.000003882, 1e-2);       // Mean
    EXPECT_NEAR(sobol_ci[0][26], -0.000012806, 1e-2); // 5% CI
    EXPECT_NEAR(sobol_ci[1][26], 0.000004494, 1e-2);  // 95% CI
  }
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
    std::vector<std::vector<Real>> data = sobolidx(q_vector, 1000000);
    SobolCalculator<std::vector<Real>, Real> calc(po, "SOBOL", true);
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
    std::vector<std::vector<Real>> data = sobolidx(q_vector, 1000000, false);
    SobolCalculator<std::vector<Real>, Real> calc(po, "SOBOL", false);
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
    std::vector<std::vector<Real>> data = sobolidx(q_vector, 1000000);
    SobolCalculator<std::vector<Real>, Real> calc(po, "SOBOL", true);
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
    std::vector<std::vector<Real>> data = sobolidx(q_vector, 1000000);
    SobolCalculator<std::vector<Real>, Real> calc(po, "SOBOL", true);
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
