//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADReal.h"
#include "metaphysicl/raw_type.h"
#include "metaphysicl/metaphysicl_version.h"

namespace Eigen
{
namespace internal
{
template <typename V, typename D, bool asd>
inline bool
isinf_impl(const MetaPhysicL::DualNumber<V, D, asd> & a)
{
  using std::isinf;
  return isinf(a);
}

template <typename V, typename D, bool asd>
inline bool
isnan_impl(const MetaPhysicL::DualNumber<V, D, asd> & a)
{
  using std::isnan;
  return isnan(a);
}

template <typename V, typename D, bool asd>
inline MetaPhysicL::DualNumber<V, D, asd>
sqrt(const MetaPhysicL::DualNumber<V, D, asd> & a)
{
#if METAPHYSICL_MAJOR_VERSION < 2
  return std::sqrt(a);
#else
  return MetaPhysicL::sqrt(a);
#endif
}

template <typename V, typename D, bool asd>
inline MetaPhysicL::DualNumber<V, D, asd>
abs(const MetaPhysicL::DualNumber<V, D, asd> & a)
{
#if METAPHYSICL_MAJOR_VERSION < 2
  return std::abs(a);
#else
  return MetaPhysicL::abs(a);
#endif
}
}
} // namespace Eigen

// this include _must_ come after the Eigen::internal overloads above. We also ignore a warning
// about an Eigen internal use of a potentially uninitialized variable
#include "libmesh/ignore_warnings.h"
#include <Eigen/Core>
#include "libmesh/restore_warnings.h"

// Eigen needs this
namespace MetaPhysicL
{
// raw_value AD->non-AD conversion for ADReal valued Eigen::Matrix objects
template <typename T, int M, int N, int O, int M2, int N2>
struct RawType<Eigen::Matrix<T, M, N, O, M2, N2>>
{
  typedef Eigen::Matrix<typename RawType<T>::value_type, M, N, O, M2, N2> value_type;

  static value_type value(const Eigen::Matrix<T, M, N, O, M2, N2> & in)
  {
    return value_type::NullaryExpr([&in](Eigen::Index i) { return raw_value(in(i)); });
  }
};

// raw_value overload for Map type objects that forces evaluation
template <typename T>
auto
raw_value(const Eigen::Map<T> & in)
{
  return raw_value(in.eval());
}
} // namespace MetaPhysicL

namespace Eigen
{
// libEigen support for dual number types
template <typename V, typename D, bool asd>
struct NumTraits<MetaPhysicL::DualNumber<V, D, asd>>
  : NumTraits<V> // permits to get the epsilon, dummy_precision, lowest, highest functions
{
  typedef MetaPhysicL::DualNumber<V, D, asd> Real;
  typedef MetaPhysicL::DualNumber<V, D, asd> NonInteger;
  typedef MetaPhysicL::DualNumber<V, D, asd> Nested;

  enum
  {
    IsComplex = 0,
    IsInteger = 0,
    IsSigned = 1,
    RequireInitialization = 1,
    ReadCost = HugeCost,
    AddCost = HugeCost,
    MulCost = HugeCost
  };
};

template <typename BinaryOp, typename V, typename D, bool asd>
struct ScalarBinaryOpTraits<Real, MetaPhysicL::DualNumber<V, D, asd>, BinaryOp>
{
  typedef MetaPhysicL::DualNumber<V, D, asd> ReturnType;
};
template <typename BinaryOp, typename V, typename D, bool asd>
struct ScalarBinaryOpTraits<MetaPhysicL::DualNumber<V, D, asd>, Real, BinaryOp>
{
  typedef MetaPhysicL::DualNumber<V, D, asd> ReturnType;
};
} // namespace Eigen

namespace Moose
{
template <typename T>
struct ADType;

template <typename T, int M, int N, int O, int M2, int N2>
struct ADType<Eigen::Matrix<T, M, N, O, M2, N2>>
{
  typedef typename Eigen::Matrix<typename ADType<T>::type, M, N, O, M2, N2> type;
};
}

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
