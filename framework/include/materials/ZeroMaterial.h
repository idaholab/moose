#ifndef ZEROMATERIAL_H
#define ZEROMATERIAL_H

#include "MooseError.h"

/**
 * Helper function templates to set a variable to zero.
 * Specializations may have to be implemented (for examples see
 * RankTwoTensor, RankFourTensor).
 */
template <typename T>
inline void
mooseSetToZero(T & v)
{
  /**
   * The default for non-pointer types is to assign zero.
   * This should either do something sensible, or throw a compiler error.
   * Otherwise the T type is designed badly.
   */
  v = 0;
}
template <typename T>
inline void
mooseSetToZero(T *&)
{
  mooseError("Cannot use pointer types for MaterialProperty derivatives.");
}

#endif
