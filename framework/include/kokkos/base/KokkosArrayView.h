//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#ifdef MOOSE_KOKKOS_SCOPE
#include "KokkosHeader.h"

#include <type_traits>
#endif

namespace Moose::Kokkos
{

template <typename MatrixView>
class ArrayDiagonalView;

template <typename MatrixView>
class ArrayTransposeView;

template <typename Derived, unsigned int dimension>
class ArrayViewBase;

template <typename T>
inline constexpr bool is_convertible_to_arithmetic_v =
    std::is_convertible_v<T, bool> || std::is_convertible_v<T, char> ||
    std::is_convertible_v<T, signed char> || std::is_convertible_v<T, unsigned char> ||
    std::is_convertible_v<T, short> || std::is_convertible_v<T, unsigned short> ||
    std::is_convertible_v<T, int> || std::is_convertible_v<T, unsigned int> ||
    std::is_convertible_v<T, long> || std::is_convertible_v<T, unsigned long> ||
    std::is_convertible_v<T, long long> || std::is_convertible_v<T, unsigned long long> ||
    std::is_convertible_v<T, float> || std::is_convertible_v<T, double> ||
    std::is_convertible_v<T, long double>;

/**
 * CRTP base for device-accessible rank-one array views.
 */
template <typename Derived>
class ArrayViewBase<Derived, 1>
{
public:
  static constexpr unsigned int rank = 1;
  static constexpr bool has_active_component = false;

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the derived view
   * @returns The derived view
   */
  KOKKOS_FUNCTION const Derived & derived() const { return static_cast<const Derived &>(*this); }

  /**
   * Forward indexing to the derived view
   * @param indices The indices into the view
   * @returns The indexed value
   */
  template <typename... Indices>
  KOKKOS_FUNCTION decltype(auto) operator()(Indices... indices) const
  {
    return derived()(indices...);
  }

#endif
};

/**
 * CRTP base for device-accessible rank-two array views.
 */
template <typename Derived>
class ArrayViewBase<Derived, 2>
{
public:
  static constexpr unsigned int rank = 2;
  static constexpr bool has_active_component = false;

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the derived view
   * @returns The derived view
   */
  KOKKOS_FUNCTION const Derived & derived() const { return static_cast<const Derived &>(*this); }

  /**
   * Forward indexing to the derived view
   * @param indices The indices into the view
   * @returns The indexed value
   */
  template <typename... Indices>
  KOKKOS_FUNCTION decltype(auto) operator()(Indices... indices) const
  {
    return derived()(indices...);
  }

  /**
   * Get the diagonal of a rank-two view
   * @returns A rank-one view of the diagonal
   */
  KOKKOS_FUNCTION auto diagonal() const { return ArrayDiagonalView<Derived>(derived()); }

  /**
   * Get the transpose of a rank-two view
   * @returns A rank-two transpose view
   */
  KOKKOS_FUNCTION auto transpose() const { return ArrayTransposeView<Derived>(derived()); }
#endif
};

/**
 * Selectively add an ArrayViewBase base for supported ranks
 */
template <typename Derived, unsigned int dimension>
class ArrayView
{
};

template <typename Derived>
class ArrayView<Derived, 1> : public ArrayViewBase<Derived, 1>
{
};

template <typename Derived>
class ArrayView<Derived, 2> : public ArrayViewBase<Derived, 2>
{
};

#ifdef MOOSE_KOKKOS_SCOPE
/**
 * A multidimensional array view that forwards to a bound context
 */
template <typename Context, unsigned int dimension, bool active_component = false>
class ArrayContextView
  : public ArrayView<ArrayContextView<Context, dimension, active_component>, dimension>
{
public:
  static constexpr bool has_active_component = active_component;

  /**
   * Constructor
   * @param context The bound array context
   * @param comp The active component
   */
  KOKKOS_FUNCTION explicit ArrayContextView(const Context & context, const unsigned int comp = 0)
    : _context(context), _comp(comp)
  {
  }

  /**
   * Forward writable indexing to the bound context
   * @param indices The indices into the view
   * @returns The indexed value
   */
  template <typename... Indices>
  KOKKOS_FUNCTION decltype(auto) operator()(Indices... indices)
  {
    return _context(indices...);
  }

  /**
   * Forward read-only indexing to the bound context
   * @param indices The indices into the view
   * @returns The indexed value
   */
  template <typename... Indices>
  KOKKOS_FUNCTION decltype(auto) operator()(Indices... indices) const
  {
    return _context(indices...);
  }

  /**
   * Get the size of a dimension
   * @param dim The dimension index
   * @returns The size of the dimension
   */
  KOKKOS_FUNCTION auto n(const unsigned int dim) const { return _context.n(dim); }

  /**
   * Get the active component
   * @returns The active component
   */
  KOKKOS_FUNCTION unsigned int comp() const
  {
    static_assert(active_component, "This array context view has no active component");
    return _comp;
  }

private:
  Context _context;
  const unsigned int _comp;
};

/**
 * Rank-one view of the diagonal of a rank-two array view.
 */
template <typename MatrixView>
class ArrayDiagonalView : public ArrayViewBase<ArrayDiagonalView<MatrixView>, 1>
{
public:
  static constexpr bool has_active_component = MatrixView::has_active_component;

