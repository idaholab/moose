//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "MultiMooseEnum.h"
#include <vector>

class MooseEnumItem;

namespace StochasticTools
{

/*
 * Free function that returns the available statistics available to the Statistics object(s)
 */
MultiMooseEnum makeCalculatorEnum();

/* Base class for computing statistics (e.g., mean, min) for use with Statistics object
 *
 * The purpose of these objects are to provide an API for computing statistics in serial or parallel
 * without any state. This allows future statistics to be quickly added and for each statistic
 * to be used with the BootstrapCalculator for computing bootstrap statistics such as confidence
 * level intervals.
 *
 * The Calculator objects are created as const objects by the Statistics object via
 * the makeCalculator function.
 *
 * To create new Calculator objects first create the Calculator class and then update the
 * above free functions above.
 *
 * Explicit instantiations are generated in the C file.
 */
template <typename InType, typename OutType>
class Calculator : public libMesh::ParallelObject
{
public:
  Calculator(const libMesh::ParallelObject & other, const std::string & name)
    : libMesh::ParallelObject(other), _name(name)
  {
  }

  virtual ~Calculator() = default;
  virtual OutType compute(const InType &, bool) const = 0;

  virtual void initialize() {}
  virtual void update(const typename InType::value_type &) = 0;
  virtual void finalize(bool) {}
  virtual OutType get() const = 0;

  const std::string & name() const { return _name; }

private:
  const std::string _name;
};

template <typename InType, typename OutType>
class Mean : public Calculator<InType, OutType>
{
public:
  using Calculator<InType, OutType>::Calculator;
  virtual OutType compute(const InType &, bool) const override;

  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _count == 0 ? OutType() : _sum / _count; }

protected:
  dof_id_type _count;
  OutType _sum;
};

template <typename InType, typename OutType>
class Min : public Calculator<InType, OutType>
{
public:
  using Calculator<InType, OutType>::Calculator;
  virtual OutType compute(const InType &, bool) const override;

  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _min; };

protected:
  OutType _min;
};

template <typename InType, typename OutType>
class Max : public Calculator<InType, OutType>
{
public:
  using Calculator<InType, OutType>::Calculator;
  virtual OutType compute(const InType &, bool) const override;

  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _max; };

protected:
  OutType _max;
};

template <typename InType, typename OutType>
class Sum : public Calculator<InType, OutType>
{
public:
  using Calculator<InType, OutType>::Calculator;
  virtual OutType compute(const InType &, bool) const override;

  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _sum; };

protected:
  OutType _sum;
};

template <typename InType, typename OutType>
class StdDev : public Calculator<InType, OutType>
{
public:
  using Calculator<InType, OutType>::Calculator;
  virtual OutType compute(const InType &, bool) const override;

  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _count <= 1 ? OutType() : std::sqrt((_sum_of_square - _sum * _sum / _count) / (_count - 1)); };

protected:
  dof_id_type _count;
  OutType _sum;
  OutType _sum_of_square;
};

template <typename InType, typename OutType>
class StdErr : public StdDev<InType, OutType>
{
public:
  using StdDev<InType, OutType>::StdDev;
  virtual OutType compute(const InType &, bool) const override;

  virtual OutType get() const override { return this->_count == 0 ? OutType() : StdDev<InType, OutType>::get() / std::sqrt(this->_count); };
};

template <typename InType, typename OutType>
class Ratio : public Calculator<InType, OutType>
{
public:
  using Calculator<InType, OutType>::Calculator;
  virtual OutType compute(const InType &, bool) const override;

  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _max / _min; };

protected:
  OutType _min;
  OutType _max;
};

template <typename InType, typename OutType>
class L2Norm : public Calculator<InType, OutType>
{
public:
  using Calculator<InType, OutType>::Calculator;
  virtual OutType compute(const InType &, bool) const override;

  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _l2_norm; };

protected:
  OutType _l2_norm;
};

/*
 * Free function for building a const Calculator object for use by Statistics object.
 *
 * Explicit instantiations in C file.
 */
template <typename InType = std::vector<Real>, typename OutType = Real>
std::unique_ptr<Calculator<InType, OutType>>
makeCalculator(const MooseEnumItem & item, const libMesh::ParallelObject & other);

} // namespace
