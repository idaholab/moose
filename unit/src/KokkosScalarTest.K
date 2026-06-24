//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosScalar.h"

#include "gtest_include.h"

namespace
{

template <typename Left, typename Right>
void
expectRealBinaryOperations(const Left & left, const Right & right)
{
  EXPECT_DOUBLE_EQ(left + right, 6.0);
  EXPECT_DOUBLE_EQ(left - right, 2.0);
  EXPECT_DOUBLE_EQ(left * right, 8.0);
  EXPECT_DOUBLE_EQ(left / right, 2.0);
}

template <typename Left, typename Right>
void
expectIntegerModulo(const Left & left, const Right & right)
{
  EXPECT_EQ(left % right, 3);
}

class KokkosScalarRealBinaryOperations
{
public:
  KokkosScalarRealBinaryOperations(const Real & left,
                                   const Real & right,
                                   Kokkos::View<Real *> results)
    : _left(left), _right(right), _results(results)
  {
  }

  KOKKOS_FUNCTION void operator()(const int) const
  {
    _results(0) = _left + _right;
    _results(1) = _left - _right;
    _results(2) = _left * _right;
    _results(3) = _left / _right;

    _results(4) = _left + 2.0;
    _results(5) = _left - 2.0;
    _results(6) = _left * 2.0;
    _results(7) = _left / 2.0;

    _results(8) = 4.0 + _right;
    _results(9) = 4.0 - _right;
    _results(10) = 4.0 * _right;
    _results(11) = 4.0 / _right;
  }

private:
  Moose::Kokkos::Scalar<const Real> _left;
  Moose::Kokkos::Scalar<const Real> _right;
  Kokkos::View<Real *> _results;
};

class KokkosScalarIntegerBinaryOperations
{
public:
  KokkosScalarIntegerBinaryOperations(const int & left,
                                      const int & right,
                                      Kokkos::View<int *> results)
    : _left(left), _right(right), _results(results)
  {
  }

  KOKKOS_FUNCTION void operator()(const int) const
  {
    _results(0) = _left % _right;
    _results(1) = _left % 5;
    _results(2) = 13 % _right;
  }

private:
  Moose::Kokkos::Scalar<const int> _left;
  Moose::Kokkos::Scalar<const int> _right;
  Kokkos::View<int *> _results;
};

}

TEST(KokkosScalar, ScalarScalarRealBinaryOperations)
{
  Real left_value = 4.0;
  Real right_value = 2.0;
  const Real const_left_value = 4.0;
  const Real const_right_value = 2.0;
  int int_left_value = 4;
  int int_right_value = 2;

  Moose::Kokkos::Scalar<Real> left(left_value);
  Moose::Kokkos::Scalar<Real> right(right_value);
  Moose::Kokkos::Scalar<const Real> const_left(const_left_value);
  Moose::Kokkos::Scalar<const Real> const_right(const_right_value);
  Moose::Kokkos::Scalar<int> int_left(int_left_value);
  Moose::Kokkos::Scalar<int> int_right(int_right_value);

  expectRealBinaryOperations(left, right);
  expectRealBinaryOperations(left, const_right);
  expectRealBinaryOperations(const_left, right);
  expectRealBinaryOperations(const_left, const_right);
  expectRealBinaryOperations(left, int_right);
  expectRealBinaryOperations(int_left, right);
  expectRealBinaryOperations(int_left, int_right);
}

TEST(KokkosScalar, ScalarRegularRealBinaryOperations)
{
  Real left_value = 4.0;
  const Real const_left_value = 4.0;
  int int_left_value = 4;
  const int const_int_left_value = 4;

  Moose::Kokkos::Scalar<Real> left(left_value);
  Moose::Kokkos::Scalar<const Real> const_left(const_left_value);
  Moose::Kokkos::Scalar<int> int_left(int_left_value);
  Moose::Kokkos::Scalar<const int> const_int_left(const_int_left_value);

  expectRealBinaryOperations(left, 2.0);
  expectRealBinaryOperations(left, 2);
  expectRealBinaryOperations(const_left, 2.0);
  expectRealBinaryOperations(const_left, 2);
  expectRealBinaryOperations(int_left, 2.0);
  expectRealBinaryOperations(const_int_left, 2.0);
}

TEST(KokkosScalar, RegularScalarRealBinaryOperations)
{
  Real right_value = 2.0;
  const Real const_right_value = 2.0;
  int int_right_value = 2;
  const int const_int_right_value = 2;

  Moose::Kokkos::Scalar<Real> right(right_value);
  Moose::Kokkos::Scalar<const Real> const_right(const_right_value);
  Moose::Kokkos::Scalar<int> int_right(int_right_value);
  Moose::Kokkos::Scalar<const int> const_int_right(const_int_right_value);

  expectRealBinaryOperations(4.0, right);
  expectRealBinaryOperations(4, right);
  expectRealBinaryOperations(4.0, const_right);
  expectRealBinaryOperations(4, const_right);
  expectRealBinaryOperations(4.0, int_right);
  expectRealBinaryOperations(4.0, const_int_right);
}

