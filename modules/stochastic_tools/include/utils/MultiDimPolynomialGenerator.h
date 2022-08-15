//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/utility.h"

#include "DataIO.h"
#include "CartesianProduct.h"

/**
 * Polynomials and quadratures based on defined distributions for Polynomial Chaos
 */
namespace StochasticTools
{
/**
 Class containing functionalitties to generate the degress of the 1D Polynomials
 within every term of a multi-dimensional polynomial.
 */
class MultiDimPolynomialGenerator
{
public:
  MultiDimPolynomialGenerator(){};

  ~MultiDimPolynomialGenerator() = default;

  /**
   * Function computing for computing _tuple
   * Example for ndim = 3, order = 4:
   * | 0 | 1 0 0 | 2 1 1 0 0 0 | 3 2 2 1 1 1 0 0 0 0 |
   * | 0 | 0 1 0 | 0 1 0 2 1 0 | 0 1 0 2 1 0 3 2 1 0 |
   * | 0 | 0 0 1 | 0 0 1 0 1 2 | 0 0 1 0 1 2 0 1 2 3 |
   */
  static std::vector<std::vector<unsigned int>>
  generateTuple(unsigned int n_dims, unsigned int max_degree, bool include_bias = true);

  /// Tuple sorter function
  static bool sortTuple(const std::vector<unsigned int> & first,
                        const std::vector<unsigned int> & second);
};

}
