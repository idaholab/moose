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
class Calculator;

/*
 * Free function for building a const Calculator object for use by Statistics object
 */
std::unique_ptr<const Calculator> makeCalculator(const MooseEnumItem & item,
                                                 const libMesh::ParallelObject & other);

/*
 * Free function that returns the available statistics available to the Statistics object(s)
 */
MultiMooseEnum makeCalculatorEnum();

/* Base class for computing statistics (e.g., mean, min) for use with Statistics object
 *
 * The purpose of these objects are to provide an API for computing statistics in serial or parallel
 * without any state. This allows future statistics to be quickly added and for each statistic
 * to be used with the BoostrapCalculator for computing bootstrap statistics such as confidence
 * level intervals.
 *
 * The Calculator objects are created as const objects by the Statistics object via
 * the makeCalculator function.
 *
 * To create new Calculator objects first create the Calculator class and then update the
 * above free functions above.
 */
class Calculator : public libMesh::ParallelObject
{
public:
  Calculator(const libMesh::ParallelObject &);
  virtual ~Calculator() = default;
  virtual Real compute(const std::vector<Real> &, bool) const = 0;
};

class Mean : public Calculator
{
public:
  Mean(const libMesh::ParallelObject &);
  virtual Real compute(const std::vector<Real> &, bool) const override;
};

class Min : public Calculator
{
public:
  Min(const libMesh::ParallelObject &);
  virtual Real compute(const std::vector<Real> &, bool) const override;
};

class Max : public Calculator
{
public:
  Max(const libMesh::ParallelObject &);
  virtual Real compute(const std::vector<Real> &, bool) const override;
};

class Sum : public Calculator
{
public:
  Sum(const libMesh::ParallelObject &);
  virtual Real compute(const std::vector<Real> &, bool) const override;
};

class StdDev : public Calculator
{
public:
  StdDev(const libMesh::ParallelObject &);
  virtual Real compute(const std::vector<Real> &, bool) const override;
};

class StdErr : public StdDev
{
public:
  StdErr(const libMesh::ParallelObject &);
  virtual Real compute(const std::vector<Real> &, bool) const override;
};

class Ratio : public Calculator
{
public:
  Ratio(const libMesh::ParallelObject &);
  virtual Real compute(const std::vector<Real> &, bool) const override;
};

class L2Norm : public Calculator
{
public:
  L2Norm(const libMesh::ParallelObject &);
  virtual Real compute(const std::vector<Real> &, bool) const override;
};
} // namespace
