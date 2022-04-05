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
#include "Distribution.h"

namespace AdaptiveMonteCarloUtils
{
/* AdaptiveMonteCarloUtils contains functions that are used across the Adaptive Monte
 Carlo set of algorithms.*/

/**
 * compute the standard deviation of a data vector by only considering values from
 * a specific index.
 *
 * @param data the data vector
 * @param start_index the starting index
 */
Real computeSTD(const std::vector<Real> & data, const unsigned int & start_index);

/**
 * compute the mean of a data vector by only considering values from
 * a specific index.
 *
 * @param data the data vector
 * @param start_index the starting index
 */
Real computeMean(const std::vector<Real> & data, const unsigned int & start_index);

/**
 * return input values corresponding to the largest po percentile output values.
 *
 ****** FOR PARALLEL SUBSET SIMULATION SAMPLER ********
 *
 * @param inputs the input vector
 * @param outputs the output vector
 * @param samplesub the number of samples per subset
 * @param subset_prob  the subset intermediate failure probability
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
 * @param outputs the output vector
 * @param samplessub the number of samples per subset
 * @param subset_prob the subset intermediate failure probability
 */
std::vector<Real> sortOutput(const std::vector<Real> & outputs,
                             const unsigned int samplessub,
                             const Real subset_prob);

/**
 * return the minimum value in a vector.
 *
 * @param data the data vector
 */
Real computeMin(const std::vector<Real> & data);

/**
 * return the absolute values in a vector.
 *
 * @param data the data vector
 */
std::vector<Real> computeVectorABS(const std::vector<Real> & data);

/**
 * propose a new sample using ComponentWise-MH.
 *
 * @param x the current sample
 * @param rnd1 the first random seed
 * @param rnd2 the second random seed
 */
Real proposeNewSampleComponentWiseMH(const Real x, const Real rnd1, const Real rnd2);

/**
 * propose a new sample w/ regular MH. (AdaptiveImportanceSampler)
 *
 * @param distributions the input distributions
 * @param rnd the random seed
 * @param inputs the input vector from the reporter
 * @param inputs_sto the previously accepted samples
 */
std::vector<Real> proposeNewSampleMH(const std::vector<const Distribution *> & distributions,
                                     const Real rnd,
                                     const std::vector<std::vector<Real>> & inputs,
                                     const std::vector<std::vector<Real>> & _inputs_sto);

} // namespace AdaptiveMonteCarloUtils
