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

#include "libmesh/parallel.h"

#include <vector>
#include <memory>
#include <numeric>

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
 * The calculators are evaluating using the following sequence:
 * \code
 *     auto calculator = makeCalculator<InType, OutType>(*this, type);
 *     calculator->initializeCalculator();
 *     for (const typename InType::value_type & val : data)
 *       calculator->updateCalculator(data);
 *     calculator->finalizeCalculator(is_distributed);
 *     OutType stat = calculator->getValue();
 * \endcode
 * The base Calculator class does this automatically with an input vector of data:
 * \code
 *     auto calculator = makeCalculator<InType, OutType>(*this, type);
 *     OutType stat = calculator->compute(data, is_distributed);
 * \endcode
 * The base class has state checks to make sure these functions are called
 * in the correct order.
 *
 * To create new Calculator objects first create the Calculator class and then update the
 * the initialize, update, finalize, and get virtual functions.
 *
 * Explicit instantiations are generated in the C file.
 */
template <typename InType, typename OutType>
class Calculator : public libMesh::ParallelObject
{
public:
  Calculator(const libMesh::ParallelObject & other, const std::string & name)
    : libMesh::ParallelObject(other), _name(name), _state(CalculatorState::NONE)
  {
  }

  virtual ~Calculator() = default;
  /**
   * Evaluate the calculator on the full vector of data. This is basically a convenient
   * wrapper around initializeCalculator, updateCalculator, finalizeCalculator, and getvalue.
   */
  OutType compute(const InType &, bool);

  /**
   * Public function that must be called before updateCalculator and finalizeCalculator.
   * Sets _state to INITIALIZED
   */
  void initializeCalculator();
  /**
   * Public function to update calculator with a piece of data.
   * _state mush be INITIALIZED
   */
  void updateCalculator(const typename InType::value_type &);
  /**
   * Public function to finalize the resulting calculator value.
   * _state must be INITLIALIZED
   * Sets _state to FINALIZED
   */
  void finalizeCalculator(bool);
  /**
   * Public function to return the calculated value
   * _state must be FINALIZED
   */
  OutType getValue() const;

  const std::string & name() const { return _name; }

protected:
  /**
   * This function is used to reset the calculator to its initial state and prepare it
   * for another evaluation. This usually involves clearing class members.
   */
  virtual void initialize() = 0;
  /**
   * Updating the calculator with a piece of data. Sometimes some clever arithmetic
   * is required to avoid storing data.
   */
  virtual void update(const typename InType::value_type &) = 0;
  /**
   * This is used to compute the resulting calculator value by performing necessary
   * arithmetic and parallel communication. This only called once after all the input
   * data is entered through update.
   */
  virtual void finalize(bool) = 0;
  /**
   * Returns the resulting calculator value. It is important to not modify member
   * data here so the calculator can retain its state.
   */
  virtual OutType get() const = 0;

private:
  enum CalculatorState
  {
    NONE,
    INITIALIZED,
    FINALIZED
  };

  const std::string _name;
  CalculatorState _state;
};

template <typename InType, typename OutType>
class Mean : public Calculator<InType, OutType>
{
public:
  using Calculator<InType, OutType>::Calculator;

protected:
  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _sum; }

  dof_id_type _count;
  OutType _sum;
};

template <typename InType, typename OutType>
class MeanAbsoluteValue : public Mean<InType, OutType>
{
public:
  using Mean<InType, OutType>::Mean;

protected:
  virtual void update(const typename InType::value_type & val) override;
};

template <typename InType, typename OutType>
class Ratio : public Calculator<InType, OutType>
{
public:
  using Calculator<InType, OutType>::Calculator;

protected:
  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _max / _min; }

  OutType _min;
  OutType _max;
};

template <typename InType, typename OutType>
class Min : public Ratio<InType, OutType>
{
public:
  using Ratio<InType, OutType>::Ratio;

protected:
  virtual OutType get() const override { return this->_min; }
};

template <typename InType, typename OutType>
class Max : public Ratio<InType, OutType>
{
public:
  using Ratio<InType, OutType>::Ratio;

protected:
  virtual OutType get() const override { return this->_max; }
};

template <typename InType, typename OutType>
class Sum : public Mean<InType, OutType>
{
public:
  using Mean<InType, OutType>::Mean;

protected:
  virtual void finalize(bool is_distributed) override;
};

template <typename InType, typename OutType>
class StdDev : public Calculator<InType, OutType>
{
public:
  using Calculator<InType, OutType>::Calculator;

protected:
  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _sum_of_square; }

  dof_id_type _count;
  OutType _sum;
  OutType _sum_of_square;
};

template <typename InType, typename OutType>
class StdErr : public StdDev<InType, OutType>
{
public:
  using StdDev<InType, OutType>::StdDev;

protected:
  virtual void finalize(bool is_distributed) override;
};

template <typename InType, typename OutType>
class L2Norm : public Calculator<InType, OutType>
{
public:
  using Calculator<InType, OutType>::Calculator;

protected:
  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _l2_norm; }

  OutType _l2_norm;
};

template <typename InType, typename OutType>
class Median : public Calculator<InType, OutType>
{
public:
  using Calculator<InType, OutType>::Calculator;

protected:
  virtual void initialize() override;
  virtual void update(const typename InType::value_type & val) override;
  virtual void finalize(bool is_distributed) override;
  virtual OutType get() const override { return _median; }

  std::vector<OutType> _storage;
  OutType _median;
};

/*
 * Free function for building a const Calculator object for use by Statistics object.
 *
 * Explicit instantiations in C file.
 */
template <typename InType = std::vector<Real>, typename OutType = Real>
std::unique_ptr<Calculator<InType, OutType>> makeCalculator(const MooseEnumItem & item,
                                                            const libMesh::ParallelObject & other);

/*
 * Simple struct that makeCalculator wraps around, this is so building calculators
 * can be partially specialized.
 */
template <typename InType, typename OutType>
struct CalculatorBuilder
{
  static std::unique_ptr<Calculator<InType, OutType>> build(const MooseEnumItem & item,
                                                            const libMesh::ParallelObject & other);
};

template <typename InType, typename OutType>
OutType
Calculator<InType, OutType>::compute(const InType & data, bool is_distributed)
{
  initializeCalculator();
  for (const auto & val : data)
    updateCalculator(val);
  finalizeCalculator(is_distributed);
  return getValue();
}

template <typename InType, typename OutType>
void
Calculator<InType, OutType>::initializeCalculator()
{
  initialize();
  _state = CalculatorState::INITIALIZED;
}

template <typename InType, typename OutType>
void
Calculator<InType, OutType>::updateCalculator(const typename InType::value_type & val)
{
  mooseAssert(_state == CalculatorState::INITIALIZED, "Calculator is in wrong state.");
  update(val);
}

template <typename InType, typename OutType>
void
Calculator<InType, OutType>::finalizeCalculator(bool is_distributed)
{
  if (_state != CalculatorState::INITIALIZED)
    ::mooseError("Calculator is in wrong state.");
  finalize(is_distributed);
  _state = CalculatorState::FINALIZED;
}

template <typename InType, typename OutType>
OutType
Calculator<InType, OutType>::getValue() const
{
  if (_state != CalculatorState::FINALIZED)
    ::mooseError("Calculator is in wrong state.");
  return get();
}

// makeCalculator //////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
std::unique_ptr<Calculator<InType, OutType>>
makeCalculator(const MooseEnumItem & item, const libMesh::ParallelObject & other)
{
  return CalculatorBuilder<InType, OutType>::build(item, other);
}

} // namespace