TEST(KokkosScalar, IntegerModuloBinaryOperations)
{
  int left_value = 13;
  int right_value = 5;
  const int const_left_value = 13;
  const int const_right_value = 5;

  Moose::Kokkos::Scalar<int> left(left_value);
  Moose::Kokkos::Scalar<int> right(right_value);
  Moose::Kokkos::Scalar<const int> const_left(const_left_value);
  Moose::Kokkos::Scalar<const int> const_right(const_right_value);

  expectIntegerModulo(left, right);
  expectIntegerModulo(left, const_right);
  expectIntegerModulo(const_left, right);
  expectIntegerModulo(const_left, const_right);
  expectIntegerModulo(left, 5);
  expectIntegerModulo(const_left, 5);
  expectIntegerModulo(13, right);
  expectIntegerModulo(13, const_right);
}

TEST(KokkosScalar, AssignmentAndCompoundAssignment)
{
  Real value = 10.0;
  Real other_value = 4.0;
  const Real const_value = 2.0;

  Moose::Kokkos::Scalar<Real> scalar(value);
  Moose::Kokkos::Scalar<Real> other(other_value);
  Moose::Kokkos::Scalar<const Real> const_scalar(const_value);

  scalar = 1;
  EXPECT_DOUBLE_EQ(value, 1.0);

  scalar = other;
  EXPECT_DOUBLE_EQ(value, 4.0);

  scalar = const_scalar;
  EXPECT_DOUBLE_EQ(value, 2.0);

  scalar = 10.0;
  scalar += 2.5;
  EXPECT_DOUBLE_EQ(value, 12.5);

  scalar = 10.0;
  scalar += const_scalar;
  EXPECT_DOUBLE_EQ(value, 12.0);

  scalar = 10.0;
  scalar -= other;
  EXPECT_DOUBLE_EQ(value, 6.0);

  scalar = 10.0;
  scalar *= const_scalar;
  EXPECT_DOUBLE_EQ(value, 20.0);

  scalar = 10.0;
  scalar /= other;
  EXPECT_DOUBLE_EQ(value, 2.5);

  int int_value = 13;
  int int_other_value = 5;
  Moose::Kokkos::Scalar<int> int_scalar(int_value);
  Moose::Kokkos::Scalar<int> int_other(int_other_value);

  int_scalar %= 5;
  EXPECT_EQ(int_value, 3);

  int_scalar = 13;
  int_scalar %= int_other;
  EXPECT_EQ(int_value, 3);
}

TEST(KokkosScalar, UnaryAndIncrementDecrement)
{
  Real value = 3.5;
  const Real const_value = 2.5;
  Moose::Kokkos::Scalar<Real> scalar(value);
  Moose::Kokkos::Scalar<const Real> const_scalar(const_value);

  EXPECT_DOUBLE_EQ(+scalar, 3.5);
  EXPECT_DOUBLE_EQ(-scalar, -3.5);
  EXPECT_DOUBLE_EQ(+const_scalar, 2.5);
  EXPECT_DOUBLE_EQ(-const_scalar, -2.5);

  int int_value = 4;
  Moose::Kokkos::Scalar<int> int_scalar(int_value);

  ++int_scalar;
  EXPECT_EQ(int_value, 5);
  EXPECT_EQ(int_scalar++, 5);
  EXPECT_EQ(int_value, 6);
  --int_scalar;
  EXPECT_EQ(int_value, 5);
  EXPECT_EQ(int_scalar--, 5);
  EXPECT_EQ(int_value, 4);
}

TEST(KokkosScalar, KokkosFunctionRealBinaryOperations)
{
  const Real left = 4.0;
  const Real right = 2.0;
  Kokkos::View<Real *> results("results", 12);

  KokkosScalarRealBinaryOperations object(left, right, results);
  Kokkos::parallel_for(1, object);
  Kokkos::fence();

  const auto host_results = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(), results);

  EXPECT_DOUBLE_EQ(host_results(0), 6.0);
  EXPECT_DOUBLE_EQ(host_results(1), 2.0);
  EXPECT_DOUBLE_EQ(host_results(2), 8.0);
  EXPECT_DOUBLE_EQ(host_results(3), 2.0);
  EXPECT_DOUBLE_EQ(host_results(4), 6.0);
  EXPECT_DOUBLE_EQ(host_results(5), 2.0);
  EXPECT_DOUBLE_EQ(host_results(6), 8.0);
  EXPECT_DOUBLE_EQ(host_results(7), 2.0);
  EXPECT_DOUBLE_EQ(host_results(8), 6.0);
  EXPECT_DOUBLE_EQ(host_results(9), 2.0);
  EXPECT_DOUBLE_EQ(host_results(10), 8.0);
  EXPECT_DOUBLE_EQ(host_results(11), 2.0);
}

TEST(KokkosScalar, KokkosFunctionIntegerModuloOperations)
{
  const int left = 13;
  const int right = 5;
  Kokkos::View<int *> results("results", 3);

  KokkosScalarIntegerBinaryOperations object(left, right, results);
  Kokkos::parallel_for(1, object);
  Kokkos::fence();

  const auto host_results = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(), results);

  EXPECT_EQ(host_results(0), 3);
  EXPECT_EQ(host_results(1), 3);
  EXPECT_EQ(host_results(2), 3);
}
