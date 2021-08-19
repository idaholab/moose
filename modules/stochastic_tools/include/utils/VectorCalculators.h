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

template <typename T1, typename T2>
class CalculatorValue<typename std::vector<T1>, typename std::vector<T2>>
{
public:
  CalculatorValue() : _value() {}

  const std::vector<T2> & get() const { return _value; }

  void zero() { _value.clear(); }
  void divide(dof_id_type num);
  void pow(int p);
  void sqrt();
  void min() { _value.assign(1, std::numeric_limits<T2>::min()); }
  void max() { _value.assign(1, std::numeric_limits<T2>::max()); }

  void add(const std::vector<T1> & a);
  void addPow(const std::vector<T1> & a, int p);
  void min(const std::vector<T1> & a);
  void max(const std::vector<T1> & a);

  CalculatorValue<std::vector<T1>, std::vector<T2>> & operator+=(const std::vector<T2> & b);
  CalculatorValue<std::vector<T1>, std::vector<T2>> & operator-=(const std::vector<T2> & b);
  CalculatorValue<std::vector<T1>, std::vector<T2>> & operator/=(const std::vector<T2> & b);
  bool less_than(const std::vector<T2> & b) const { return _value < b; };

  void sum(const libMesh::Parallel::Communicator & comm);
  void min(const libMesh::Parallel::Communicator & comm);
  void max(const libMesh::Parallel::Communicator & comm);
  void broadcast(const libMesh::Parallel::Communicator & comm, processor_id_type root_id)
  {
    comm.broadcast(_value, root_id);
  }

private:
  std::vector<T2> _value;
};

template <typename T1, typename T2>
class Median<typename std::vector<std::vector<T1>>, typename std::vector<T2>>
  : public Calculator<typename std::vector<std::vector<T1>>, typename std::vector<T2>>
{
public:
  using Calculator<std::vector<std::vector<T1>>, std::vector<T2>>::Calculator;

  virtual void initialize() override;
  virtual void update(const std::vector<T1> & data) override;
  virtual void finalize(bool is_distributed) override;
  virtual std::vector<T2> get() const override { return _median; }

protected:
  std::vector<T2> _median;
  std::vector<Median<std::vector<T1>, T2>> _median_calcs;
};
} // namespace
