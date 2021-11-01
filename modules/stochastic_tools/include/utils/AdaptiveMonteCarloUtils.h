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

std::vector<Real> sortINPUT(const std::vector<Real> & inputs, const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob);

std::vector<Real> sortOUTPUT(const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob);

Real computeMIN(const std::vector<Real> & data);

std::vector<Real> computeVectorABS(const std::vector<Real> & data);

} // namespace AdaptiveMonteCarloUtils
