//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosJaggedArray.h"

#include "gtest_include.h"

template <unsigned int inner, unsigned int outer>
class KokkosJaggedArrayTestObjectBase
{
public:
  KokkosJaggedArrayTestObjectBase(Moose::Kokkos::JaggedArray<unsigned int, inner, outer> array)
    : _array(array)
  {
  }

protected:
  KOKKOS_FUNCTION void
  fillInner(const Moose::Kokkos::JaggedArrayInnerData<unsigned int, inner> data) const
  {
    if constexpr (inner == 1)
      for (dof_id_type i = 0; i < data.n(0); ++i)
        data(i) = i;

    if constexpr (inner == 2)
      for (dof_id_type i = 0; i < data.n(0); ++i)
        for (dof_id_type j = 0; j < data.n(1); ++j)
          data(i, j) = i + j;

    if constexpr (inner == 3)
      for (dof_id_type i = 0; i < data.n(0); ++i)
        for (dof_id_type j = 0; j < data.n(1); ++j)
          for (dof_id_type k = 0; k < data.n(2); ++k)
            data(i, j, k) = i + j + k;
  }

  Moose::Kokkos::JaggedArray<unsigned int, inner, outer> _array;
};

template <unsigned int inner>
class KokkosJaggedArrayTestObject1D : public KokkosJaggedArrayTestObjectBase<inner, 1>
{
public:
  KokkosJaggedArrayTestObject1D(Moose::Kokkos::JaggedArray<unsigned int, inner, 1> array)
    : KokkosJaggedArrayTestObjectBase<inner, 1>(array)
  {
  }

  KOKKOS_FUNCTION void operator()(const int i) const { this->fillInner(this->_array(i)); }
};

template <unsigned int inner>
class KokkosJaggedArrayTestObject2D : public KokkosJaggedArrayTestObjectBase<inner, 2>
{
public:
  KokkosJaggedArrayTestObject2D(Moose::Kokkos::JaggedArray<unsigned int, inner, 2> array)
    : KokkosJaggedArrayTestObjectBase<inner, 2>(array)
  {
  }

  KOKKOS_FUNCTION void operator()(const int i, const int j) const
  {
    this->fillInner(this->_array(i, j));
  }
};

template <unsigned int inner>
class KokkosJaggedArrayTestObject3D : public KokkosJaggedArrayTestObjectBase<inner, 3>
{
public:
  KokkosJaggedArrayTestObject3D(Moose::Kokkos::JaggedArray<unsigned int, inner, 3> array)
    : KokkosJaggedArrayTestObjectBase<inner, 3>(array)
  {
  }

  KOKKOS_FUNCTION void operator()(const int i, const int j, const int k) const
  {
    this->fillInner(this->_array(i, j, k));
  }
};

class KokkosArrayTest : public ::testing::Test
{
};
