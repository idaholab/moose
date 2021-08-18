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
#include "MathUtils.h"
#include <vector>

class MooseEnumItem;

namespace StochasticTools
{

/*
 * Free function that returns the available statistics available to the Statistics object(s)
 */
MultiMooseEnum makeCalculatorEnum();

template <typename T1, typename T2>
class CalculatorValue;
template <typename InType, typename OutType>
using CValue = CalculatorValue<typename InType::value_type, OutType>;

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
  OutType compute(const InType &, bool);

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

  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _sum.get(); }

protected:
  dof_id_type _count;
  CValue<InType, OutType> _sum;
};

template <typename InType, typename OutType>
class Min : public Calculator<InType, OutType>
{
public:
  using Calculator<InType, OutType>::Calculator;

  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _min.get(); }

protected:
  CValue<InType, OutType> _min;
};

template <typename InType, typename OutType>
class Max : public Calculator<InType, OutType>
{
public:
  using Calculator<InType, OutType>::Calculator;

  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _max.get(); }

protected:
  CValue<InType, OutType> _max;
};

template <typename InType, typename OutType>
class Sum : public Calculator<InType, OutType>
{
public:
  using Calculator<InType, OutType>::Calculator;

  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _sum.get(); }

protected:
  CValue<InType, OutType> _sum;
};

template <typename InType, typename OutType>
class StdDev : public Calculator<InType, OutType>
{
public:
  using Calculator<InType, OutType>::Calculator;

  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _sum_of_square.get(); }

protected:
  dof_id_type _count;
  CValue<InType, OutType> _sum;
  CValue<InType, OutType> _sum_of_square;
};

template <typename InType, typename OutType>
class StdErr : public StdDev<InType, OutType>
{
public:
  using StdDev<InType, OutType>::StdDev;

  virtual void finalize(bool is_distributed) override;
};

template <typename InType, typename OutType>
class Ratio : public Calculator<InType, OutType>
{
public:
  using Calculator<InType, OutType>::Calculator;

  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _max.get(); }

protected:
  CValue<InType, OutType> _min;
  CValue<InType, OutType> _max;
};

template <typename InType, typename OutType>
class L2Norm : public Calculator<InType, OutType>
{
public:
  using Calculator<InType, OutType>::Calculator;

  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _l2_norm.get(); }

protected:
  CValue<InType, OutType> _l2_norm;
};

template <typename InType, typename OutType>
class Median : public Calculator<InType, OutType>
{
public:
  using Calculator<InType, OutType>::Calculator;

  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _median.get(); }

protected:
  std::vector<CValue<InType, OutType>> _storage;
  CValue<InType, OutType> _median;
};

/*
 * Free function for building a const Calculator object for use by Statistics object.
 *
 * Explicit instantiations in C file.
 */
template <typename InType = std::vector<Real>, typename OutType = Real>
std::unique_ptr<Calculator<InType, OutType>> makeCalculator(const MooseEnumItem & item,
                                                            const libMesh::ParallelObject & other);

/**
 * This class is used as a general interface for doing arithmetic needed for Calculators.
 * The idea is that instead of redefining each calculator method for new in-out data types,
 * one can just redefine these simple operations. The operations defined here are just
 * for scalar value types like Real and int.
 *
 * @tparam T1 the "in-value" type, this is the Intype::value_type in the Calculators
 * @tparam T2 the "out-value" type, this is underlying type of this class and is what
 *            is returned with the get() function. This is the OutType in the Calculators
 */
template <typename T1, typename T2>
class CalculatorValue
{
public:
  CalculatorValue() : _value() {}

  /// Returns a reference to the value computed
  const T2 & get() const { return _value; }

  /**
   * These are functions that do modifications on the value that do not depend
   * on the in-type @tparam T1.
   */
  ///@{
  /// Set the value to zero
  void zero() { _value = T2(); }
  /// Divide the value by a interger value
  void divide(dof_id_type num) { _value /= num; }
  /// Perfoming a exponential operation: _value = _value^p
  void pow(int p) { _value = MathUtils::pow(_value, p); }
  /// Square root of the value
  void sqrt() { _value = std::sqrt(_value); }
  /// Setting the value to the minimum of the data type
  void min() { _value = std::numeric_limits<T2>::min(); }
  /// Setting the value to the maximum of the data type
  void max() { _value = std::numeric_limits<T2>::max(); }
  ///@}

  /**
   * These are functions that modify the value with an in-type value
   */
  ///@{
  /// _value += a
  void add(const T1 & a) { _value += static_cast<T2>(a); }
  /// _value += a^p
  void addPow(const T1 & a, int p) { _value += MathUtils::pow(static_cast<T2>(a), p); }
  /// _value = min(_value, a)
  void min(const T1 & a) { _value = std::min(static_cast<T2>(a), _value); }
  /// _value = max(_value, a)
  void max(const T1 & a) { _value = std::max(static_cast<T2>(a), _value); }
  ///@}

  /**
   * These are overloaded operators that modify the value based on out-type values
   */
  ///@{
  CalculatorValue<T1, T2> & operator+=(const T2 & b)
  {
    _value += b;
    return *this;
  }
  CalculatorValue<T1, T2> & operator-=(const T2 & b)
  {
    _value -= b;
    return *this;
  }
  CalculatorValue<T1, T2> & operator/=(const T2 & b)
  {
    _value /= b;
    return *this;
  }
  bool less_than(const T2 & b) const { return _value < b; }
  ///@}

  /**
   * These are MPI operations
   */
  ///@{
  void sum(const libMesh::Parallel::Communicator & comm) { comm.sum(_value); }
  void min(const libMesh::Parallel::Communicator & comm) { comm.min(_value); }
  void max(const libMesh::Parallel::Communicator & comm) { comm.max(_value); }
  void broadcast(const libMesh::Parallel::Communicator & comm, processor_id_type root_id)
  {
    comm.broadcast(_value, root_id);
  }
  ///@}

private:
  T2 _value;
};

} // namespace

#define createCalculators(InType, OutType)                                                         \
  template class CalculatorValue<typename InType::value_type, OutType>;                            \
  template class Calculator<InType, OutType>;                                                      \
  template class Mean<InType, OutType>;                                                            \
  template class Max<InType, OutType>;                                                             \
  template class Min<InType, OutType>;                                                             \
  template class Sum<InType, OutType>;                                                             \
  template class StdDev<InType, OutType>;                                                          \
  template class StdErr<InType, OutType>;                                                          \
  template class Ratio<InType, OutType>;                                                           \
  template class L2Norm<InType, OutType>;                                                          \
  template class Median<InType, OutType>;                                                          \
  template std::unique_ptr<Calculator<InType, OutType>> makeCalculator<InType, OutType>(           \
      const MooseEnumItem &, const libMesh::ParallelObject &)
