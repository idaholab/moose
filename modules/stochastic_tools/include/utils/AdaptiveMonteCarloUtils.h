/*************************************************/
/*           DO NOT MODIFY THIS HEADER           */
/*                                               */
/*                     MASTODON                  */
/*                                               */
/*    (c) 2015 Battelle Energy Alliance, LLC     */
/*            ALL RIGHTS RESERVED                */
/*                                               */
/*   Prepared by Battelle Energy Alliance, LLC   */
/*     With the U. S. Department of Energy       */
/*                                               */
/*     See COPYRIGHT for full restrictions       */
/*************************************************/

#ifndef ADAPTIVEMONTECARLOUTILS_H
#define ADAPTIVEMONTECARLOUTILS_H

// Forward Declarations
namespace AdaptiveMonteCarloUtils
{
  /* AdaptiveMonteCarloUtils contains functions that are used across the Adaptive Monte
  Carlo set of algorithms.*/
  
 Real computeSTD(const std::vector<Real> & data, const unsigned int start_index);

 Real computeMEAN(const std::vector<Real> & data, const unsigned int start_index);

} // namespace AdaptiveMonteCarloUtils
#endif
