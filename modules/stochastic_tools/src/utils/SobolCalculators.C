//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "SobolCalculators.h"
#include "MooseError.h"

#include <numeric>

namespace StochasticTools
{
SobolCalculator::SobolCalculator(const ParallelObject & other, std::size_t n, bool resample)
  : ParallelObject(other), _num_rows_per_matrix(n), _resample(resample)
{
}

std::vector<Real>
SobolCalculator::compute(const std::vector<Real> & data, bool is_distributed) const
{
  if (is_distributed)
    mooseError("Distributed VPP data is not yet supported for Sobol indice calculation.");

  // Set local convience variable
  const std::size_t n = _num_rows_per_matrix;

  // The size of the A matrix
  const std::size_t s = _resample ? 2 * n + 2 : n + 2;

  // The number of replicates per matrix
  const std::size_t K = data.size() / s;

  // These data structures correspond to the comments in the calculations of the various indices;
  // they are useful keeping around for debugging since the names match up with literature.
  /*
  std::vector<Real> a_0(data.begin() + 0*K, data.begin() + 1*K);
  std::vector<Real> a_K(data.begin() + (s-1)*K, data.begin() + s*K);
  std::vector<std::vector<Real>> a_i(n);
  std::vector<std::vector<Real>> a_ni(n);
  for (std::size_t i = 0; i < n; ++i)
  {
    a_i[i] = std::vector<Real>(data.begin() + (i+1)*K, data.begin() + (i+2)*K);
    a_ni[i] = std::vector<Real>(data.begin() + (i+n+1)*K, data.begin() + (i+n+2)*K);
  }
  */

  // Compute the necessary dot products of result vectors
  DenseMatrix<double> A(s, s);
  for (std::size_t c = 0; c < A.n(); ++c)
    for (std::size_t r = 0; r <= c; ++r)
    {
      const std::size_t row_offset = r * K;
      const std::size_t col_offset = c * K;
      A(r, c) = 1. / K *
                std::inner_product(data.begin() + row_offset,
                                   data.begin() + row_offset + K,
                                   data.begin() + col_offset,
                                   0.);
    }

  // The data to output
  DenseMatrix<Real> S(n, n);
  std::vector<Real> ST(n);

  // First order
  {
    // Real E = 1./K * std::inner_product(a_0.begin(), a_0.end(), a_K.begin(), 0.);
    // Real V = // 1./K * std::inner_product(a_K.begin(), a_K.end(), a_K.begin(), 0.) - E;
    Real E = A(0, s - 1);
    Real V = A(s - 1, s - 1) - E;
    for (std::size_t i = 0; i < n; ++i)
    {
      // Real U0 = 1./K * std::inner_product(a_i[i].begin(), a_i[i].end(), a_K.begin(), 0.);
      Real U0 = A(i + 1, s - 1);
      if (_resample)
      {
        // Real U1 = // 1./K * std::inner_product(a_K.begin(), a_K.end(), a_ni[i].begin(), 0.);
        Real U1 = A(0, i + n + 1);
        S(i, i) = (((U0 + U1) / 2) - E) / V;
      }
      else
        S(i, i) = (U0 - E) / V;
    }
  }

  // Total-effect
  {
    // Real E = 1./K * std::inner_product(a_0.begin(), a_0.end(), a_K.begin(), 0.);
    // Real V = 1./K * std::inner_product(a_0.begin(), a_0.end(), a_0.begin(), 0.) - E;
    Real E = A(0, s - 1);
    Real V = A(0, 0) - E;
    for (std::size_t i = 0; i < n; ++i)
    {
      // Real U0 =  1./K * std::inner_product(a_0.begin(), a_0.end(), a_i[i].begin(), 0.);
      Real U0 = A(0, i + 1);
      if (_resample)
      {
        // Real U1 = 1./K * std::inner_product(a_K.begin(), a_K.end(), a_ni[i].begin(), 0.);
        Real U1 = A(i + n + 1, s - 1);
        ST[i] = 1 - (((U0 + U1) / 2) - E) / V;
      }
      else
        ST[i] = 1 - (U0 - E) / V;
    }
  }

  // Second-order
  if (_resample)
  {
    for (std::size_t i = 0; i < n; ++i)
    {
      // Real E = 1./K * std::inner_product(a_i[i].begin(), a_i[i].end(), a_ni[i].begin(), 0.);
      Real E = A(i + 1, i + n + 1);
      for (std::size_t j = 0; j < i; ++j)
      {
        // clang-format off
        // Real V = 1./K*std::inner_product(a_ni[i].begin(), a_ni[i].end(), a_ni[i].begin(), 0.) - E;
        // Real U0 = 1./K * std::inner_product(a_i[i].begin(), a_i[i].end(), a_ni[j].begin(), 0.);
        // Real U1 = 1./K * std::inner_product(a_i[j].begin(), a_i[j].end(), a_ni[i].begin(), 0.);
        // clang-format on
        Real V = A(j + n + 1, j + n + 1) - E;
        Real U0 = A(i + 1, j + n + 1);
        Real U1 = A(j + 1, i + n + 1);
        Real Sc = (((U0 + U1) / 2) - E) / V;
        Real S2 = Sc - S(i, i) - S(j, j);
        S(j, i) = S2;
      }
    }
  }

  // Output the data
  std::vector<Real> output;
  if (_resample)
    output.reserve(n * (1 + n));
  else
    output.reserve(2 * n);

  // First-order
  for (std::size_t i = 0; i < n; ++i)
    output.push_back(S(i, i));

  // Total-effect
  for (std::size_t i = 0; i < n; ++i)
    output.push_back(ST[i]);

  // Second-order
  if (_resample)
  {
    for (std::size_t i = 0; i < n; ++i)
      for (std::size_t j = i + 1; j < n; ++j)
        output.push_back(S(i, j));
  }

  return output;
}
} // namespace