  /**
   * Constructor
   * @param matrix The rank-two array view
   */
  KOKKOS_FUNCTION explicit ArrayDiagonalView(const MatrixView & matrix) : _matrix(matrix) {}

  /**
   * Get the diagonal length
   * @returns The diagonal length
   */
  KOKKOS_FUNCTION auto n(const unsigned int) const
  {
    return _matrix.n(0) < _matrix.n(1) ? _matrix.n(0) : _matrix.n(1);
  }

  /**
   * Get a diagonal entry
   * @param i The diagonal entry index
   * @returns The diagonal entry
   */
  KOKKOS_FUNCTION const auto & operator()(const unsigned int i) const { return _matrix(i, i); }

  /**
   * Get the active component
   * @returns The active component of the rank-two view
   */
  KOKKOS_FUNCTION unsigned int comp() const;

private:
  const MatrixView & _matrix;
};

template <typename MatrixView>
KOKKOS_FUNCTION unsigned int
ArrayDiagonalView<MatrixView>::comp() const
{
  static_assert(has_active_component, "This array diagonal view has no active component");
  return _matrix.comp();
}

/**
 * Rank-two transpose view of a rank-two array view.
 */
template <typename MatrixView>
class ArrayTransposeView : public ArrayViewBase<ArrayTransposeView<MatrixView>, 2>
{
public:
  static constexpr bool has_active_component = MatrixView::has_active_component;

  /**
   * Constructor
   * @param matrix The rank-two array view
   */
  KOKKOS_FUNCTION explicit ArrayTransposeView(const MatrixView & matrix) : _matrix(matrix) {}

  /**
   * Get the size of a dimension
   * @param dim The dimension index
   * @returns The size of the transposed dimension
   */
  KOKKOS_FUNCTION auto n(unsigned int dim) const;

  /**
   * Get an entry of the transpose
   * @param i The row index
   * @param j The column index
   * @returns The transposed matrix entry
   */
  KOKKOS_FUNCTION const auto & operator()(unsigned int i, unsigned int j) const;

