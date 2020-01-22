//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseTypes.h"
#include "MooseObject.h"
#include <vector>

class MooseEnumItem;

namespace Statistics
{

class Calculator : public libMesh::ParallelObject
{
public:
  Calculator(const MooseObject &);
  virtual ~Calculator() = default;
  virtual Real compute(const std::vector<Real> &);
  virtual void initialize(bool) = 0;
  virtual void execute(const std::vector<Real> &, bool) = 0;
  virtual void finalize(bool) = 0;
  virtual Real value() = 0;
};

std::unique_ptr<Calculator> makeCalculator(const MooseEnumItem & name, const MooseObject &);
MultiMooseEnum makeCalculatorEnum();

class Mean : public Calculator
{
public:
  Mean(const MooseObject &);
  virtual void initialize(bool) override;
  virtual void execute(const std::vector<Real> &, bool) override;
  virtual void finalize(bool) override;
  virtual Real value() override;

protected:
  std::size_t _local_count = 0;
  Real _local_sum = 0.0;
};

class Min : public Calculator
{
public:
  Min(const MooseObject &);
  virtual void initialize(bool) override;
  virtual void execute(const std::vector<Real> &, bool) override;
  virtual void finalize(bool) override;
  virtual Real value() override;

protected:
  Real _local_min = std::numeric_limits<Real>::max();
};

class Max : public Calculator
{
public:
  Max(const MooseObject &);
  virtual void initialize(bool) override;
  virtual void execute(const std::vector<Real> &, bool) override;
  virtual void finalize(bool) override;
  virtual Real value() override;

protected:
  Real _local_max = std::numeric_limits<Real>::min();
};

class Sum : public Calculator
{
public:
  Sum(const MooseObject &);
  virtual void initialize(bool) override;
  virtual void execute(const std::vector<Real> &, bool) override;
  virtual void finalize(bool) override;
  virtual Real value() override;

protected:
  Real _local_sum = 0;
};

class StdDev : public Calculator
{
public:
  StdDev(const MooseObject &);
  virtual void initialize(bool) override {}
  virtual void execute(const std::vector<Real> &, bool) override;
  virtual void finalize(bool) override {}
  virtual Real value() override;

protected:
  Real _value = 0;
};

class Ratio : public Calculator
{
public:
  Ratio(const MooseObject &);
  virtual void initialize(bool) override;
  virtual void execute(const std::vector<Real> &, bool) override;
  virtual void finalize(bool) override;
  virtual Real value() override;

protected:
  Real _local_min = std::numeric_limits<Real>::max();
  Real _local_max = std::numeric_limits<Real>::min();
};

class L2Norm : public Calculator
{
public:
  L2Norm(const MooseObject &);
  virtual void initialize(bool) override;
  virtual void execute(const std::vector<Real> &, bool) override;
  virtual void finalize(bool) override;
  virtual Real value() override;

protected:
  Real _local_sum = 0;
};

} // namespace Statistics
