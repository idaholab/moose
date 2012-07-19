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

#ifndef MOOSESTATEFULRANDOM_H
#define MOOSESTATEFULRANDOM_H

#include "mtwist.h"

#include "libmesh_config.h"
#include LIBMESH_INCLUDE_UNORDERED_MAP

/**
 * This class is useful for maintaining multiple independent streams of random numbers.
 * The state of the generator can be saved and restored for all streams by using the save
 * and restore state methods.  In addition this class uses a hash map so that indexes are
 * not required to be contigous.
 */
class MooseStatefulRandom
{
public:
  /**
   * The methoed seeds one of the independent random number generators
   * @param i     the index of the generator
   * @param seed  the seed number
   */
  inline void seed(unsigned int i, unsigned int seed)
  {
    mts_seed32(&(_states[i]), seed);
  }

  /**
   * This method returns the next random number from the specified generator
   * @param i     the index of the generator
   * @return      the next random number in the range [0,1) with 64-bit precision
   */
  inline double rand(unsigned int i)
  {
    return mts_ldrand(&(_states[i]));
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

#endif // MOOSESTATEFULRANDOM_H
