//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_common.h"
#include "libmesh/compare_types.h"

namespace MetaPhysicL
{
template <typename, typename>
class DualNumber;
template <typename, typename, typename>
class SemiDynamicSparseNumberArray;
template <std::size_t, typename>
class NumberArray;
template <std::size_t N>
struct NWrapper;
}

using libMesh::Real;
using MetaPhysicL::DualNumber;
using MetaPhysicL::NumberArray;
using MetaPhysicL::NWrapper;
using MetaPhysicL::SemiDynamicSparseNumberArray;

#ifdef SPARSE_AD

typedef SemiDynamicSparseNumberArray<Real, unsigned int, NWrapper<AD_MAX_DOFS_PER_ELEM>>
    DNDerivativeType;

template <std::size_t N>
using DNDerivativeSize = SemiDynamicSparseNumberArray<Real, unsigned int, NWrapper<N>>;

#else

typedef NumberArray<AD_MAX_DOFS_PER_ELEM, Real> DNDerivativeType;

template <std::size_t N>
using DNDerivativeSize = NumberArray<N, Real>;

#endif // SPARSE_AD

typedef DualNumber<Real, DNDerivativeType> DualReal;

#ifndef LIBMESH_DUAL_NUMBER_COMPARE_TYPES

namespace libMesh
{
template <typename T, typename T2, typename D>
struct CompareTypes<T, DualNumber<T2, D>>
{
  typedef DualNumber<typename CompareTypes<T, T2>::supertype,
                     typename D::template rebind<typename CompareTypes<T, T2>::supertype>::other>
      supertype;
};
template <typename T, typename D, typename T2>
struct CompareTypes<DualNumber<T, D>, T2>
{
  typedef DualNumber<typename CompareTypes<T, T2>::supertype,
                     typename D::template rebind<typename CompareTypes<T, T2>::supertype>::other>
      supertype;
};
template <typename T, typename D, typename T2, typename D2>
struct CompareTypes<DualNumber<T, D>, DualNumber<T2, D2>>
{
  typedef DualNumber<typename CompareTypes<T, T2>::supertype,
                     typename D::template rebind<typename CompareTypes<T, T2>::supertype>::other>
      supertype;
};
template <typename T, typename D>
struct CompareTypes<DualNumber<T, D>, DualNumber<T, D>>
{
  typedef DualNumber<T, D> supertype;
};
template <typename T, typename D>
struct ScalarTraits<DualNumber<T, D>>
{
  static const bool value = ScalarTraits<T>::value;
};
}

#endif
