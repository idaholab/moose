//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseError.h"
#include "DataIO.h"

#include <unordered_map>

// External library includes
#include "randistrs.h"

/**
 * This class encapsulates a useful, consistent, cross-platform random number generator
 * with multiple utilities.
 *
 * 1. SIMPLE INTERFACE:
 *    There are three static functions that are suitable as a drop in replacement for the
 *    random number capabilities available in the standard C++ library.
 *
 * 2. ADVANCED INTERFACE:
 *    When creating an instance of this class, one can maintain an arbitrary number of
 *    multiple independent streams of random numbers.  Furthermore, the state of these
 *    generators can be saved and restored for all streams by using the "saveState" and
 *    "restoreState" methods.  Finally, this class uses a fast hash map so that indexes
 *    for the generators are not required to be contiguous.
 */
class MooseRandom
{
public:
  /**
   * The method seeds the random number generator
   * @param seed  the seed number
   */
  static inline void seed(unsigned int seed) { mt_seed32new(seed); }

  /**
   * This method returns the next random number (Real format) from the generator
   * @return      the next random number in the range [0,1) with 64-bit precision
   */
  static inline Real rand() { return mt_ldrand(); }

  /**
   * This method returns the next random number (Real format) from the generator,
   * drawn from a normal distribution centered around mean, with a width of sigma
   * @param mean     center of the random number distribution
   * @param sigma    width  of the random number distribution
   * @return      the next random number following a normal distribution of width sigma around mean
   * with 64-bit precision
   */
  static inline Real randNormal(Real mean, Real sigma) { return rd_normal(mean, sigma); }

  /**
   * Return next random number drawn from a standard distribution.
   */
  static inline Real randNormal() { return randNormal(0.0, 1.0); }

  /**
   * This method returns the next random number (long format) from the generator
   * @return      the next random number in the range [0,max(uinit32_t)) with 32-bit number
   */
  static inline uint32_t randl() { return mt_lrand(); }

  /**
   * The method seeds one of the independent random number generators
   * @param i     the index of the generator
   * @param seed  the seed number
   */
  inline void seed(std::size_t i, unsigned int seed) { mts_seed32new(&(_states[i].first), seed); }

  /**
   * This method returns the next random number (Real format) from the specified generator
   * @param i     the index of the generator
   * @return      the next random number in the range [0,1) with 64-bit precision
   */
  inline Real rand(std::size_t i)
  {
    // mooseAssert(_states.find(i) != _states.end(), "No random state initialized for id: " << i);
    return mts_ldrand(&(_states[i].first));
  }

  /**
   * This method returns the next random number (Real format) from the specified generator,
   * drawn from a normal distribution centered around mean, with a width of sigma
   * @param i     the index of the generator
   * @param mean     center of the random number distribution
   * @param sigma    width  of the random number distribution
   * @return      the next random number following a normal distribution of width sigma around mean
   * with 64-bit precision
   */
  inline Real randNormal(std::size_t i, Real mean, Real sigma)
  {
    mooseAssert(_states.find(i) != _states.end(), "No random state initialized for id: " << i);
    return rds_normal(&(_states[i].first), mean, sigma);
  }

  /**
   * Return next random number drawn from a standard distribution.
   */
  inline Real randNormal(std::size_t i) { return randNormal(i, 0.0, 1.0); }

  /**
   * This method returns the next random number (long format) from the specified generator
   * @param i     the index of the generator
   * @return      the next random number in the range [0,max(uinit32_t)) with 32-bit number
   */
  inline uint32_t randl(std::size_t i)
  {
    mooseAssert(_states.find(i) != _states.end(), "No random state initialized for id: " << i);
    return mts_lrand(&(_states[i].first));
  }

  /**
   * This method returns the next random number (long format) from the specified generator
   * within the supplied range.
   *
   * @param lower lower bounds of value
   * @param upper upper bounds of value
   * @param i     the index of the generator
   * @return      the next random number in the range [0,max(uinit32_t)) with 32-bit number
   */
  inline uint32_t randl(std::size_t i, uint32_t lower, uint32_t upper)
  {
    mooseAssert(_states.find(i) != _states.end(), "No random state initialized for id: " << i);
    return rds_iuniform(&(_states[i].first), lower, upper);
  }

  /**
   * This method saves the current state of all generators which can be restored at a later time
   * (i.e. re-generate the same sequence of random numbers of this generator
   */
  void saveState()
  {
    _saved = true;
    std::for_each(_states.begin(),
                  _states.end(),
                  [](std::pair<const std::size_t, std::pair<mt_state, mt_state>> & pair)
                  { pair.second.second = pair.second.first; });
  }

  /**
   * This method restores the last saved generator state
   */
  void restoreState()
  {
    mooseAssert(_saved, "saveState() must be called prior to restoreState().");
    std::for_each(_states.begin(),
                  _states.end(),
                  [](std::pair<const std::size_t, std::pair<mt_state, mt_state>> & pair)
                  { pair.second.first = pair.second.second; });
  }

  /**
   * Return the number of states.
   */
  inline std::size_t size() { return _states.size(); }

private:
  /**
   * We store a pair of states in this map. The first one is the active state, the
   * second is the backup state. It is used to restore state at a later time
   * to the active state.
   */
  std::unordered_map<std::size_t, std::pair<mt_state, mt_state>> _states;

  // for restart capability
  friend void dataStore<MooseRandom>(std::ostream & stream, MooseRandom & v, void * context);
  friend void dataLoad<MooseRandom>(std::istream & stream, MooseRandom & v, void * context);

  /// Flag to make certain that saveState is called prior to restoreState
  bool _saved = false;
};

template <>
inline void
dataStore(std::ostream & stream, MooseRandom & v, void * context)
{
  storeHelper(stream, v._states, context);
}
template <>
inline void
dataLoad(std::istream & stream, MooseRandom & v, void * context)
{
  loadHelper(stream, v._states, context);
}
