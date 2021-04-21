//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once
#ifndef ADAPTIVEMONTECARLOUTILS_H
#define ADAPTIVEMONTECARLOUTILS_H

// Forward Declarations
namespace AdaptiveMonteCarloUtils
{
  /* AdaptiveMonteCarloUtils contains functions that are used across the Adaptive Monte
  Carlo set of algorithms.*/

 Real computeSTD(const std::vector<Real> & data, const unsigned int start_index);

 Real computeMEAN(const std::vector<Real> & data, const unsigned int start_index);

} // namespace
#endif
