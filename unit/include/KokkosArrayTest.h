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

template <unsigned int dimension, Moose::Kokkos::LayoutType layout>
class KokkosArrayTestObjectBase
{
public:
  KokkosArrayTestObjectBase(Moose::Kokkos::Array<unsigned int, dimension, layout> array)
    : _array(array)
  {
  }

protected:
  Moose::Kokkos::Array<unsigned int, dimension, layout> _array;
};

class KokkosArrayTestObject1D : public KokkosArrayTestObjectBase<1, Moose::Kokkos::LayoutType::LEFT>
{
public:
  KokkosArrayTestObject1D(Moose::Kokkos::Array<unsigned int, 1> array)
    : KokkosArrayTestObjectBase<1, Moose::Kokkos::LayoutType::LEFT>(array)
  {
  }

  KOKKOS_FUNCTION void operator()(const int i) const { this->_array(i) = i; }
};

template <Moose::Kokkos::LayoutType layout = Moose::Kokkos::LayoutType::LEFT>
class KokkosArrayTestObject2D : public KokkosArrayTestObjectBase<2, layout>
{
public:
  KokkosArrayTestObject2D(Moose::Kokkos::Array<unsigned int, 2, layout> array)
    : KokkosArrayTestObjectBase<2, layout>(array)
  {
  }

  KOKKOS_FUNCTION void operator()(const int i, const int j) const { this->_array(i, j) = i + j; }
};

template <Moose::Kokkos::LayoutType layout = Moose::Kokkos::LayoutType::LEFT>
class KokkosArrayTestObject3D : public KokkosArrayTestObjectBase<3, layout>
{
public:
  KokkosArrayTestObject3D(Moose::Kokkos::Array<unsigned int, 3, layout> array)
    : KokkosArrayTestObjectBase<3, layout>(array)
  {
  }

  KOKKOS_FUNCTION void operator()(const int i, const int j, const int k) const
  {
    this->_array(i, j, k) = i + j + k;
  }
};

template <Moose::Kokkos::LayoutType layout = Moose::Kokkos::LayoutType::LEFT>
class KokkosArrayTestObject4D : public KokkosArrayTestObjectBase<4, layout>
{
public:
  KokkosArrayTestObject4D(Moose::Kokkos::Array<unsigned int, 4, layout> array)
    : KokkosArrayTestObjectBase<4, layout>(array)
  {
  }

  KOKKOS_FUNCTION void operator()(const int i, const int j, const int k, const int l) const
  {
    this->_array(i, j, k, l) = i + j + k + l;
  }
};

template <Moose::Kokkos::LayoutType layout = Moose::Kokkos::LayoutType::LEFT>
class KokkosArrayTestObject5D : public KokkosArrayTestObjectBase<5, layout>
{
public:
  KokkosArrayTestObject5D(Moose::Kokkos::Array<unsigned int, 5, layout> array)
    : KokkosArrayTestObjectBase<5, layout>(array)
  {
  }

  KOKKOS_FUNCTION void
  operator()(const int i, const int j, const int k, const int l, const int m) const
  {
    this->_array(i, j, k, l, m) = i + j + k + l + m;
  }
};

template <unsigned int inner, unsigned int outer, Moose::Kokkos::LayoutType layout>
class KokkosJaggedArrayTestObjectBase
{
public:
  KokkosJaggedArrayTestObjectBase(
      Moose::Kokkos::JaggedArray<unsigned int, inner, outer, layout> array)
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

  Moose::Kokkos::JaggedArray<unsigned int, inner, outer, layout> _array;
};

template <unsigned int inner, Moose::Kokkos::LayoutType layout = Moose::Kokkos::LayoutType::LEFT>
class KokkosJaggedArrayTestObject1D : public KokkosJaggedArrayTestObjectBase<inner, 1, layout>
{
public:
  KokkosJaggedArrayTestObject1D(Moose::Kokkos::JaggedArray<unsigned int, inner, 1, layout> array)
    : KokkosJaggedArrayTestObjectBase<inner, 1, layout>(array)
  {
  }

  KOKKOS_FUNCTION void operator()(const int i) const { this->fillInner(this->_array(i)); }
};

template <unsigned int inner, Moose::Kokkos::LayoutType layout = Moose::Kokkos::LayoutType::LEFT>
class KokkosJaggedArrayTestObject2D : public KokkosJaggedArrayTestObjectBase<inner, 2, layout>
{
public:
  KokkosJaggedArrayTestObject2D(Moose::Kokkos::JaggedArray<unsigned int, inner, 2, layout> array)
    : KokkosJaggedArrayTestObjectBase<inner, 2, layout>(array)
  {
  }

  KOKKOS_FUNCTION void operator()(const int i, const int j) const
  {
    this->fillInner(this->_array(i, j));
  }
};

template <unsigned int inner, Moose::Kokkos::LayoutType layout = Moose::Kokkos::LayoutType::LEFT>
class KokkosJaggedArrayTestObject3D : public KokkosJaggedArrayTestObjectBase<inner, 3, layout>
{
public:
  KokkosJaggedArrayTestObject3D(Moose::Kokkos::JaggedArray<unsigned int, inner, 3, layout> array)
    : KokkosJaggedArrayTestObjectBase<inner, 3, layout>(array)
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
