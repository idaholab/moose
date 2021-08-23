//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "SobolCalculators.h"

namespace StochasticTools
{
SobolCalculator::SobolCalculator(const ParallelObject & other,
                                 const std::string & name,
                                 bool resample)
  : Calculator<std::vector<std::vector<Real>>, std::vector<Real>>(other, name), _resample(resample)
{
}

void
SobolCalculator::initialize()
{
  _amat.resize(0, 0);
}

void
SobolCalculator::update(const std::vector<Real> & data)
{
  if (_amat.n() == 0)
  {
    _amat.resize(data.size(), data.size());
    if (_amat.n() < 2 || (_resample && _amat.n() % 2 != 0))
      mooseError("Size of input data is inconsistent with Sobol resampling scheme.");
  }
  mooseAssert(_amat.n() == data.size(), "Size of data updating SobolCalculator has changed.");

  for (const auto & c : index_range(data))
    for (const auto & r : make_range(c + 1))
      _amat(r, c) += data[r] * data[c];
}

void
SobolCalculator::finalize(bool is_distributed)
{
  if (is_distributed)
    this->_communicator.sum(_amat.get_values());

  // The size of the A matrix
  const std::size_t s = _amat.n();

  // Number of independent parameters
  const std::size_t n = (s - 2) / (_resample ? 2 : 1);

  // The data to output
  DenseMatrix<Real> S(n, n);
  std::vector<Real> ST(n);

  // First order
  {
    Real E = _amat(0, s - 1);
    Real V = _amat(s - 1, s - 1) - E;
    for (std::size_t i = 0; i < n; ++i)
    {
      Real U0 = _amat(i + 1, s - 1);
      if (_resample)
      {
        Real U1 = _amat(0, i + n + 1);
        S(i, i) = (((U0 + U1) / 2) - E) / V;
      }
      else
        S(i, i) = (U0 - E) / V;
    }
  }

  // Total-effect
  {
    Real E = _amat(0, s - 1);
    Real V = _amat(0, 0) - E;
    for (std::size_t i = 0; i < n; ++i)
    {
      Real U0 = _amat(0, i + 1);
      if (_resample)
      {
        Real U1 = _amat(i + n + 1, s - 1);
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
      Real E = _amat(i + 1, i + n + 1);
      for (std::size_t j = 0; j < i; ++j)
      {
        Real V = _amat(j + n + 1, j + n + 1) - E;
        Real U0 = _amat(i + 1, j + n + 1);
        Real U1 = _amat(j + 1, i + n + 1);
        Real Sc = (((U0 + U1) / 2) - E) / V;
        Real S2 = Sc - S(i, i) - S(j, j);
        S(j, i) = S2;
      }
    }
  }

  // Output the data
  _sobol.clear();
  if (_resample)
    _sobol.reserve(n * (1 + n));
  else
    _sobol.reserve(2 * n);

  // First-order
  for (std::size_t i = 0; i < n; ++i)
    _sobol.push_back(S(i, i));

  // Total-effect
  for (std::size_t i = 0; i < n; ++i)
    _sobol.push_back(ST[i]);

  // Second-order
  if (_resample)
  {
    for (std::size_t i = 0; i < n; ++i)
      for (std::size_t j = i + 1; j < n; ++j)
        _sobol.push_back(S(i, j));
  }
}

} // namespace
