//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseUtils.h"

namespace AdaptiveMonteCarloUtils
{
/* AdaptiveMonteCarloUtils contains functions that are used across the Adaptive Monte
 Carlo set of algorithms.*/

/**
 * compute the standard deviation of a data vector by only considering values from
 * a specific index.
 *
 * @param the data vector
 * @param the starting index
 */
Real computeSTD(const std::vector<Real> & data, const unsigned int & start_index);

/**
 * compute the mean of a data vector by only considering values from
 * a specific index.
 *
 * @param the data vector
 * @param the starting index
 */
Real computeMean(const std::vector<Real> & data, const unsigned int & start_index);

/**
 * return input values corresponding to the largest po percentile output values.
 *
 ****** FOR PARALLEL SUBSET SIMULATION SAMPLER ********
 *
 * @param the input vector
 * @param the output vector
 * @param the number of samples per subset
 * @param the subset index
 * @param the subset intermediate failure probability
 */
std::vector<std::vector<Real>> sortInput(const std::vector<std::vector<Real>> & inputs,
                                         const std::vector<Real> & outputs,
                                         const unsigned int samplessub,
                                         const Real subset_prob);

/**
 * return the largest po percentile output values.
 *
 ****** FOR PARALLEL SUBSET SIMULATION SAMPLER ********
 *
 * @param the output vector
 * @param the number of samples per subset
 * @param the subset index
 * @param the subset intermediate failure probability
 */
std::vector<Real> sortOutput(const std::vector<Real> & outputs,
                             const unsigned int samplessub,
                             const Real subset_prob);

/**
 * return the minimum value in a vector.
 *
 * @param the data vector
 */
Real computeMin(const std::vector<Real> & data);

/**
 * return the absolute values in a vector.
 *
 * @param the data vector
 */
std::vector<Real> computeVectorABS(const std::vector<Real> & data);

} // namespace AdaptiveMonteCarloUtils
