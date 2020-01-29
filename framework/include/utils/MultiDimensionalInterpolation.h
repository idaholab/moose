//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "MooseTypes.h"
#include "DualReal.h"
#include "MultiIndex.h"
#include <vector>
#include <string>

/**
 * This class interpolates multi-dimensional data sets
 */
template <typename T>
class MultiDimensionalInterpolationTempl
{
public:
  /**
   * Constructor, Takes a double vector containing the interpolation base points, and a
   * MultiIndex object. The double vector base_points and the MultiIndex object data must
   */
  MultiDimensionalInterpolationTempl(const std::vector<std::vector<Real>> & base_points,
                                     const MultiIndex<Real> & data);
  MultiDimensionalInterpolationTempl();

  virtual ~MultiDimensionalInterpolationTempl() = default;

  /**
   * Set the x and y values.
   */
  void setData(const std::vector<std::vector<Real>> & base_points, const MultiIndex<Real> & data)
  {
    _base_points = base_points;
    _data = data;
    errorCheck();
  }

  /**
   * linearSearch finds the indices i_k of the base point such that
   * base_point[d][i_k] <= values[d] < base_point[d][i_k + 1]
   * Note that argument is non-const to handle smaller/larger than tabulation range
   * For tabulations < 100 entries, linearSearch is expected to be better than bisection search
   */
  void linearSearch(std::vector<T> & values, MultiIndex<Real>::size_type & indices) const;

  /**
   * This function will take an independent variable input and will return the dependent variable
   * based on multi-linear interpolation. Multi-linear interpolation uses all 2^d values in the
   * base points surrounding x.
   */
  T multiLinearInterpolation(const std::vector<T> & x) const;

protected:
  /// checks consistency of the data
  void errorCheck();

  /**
   * linear search helper for a single std::vector; note that argument is non-const to
   * handle smaller/larger than tabulation range
   */
  unsigned int linearSearchHelper(T & x, const std::vector<Real> & vector) const;

private:
  std::vector<std::vector<Real>> _base_points;
  MultiIndex<Real> _data;
};

#define ADMultiDimensionalInterpolation                                                            \
  typename MultiDimensionalInterpolationType<compute_stage>::type

typedef MultiDimensionalInterpolationTempl<Real> MultiDimensionalInterpolation;
typedef MultiDimensionalInterpolationTempl<DualReal> DualMultiDimensionalInterpolation;

template <ComputeStage compute_stage>
struct MultiDimensionalInterpolationType
{
  typedef MultiDimensionalInterpolation type;
};
template <>
struct MultiDimensionalInterpolationType<JACOBIAN>
{
  typedef DualMultiDimensionalInterpolation type;
};