  /**
   * Get the active component
   * @returns The active component of the rank-two view
   */
  KOKKOS_FUNCTION unsigned int comp() const;

private:
  const MatrixView & _matrix;
};

template <typename MatrixView>
KOKKOS_FUNCTION auto
ArrayTransposeView<MatrixView>::n(const unsigned int dim) const
{
  KOKKOS_ASSERT(dim < 2);
  return dim == 0 ? _matrix.n(1) : _matrix.n(0);
}

template <typename MatrixView>
KOKKOS_FUNCTION const auto &
ArrayTransposeView<MatrixView>::operator()(const unsigned int i, const unsigned int j) const
{
  return _matrix(j, i);
}

template <typename MatrixView>
KOKKOS_FUNCTION unsigned int
ArrayTransposeView<MatrixView>::comp() const
{
  static_assert(has_active_component, "This array transpose view has no active component");
  return _matrix.comp();
}

/**
 * Get the active component shared by two rank-one views
 * @param left_view The left rank-one view
 * @param right_view The right rank-one view
 * @returns The active component
 */
template <typename LeftDerived, typename RightDerived>
KOKKOS_FUNCTION unsigned int
activeComponent(const ArrayViewBase<LeftDerived, 1> & left_view,
                const ArrayViewBase<RightDerived, 1> & right_view)
{
  static_assert(LeftDerived::has_active_component || RightDerived::has_active_component,
                "Element-wise array products require an active component");

  const auto & left = left_view.derived();
  const auto & right = right_view.derived();

  if constexpr (LeftDerived::has_active_component)
  {
    if constexpr (RightDerived::has_active_component)
      KOKKOS_ASSERT(left.comp() == right.comp());

    return left.comp();
  }
  else
    return right.comp();
}

/**
 * Get the active value of a rank-one view
 * @param view The rank-one view
 * @returns The value of the active component
 */
template <typename Derived>
KOKKOS_FUNCTION auto
activeValue(const ArrayViewBase<Derived, 1> & view)
{
  static_assert(Derived::has_active_component,
                "Array-scalar operations require an active array component");

  const auto & derived = view.derived();
  return view(derived.comp());
}

/**
 * Compute the active component of an element-wise rank-one product
 * @param left_view The left rank-one view
 * @param right_view The right rank-one view
 * @returns The product of the active components
 */
template <typename LeftDerived, typename RightDerived>
KOKKOS_FUNCTION auto
operator*(const ArrayViewBase<LeftDerived, 1> & left_view,
          const ArrayViewBase<RightDerived, 1> & right_view)
{
  KOKKOS_ASSERT(left_view.derived().n(0) == right_view.derived().n(0));

  const auto comp = activeComponent(left_view, right_view);
  return left_view(comp) * right_view(comp);
}

/**
 * Compute the active component of an element-wise rank-one sum
 * @param left_view The left rank-one view
 * @param right_view The right rank-one view
 * @returns The sum of the active components
 */
template <typename LeftDerived, typename RightDerived>
KOKKOS_FUNCTION auto
operator+(const ArrayViewBase<LeftDerived, 1> & left_view,
          const ArrayViewBase<RightDerived, 1> & right_view)
{
  KOKKOS_ASSERT(left_view.derived().n(0) == right_view.derived().n(0));

  const auto comp = activeComponent(left_view, right_view);
  return left_view(comp) + right_view(comp);
}

/**
 * Compute the active component of an element-wise rank-one difference
 * @param left_view The left rank-one view
 * @param right_view The right rank-one view
 * @returns The difference of the active components
 */
template <typename LeftDerived, typename RightDerived>
KOKKOS_FUNCTION auto
operator-(const ArrayViewBase<LeftDerived, 1> & left_view,
          const ArrayViewBase<RightDerived, 1> & right_view)
{
  KOKKOS_ASSERT(left_view.derived().n(0) == right_view.derived().n(0));

  const auto comp = activeComponent(left_view, right_view);
  return left_view(comp) - right_view(comp);
}

/**
 * Compute the active component of an element-wise rank-one quotient
 * @param left_view The left rank-one view
 * @param right_view The right rank-one view
 * @returns The quotient of the active components
 */
template <typename LeftDerived, typename RightDerived>
KOKKOS_FUNCTION auto
operator/(const ArrayViewBase<LeftDerived, 1> & left_view,
          const ArrayViewBase<RightDerived, 1> & right_view)
{
  KOKKOS_ASSERT(left_view.derived().n(0) == right_view.derived().n(0));

  const auto comp = activeComponent(left_view, right_view);
  return left_view(comp) / right_view(comp);
}

/**
 * Compute an active rank-one value multiplied by a scalar
 * @param view The rank-one view
 * @param scalar The scalar value
 * @returns The active component multiplied by the scalar
 */
template <typename Derived,
          typename Scalar,
          std::enable_if_t<is_convertible_to_arithmetic_v<Scalar>, int> = 0>
KOKKOS_FUNCTION auto
operator*(const ArrayViewBase<Derived, 1> & view, const Scalar & scalar)
{
  return activeValue(view) * scalar;
}

/**
 * Compute a scalar multiplied by an active rank-one value
 * @param scalar The scalar value
 * @param view The rank-one view
 * @returns The scalar multiplied by the active component
 */
template <typename Scalar,
          typename Derived,
          std::enable_if_t<is_convertible_to_arithmetic_v<Scalar>, int> = 0>
KOKKOS_FUNCTION auto
operator*(const Scalar & scalar, const ArrayViewBase<Derived, 1> & view)
{
  return scalar * activeValue(view);
}

/**
 * Compute an active rank-one value plus a scalar
 * @param view The rank-one view
 * @param scalar The scalar value
 * @returns The active component plus the scalar
 */
template <typename Derived,
          typename Scalar,
          std::enable_if_t<is_convertible_to_arithmetic_v<Scalar>, int> = 0>
KOKKOS_FUNCTION auto
operator+(const ArrayViewBase<Derived, 1> & view, const Scalar & scalar)
{
  return activeValue(view) + scalar;
}

/**
 * Compute a scalar plus an active rank-one value
 * @param scalar The scalar value
 * @param view The rank-one view
 * @returns The scalar plus the active component
 */
template <typename Scalar,
          typename Derived,
          std::enable_if_t<is_convertible_to_arithmetic_v<Scalar>, int> = 0>
KOKKOS_FUNCTION auto
operator+(const Scalar & scalar, const ArrayViewBase<Derived, 1> & view)
{
  return scalar + activeValue(view);
}

/**
 * Compute an active rank-one value minus a scalar
 * @param view The rank-one view
 * @param scalar The scalar value
 * @returns The active component minus the scalar
 */
template <typename Derived,
          typename Scalar,
          std::enable_if_t<is_convertible_to_arithmetic_v<Scalar>, int> = 0>
KOKKOS_FUNCTION auto
operator-(const ArrayViewBase<Derived, 1> & view, const Scalar & scalar)
{
  return activeValue(view) - scalar;
}

/**
 * Compute a scalar minus an active rank-one value
 * @param scalar The scalar value
 * @param view The rank-one view
 * @returns The scalar minus the active component
 */
template <typename Scalar,
          typename Derived,
          std::enable_if_t<is_convertible_to_arithmetic_v<Scalar>, int> = 0>
KOKKOS_FUNCTION auto
operator-(const Scalar & scalar, const ArrayViewBase<Derived, 1> & view)
{
  return scalar - activeValue(view);
}

/**
 * Compute an active rank-one value divided by a scalar
 * @param view The rank-one view
 * @param scalar The scalar value
 * @returns The active component divided by the scalar
 */
template <typename Derived,
          typename Scalar,
          std::enable_if_t<is_convertible_to_arithmetic_v<Scalar>, int> = 0>
KOKKOS_FUNCTION auto
operator/(const ArrayViewBase<Derived, 1> & view, const Scalar & scalar)
{
  return activeValue(view) / scalar;
}

/**
 * Compute a scalar divided by an active rank-one value
 * @param scalar The scalar value
 * @param view The rank-one view
 * @returns The scalar divided by the active component
 */
template <typename Scalar,
          typename Derived,
          std::enable_if_t<is_convertible_to_arithmetic_v<Scalar>, int> = 0>
KOKKOS_FUNCTION auto
operator/(const Scalar & scalar, const ArrayViewBase<Derived, 1> & view)
{
  return scalar / activeValue(view);
}

/**
 * Compute the dot product of two rank-one views
 * @param left_view The left rank-one view
 * @param right_view The right rank-one view
 * @returns The dot product
 */
template <typename LeftDerived, typename RightDerived>
KOKKOS_FUNCTION auto
dot(const ArrayViewBase<LeftDerived, 1> & left_view,
    const ArrayViewBase<RightDerived, 1> & right_view)
{
  KOKKOS_ASSERT(left_view.derived().n(0) == right_view.derived().n(0));

  using result_type = decltype(left_view(0) * right_view(0));
  result_type result = 0;

  for (unsigned int i = 0; i < left_view.derived().n(0); ++i)
    result += left_view(i) * right_view(i);

  return result;
}

/**
 * Compute the active row of a rank-two/rank-one product
 * @param matrix_view The rank-two view
 * @param vector_view The rank-one view
 * @returns The active component of the matrix-vector product
 */
template <typename MatrixDerived, typename VectorDerived>
KOKKOS_FUNCTION auto
operator*(const ArrayViewBase<MatrixDerived, 2> & matrix_view,
          const ArrayViewBase<VectorDerived, 1> & vector_view)
{
  const auto & vector = vector_view.derived();
  static_assert(VectorDerived::has_active_component,
                "Matrix-vector products require an active vector component");
  KOKKOS_ASSERT(matrix_view.derived().n(0) > vector.comp());
  KOKKOS_ASSERT(matrix_view.derived().n(1) == vector_view.derived().n(0));

  using result_type = decltype(matrix_view(vector.comp(), 0) * vector_view(0));
  result_type result = 0;

  for (unsigned int j = 0; j < vector_view.derived().n(0); ++j)
    result += matrix_view(vector.comp(), j) * vector_view(j);

  return result;
}
#endif

} // namespace Moose::Kokkos
