//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BootstrapCalculators.h"

namespace StochasticTools
{

template <typename InType, typename OutType, template <typename, typename> class CalcType>
class VectorCalculator : public Calculator<std::vector<InType>, std::vector<OutType>>
{
public:
  using Calculator<std::vector<InType>, std::vector<OutType>>::Calculator;

protected:
  virtual void initialize() override;
  virtual void update(const InType & data) override;
  virtual void finalize(bool is_distributed) override;
  virtual std::vector<OutType> get() const override { return _values; }

private:
  std::vector<CalcType<InType, OutType>> _calcs;
  std::vector<OutType> _values;
};

template <typename InType, typename OutType, template <typename, typename> class CalcType>
void
VectorCalculator<InType, OutType, CalcType>::initialize()
{
  _calcs.clear();
  _values.clear();
}

template <typename InType, typename OutType, template <typename, typename> class CalcType>
void
VectorCalculator<InType, OutType, CalcType>::update(const InType & data)
{
  for (const auto & i : index_range(data))
  {
    if (i >= _calcs.size())
    {
      _calcs.emplace_back(*this, this->name() + "_" + std::to_string(i));
      _calcs.back().initializeCalculator();
    }
    _calcs[i].updateCalculator(data[i]);
  }
}

template <typename InType, typename OutType, template <typename, typename> class CalcType>
void
VectorCalculator<InType, OutType, CalcType>::finalize(bool is_distributed)
{
  // Need to make calculator objects if some data added
  if (is_distributed)
  {
    auto ncalc = _calcs.size();
    this->_communicator.max(ncalc);
    for (const auto & i : make_range(_calcs.size(), ncalc))
    {
      _calcs.emplace_back(*this, this->name() + "_" + std::to_string(i));
      _calcs.back().initializeCalculator();
    }
  }

  _values.reserve(_calcs.size());
  for (auto & cc : _calcs)
  {
    cc.finalizeCalculator(is_distributed);
    _values.push_back(cc.getValue());
  }
}

template <typename InType, typename OutType>
class Percentile<std::vector<InType>, std::vector<OutType>>
  : public BootstrapCalculator<std::vector<InType>, std::vector<OutType>>
{
public:
  using BootstrapCalculator<std::vector<InType>, std::vector<OutType>>::BootstrapCalculator;
  virtual std::vector<std::vector<OutType>> compute(const std::vector<InType> &,
                                                    const bool) override;
};

/*
 * Implement BCa method of Efron and Tibshirani (2003), Chapter 14.
 */
template <typename InType, typename OutType>
class BiasCorrectedAccelerated<std::vector<InType>, std::vector<OutType>>
  : public BootstrapCalculator<std::vector<InType>, std::vector<OutType>>
{
public:
  using BootstrapCalculator<std::vector<InType>, std::vector<OutType>>::BootstrapCalculator;
  virtual std::vector<std::vector<OutType>> compute(const std::vector<InType> &,
                                                    const bool) override;

private:
  std::vector<OutType> acceleration(const std::vector<InType> &, const bool);
};

template <typename InType, typename OutType>
struct CalculatorBuilder<std::vector<InType>, std::vector<OutType>>
{
  static std::unique_ptr<Calculator<std::vector<InType>, std::vector<OutType>>>
  build(const MooseEnumItem & item, const libMesh::ParallelObject & other);
};

template <typename InType, typename OutType>
struct BootstrapCalculatorBuilder<std::vector<InType>, std::vector<OutType>>
{
  static std::unique_ptr<BootstrapCalculator<std::vector<InType>, std::vector<OutType>>>
  build(const MooseEnum &,
        const libMesh::ParallelObject &,
        const std::vector<Real> &,
        unsigned int,
        unsigned int,
        StochasticTools::Calculator<std::vector<InType>, std::vector<OutType>> &);
};

} // namespace
