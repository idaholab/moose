//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once
#include "Calculators.h"

#include "libmesh/dense_matrix.h"

namespace StochasticTools
{
/**
 * Calculator for computing Sobol sensitivity indices according to the paper by Saltelli (2002)
 * https://doi.org/10.1016/S0010-4655(02)00280-1
 *
 * The data provided is stacked vectors provided by the SobolSampler. Example use of this object
 * is also available in the stochastic_tools unit testing.
 */
template <typename InType, typename OutType>
class SobolCalculator : public Calculator<std::vector<InType>, std::vector<OutType>>
{
public:
  SobolCalculator(const libMesh::ParallelObject & other, const std::string & name, bool resample);

protected:
  virtual void initialize() override;
  virtual void update(const InType & data) override;
  virtual void finalize(bool is_distributed) override;
  virtual std::vector<OutType> get() const override { return _sobol; }

private:
  /// Set to true if the resampling matrix exists for computing second-order indices
  const bool _resample;
  /// Matrix containing dot products of data
  DenseMatrix<OutType> _amat;
  /// The returned sobol indices
  std::vector<OutType> _sobol;
};

template <typename InType, typename OutType>
class SobolCalculator<std::vector<InType>, std::vector<OutType>>
  : public Calculator<std::vector<std::vector<InType>>, std::vector<std::vector<OutType>>>
{
public:
  SobolCalculator(const libMesh::ParallelObject & other, const std::string & name, bool resample);

protected:
  virtual void initialize() override;
  virtual void update(const std::vector<InType> & data) override;
  virtual void finalize(bool is_distributed) override;
  virtual std::vector<std::vector<OutType>> get() const override { return _values; }

private:
  /// Set to true if the resampling matrix exists for computing second-order indices
  const bool _resample;
  /// Sobol calculator for each entry in vector data
  std::vector<SobolCalculator<InType, OutType>> _sobol_calcs;
  /// Sobol indices
  std::vector<std::vector<OutType>> _values;
};

template <typename InType, typename OutType>
SobolCalculator<std::vector<InType>, std::vector<OutType>>::SobolCalculator(
    const ParallelObject & other, const std::string & name, bool resample)
  : Calculator<std::vector<std::vector<InType>>, std::vector<std::vector<OutType>>>(other, name),
    _resample(resample)
{
}

template <typename InType, typename OutType>
void
SobolCalculator<std::vector<InType>, std::vector<OutType>>::initialize()
{
  _sobol_calcs.clear();
  _values.clear();
}

template <typename InType, typename OutType>
void
SobolCalculator<std::vector<InType>, std::vector<OutType>>::update(const std::vector<InType> & data)
{
  // Need to transpose data
  std::vector<InType> data_update;
  for (const auto & d : index_range(data))
    for (const auto & i : index_range(data[d]))
    {
      if (data_update.size() <= i)
        data_update.emplace_back(data.size());
      data_update[i][d] = data[d][i];
    }

  // Build calculators and update
  for (const auto & i : index_range(data_update))
  {
    if (_sobol_calcs.size() <= i)
    {
      _sobol_calcs.emplace_back(*this, "SOBOL_" + std::to_string(i), _resample);
      _sobol_calcs[i].initializeCalculator();
    }
    _sobol_calcs[i].updateCalculator(data_update[i]);
  }
}

template <typename InType, typename OutType>
void
SobolCalculator<std::vector<InType>, std::vector<OutType>>::finalize(bool is_distributed)
{
  // Need to create calculators here no data was added
  if (is_distributed)
  {
    auto ncalc = _sobol_calcs.size();
    this->_communicator.max(ncalc);
    for (const auto & i : make_range(_sobol_calcs.size(), ncalc))
    {
      _sobol_calcs.emplace_back(*this, "SOBOL_" + std::to_string(i), _resample);
      _sobol_calcs[i].initializeCalculator();
    }
  }

  for (const auto & i : index_range(_sobol_calcs))
  {
    _sobol_calcs[i].finalizeCalculator(is_distributed);
    const auto val = _sobol_calcs[i].getValue();

    for (const auto & ind : index_range(val))
    {
      if (_values.size() <= ind)
        _values.emplace_back(_sobol_calcs.size());
      _values[ind][i] = val[ind];
    }
  }
}

} // namespace
