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
  void min() { _value.assign(_value.size(), std::numeric_limits<T2>::min()); }
  void max() { _value.assign(_value.size(), std::numeric_limits<T2>::max()); }

  void add(const std::vector<T1> & a);
  void addPow(const std::vector<T1> & a, int p);
  void min(const std::vector<T1> & a);
  void max(const std::vector<T1> & a);

  CalculatorValue<std::vector<T1>, std::vector<T2>> & operator-=(const std::vector<T2> & b);
  CalculatorValue<std::vector<T1>, std::vector<T2>> & operator/=(const std::vector<T2> & b);

  void sum(const libMesh::Parallel::Communicator & comm);
  void min(const libMesh::Parallel::Communicator & comm);
  void max(const libMesh::Parallel::Communicator & comm);

private:
  std::vector<T2> _value;
};

} // namespace
