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
template <typename InType, typename OutType>
SobolCalculator<InType, OutType>::SobolCalculator(const libMesh::ParallelObject & other,
                                                  const std::string & name,
                                                  bool resample)
  : Calculator<std::vector<InType>, std::vector<OutType>>(other, name), _resample(resample)
{
}

template <typename InType, typename OutType>
void
SobolCalculator<InType, OutType>::initialize()
{
  _amat.resize(0, 0);
}

template <typename InType, typename OutType>
void
SobolCalculator<InType, OutType>::update(const InType & data)
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

template <typename InType, typename OutType>
void
SobolCalculator<InType, OutType>::finalize(bool is_distributed)
{
  // The size of the A matrix
  std::size_t s = _amat.n();

  if (is_distributed)
  {
    this->_communicator.max(s);
    if (_amat.n() == 0)
      _amat.resize(s, s);
    mooseAssert(_amat.n() == s, "Size of data updating SobolCalculator has changed.");
    this->_communicator.sum(_amat.get_values());
  }

  // Number of independent parameters
  const std::size_t n = (s - 2) / (_resample ? 2 : 1);

  // The data to output
  DenseMatrix<OutType> S(n, n);
  std::vector<OutType> ST(n);

  // First order
  {
    auto E = _amat(0, s - 1);
    auto V = _amat(s - 1, s - 1) - E;
    for (std::size_t i = 0; i < n; ++i)
    {
      auto U0 = _amat(i + 1, s - 1);
      if (_resample)
      {
        auto U1 = _amat(0, i + n + 1);
        S(i, i) = (((U0 + U1) / 2) - E) / V;
      }
      else
        S(i, i) = (U0 - E) / V;
    }
  }

  // Total-effect
  {
    auto E = _amat(0, s - 1);
    auto V = _amat(0, 0) - E;
    for (std::size_t i = 0; i < n; ++i)
    {
      auto U0 = _amat(0, i + 1);
      if (_resample)
      {
        auto U1 = _amat(i + n + 1, s - 1);
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
      auto E = _amat(i + 1, i + n + 1);
      for (std::size_t j = 0; j < i; ++j)
      {
        auto V = _amat(j + n + 1, j + n + 1) - E;
        auto U0 = _amat(i + 1, j + n + 1);
        auto U1 = _amat(j + 1, i + n + 1);
        auto Sc = (((U0 + U1) / 2) - E) / V;
        auto S2 = Sc - S(i, i) - S(j, j);
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

template class SobolCalculator<std::vector<Real>, Real>;
template class SobolCalculator<std::vector<std::vector<Real>>, std::vector<Real>>;
} // namespace
