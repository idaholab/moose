/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MOOSERANDOM_H
#define MOOSERANDOM_H

#include "mtwist.h"

#include "libmesh_config.h"
#include LIBMESH_INCLUDE_UNORDERED_MAP

/**
 * This class encapsulates a useful, consistent, cross-platform random number generator
 * with multiple utilities.
 *
 * 1. SIMPLE INTERFACE:
 *    There are three static functions that are suitable as a drop in replacement for the
 *    random number capabilities available in the stanadard C++ libarary.
 *
 * 2. ADVANCED INTERFACE:
 *    When creating an instance of this class, one can maintain an arbitrary number of
 *    multiple independent streams of random numbers.  Furthermore, the state of these
 *    generators can be saved and restored for all streams by using the "saveState" and
 *    "restoreState" methods.  Finally, this class uses a fast hash map so that indexes
 *    for the generators are not required to be contigous.
 */
class MooseRandom
{
public:

  /**
   * The methoed seeds the random number generator
   * @param seed  the seed number
   */
  static inline void seed(unsigned int seed)
  {
    mt_seed32new(seed);
  }

  /**
   * This method returns the next random number (double format) from the generator
   * @return      the next random number in the range [0,1) with 64-bit precision
   */
  static inline double rand()
  {
    return mt_ldrand();
  }

  /**
   * This method returns the next random number (long format) from the generator
   * @return      the next random number in the range [0,max(uinit64_t)) with 64-bit precision
   */
  static inline uint64_t randl()
  {
    return mt_llrand();
  }

  /**
   * The methoed seeds one of the independent random number generators
   * @param i     the index of the generator
   * @param seed  the seed number
   */
  inline void seed(unsigned int i, unsigned int seed)
  {
    mts_seed32new(&(_states[i]), seed);
  }

  /**
   * This method returns the next random number (double format) from the specified generator
   * @param i     the index of the generator
   * @return      the next random number in the range [0,1) with 64-bit precision
   */
  inline double rand(unsigned int i)
  {
    return mts_ldrand(&(_states[i]));
  }

  /**
   * This method returns the next random number (long format) from the specified generator
   * @param i     the index of the generator
   * @return      the next random number in the range [0,max(uinit64_t)) with 64-bit precision
   */
  inline uint64_t randl(unsigned int i)
  {
    return mts_llrand(&(_states[i]));
  }

  /**
   * This method saves the current state of all generators which can be restored at a later time
   * (i.e. re-generate the same sequence of random numbers of this generator
   */
  void saveState()
  {
    _old_states = _states;
  }

  /**
   * This method restores the last saved generator state
   */
  void restoreState()
  {
    _states = _old_states;
  }

private:
  LIBMESH_BEST_UNORDERED_MAP<unsigned int, mt_state> _states;
  LIBMESH_BEST_UNORDERED_MAP<unsigned int, mt_state> _old_states;
};

#endif // MOOSERANDOM_H
