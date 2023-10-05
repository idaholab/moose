//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADReal.h"
#include <Eigen/Core>

namespace Eigen::internal
{

template <>
inline long double
cast<ADReal, long double>(const ADReal & x)
{
  return MetaPhysicL::raw_value(x);
}

template <>
inline double
cast<ADReal, double>(const ADReal & x)
{
  return MetaPhysicL::raw_value(x);
}

template <>
inline long
cast<ADReal, long>(const ADReal & x)
{
  return MetaPhysicL::raw_value(x);
}

template <>
inline int
cast<ADReal, int>(const ADReal & x)
{
  return MetaPhysicL::raw_value(x);
}

/**
 * Override number traits for the ADReal type used in libEigen calculations.
 * nr and mr are used to determine block matrix (panel) sizes. We keep them
 * as small as possible to avoid panel sizes that exceed the allowable stack
 * size. Those numbers could be made a function of the Eigen stack limit and
 * the number of derivative entries.
 */
template <>
class gebp_traits<ADReal, ADReal, false, false>
{
public:
  typedef ADReal ResScalar;
  enum
  {
    Vectorizable = false,
    LhsPacketSize = 1,
    RhsPacketSize = 1,
    ResPacketSize = 1,
    NumberOfRegisters = 1,
    nr = 1,
    mr = 1,
    LhsProgress = 1,
    RhsProgress = 1
  };
  typedef ResScalar LhsPacket;
  typedef ResScalar RhsPacket;
  typedef ResScalar ResPacket;
  typedef ResScalar AccPacket;
  typedef ResScalar LhsPacket4Packing;
};

template <typename Index, typename DataMapper, bool ConjugateLhs, bool ConjugateRhs>
struct gebp_kernel<ADReal, ADReal, Index, DataMapper, 1, 1, ConjugateLhs, ConjugateRhs>
{
  EIGEN_DONT_INLINE
  void operator()(const DataMapper & res,
                  const ADReal * blockA,
                  const ADReal * blockB,
                  Index rows,
                  Index depth,
                  Index cols,
                  const ADReal & alpha,
                  Index strideA = -1,
                  Index strideB = -1,
                  Index offsetA = 0,
                  Index offsetB = 0)
  {
    if (rows == 0 || cols == 0 || depth == 0)
      return;

    ADReal acc1;

    if (strideA == -1)
      strideA = depth;
    if (strideB == -1)
      strideB = depth;

    for (Index i = 0; i < rows; ++i)
      for (Index j = 0; j < cols; ++j)
      {
        const ADReal * A = blockA + i * strideA + offsetA;
        const ADReal * B = blockB + j * strideB + offsetB;

        acc1 = 0;
        for (Index k = 0; k < depth; k++)
          acc1 += A[k] * B[k];
        res(i, j) += acc1 * alpha;
      }
  }
};

}
