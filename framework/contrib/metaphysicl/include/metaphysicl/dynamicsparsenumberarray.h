//-----------------------------------------------------------------------bl-
//--------------------------------------------------------------------------
//
// MetaPhysicL - A metaprogramming library for physics calculations
//
// Copyright (C) 2013 The PECOS Development Team
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the Version 2.1 GNU Lesser General
// Public License as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc. 51 Franklin Street, Fifth Floor,
// Boston, MA  02110-1301  USA
//
//-----------------------------------------------------------------------el-
//
// $Id: core.h 37197 2013-02-21 05:49:09Z roystgnr $
//
//--------------------------------------------------------------------------


#ifndef METAPHYSICL_DYNAMICSPARSENUMBERARRAY_H
#define METAPHYSICL_DYNAMICSPARSENUMBERARRAY_H

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <ostream>
#include <vector>

#include "metaphysicl/compare_types.h"
#include "metaphysicl/ct_set.h"
#include "metaphysicl/metaphysicl_asserts.h"
#include "metaphysicl/raw_type.h"
#include "metaphysicl/sparsenumberutils.h"
#include "metaphysicl/testable.h"

namespace MetaPhysicL {

// Forward declarations

// Data type T, index type I
template <typename T, typename I>
class DynamicSparseNumberArray;

// Helper structs

template<typename I1, typename I2, typename S, typename T, bool reverseorder>
struct DotType<DynamicSparseNumberArray<S,I1>,
               DynamicSparseNumberArray<T,I2>, reverseorder> {
  typedef
    DynamicSparseNumberArray
      <typename DotType<S,T,reverseorder>::supertype,
       typename CompareTypes<I1, I2>::supertype>
      supertype;
};

template<typename I1, typename I2, typename S, typename T, bool reverseorder>
struct OuterProductType<DynamicSparseNumberArray<S, I1>,
                        DynamicSparseNumberArray<T, I2>, reverseorder> {
  typedef
    DynamicSparseNumberArray
      <typename OuterProductType<S,T,reverseorder>::supertype,
       typename CompareTypes<I1, I2>::supertype>
      supertype;
};

template<typename S, typename I>
struct SumType<DynamicSparseNumberArray<S, I> > {
  typedef DynamicSparseNumberArray<typename SumType<S>::supertype, I> supertype;
};


template <typename T, typename I>
class DynamicSparseNumberArray :
  public safe_bool<DynamicSparseNumberArray<T,I> >
{
public:
  typedef T value_type;

  template <unsigned int i>
  struct entry_type {
    typedef value_type type;
  };

  typedef I index_value_type;

  template <typename T2>
  struct rebind {
    typedef DynamicSparseNumberArray<T2, I> other;
  };

  std::size_t size() const
    { metaphysicl_assert_equal_to(_data.size(), _indices.size());
      return _data.size(); }

  void resize(std::size_t s)
    { metaphysicl_assert_equal_to(_data.size(), _indices.size());
      _data.resize(s);
      _indices.resize(s); }

  DynamicSparseNumberArray() {}

  DynamicSparseNumberArray(const T& val) {
    // This makes no sense unless val is 0!
#ifndef NDEBUG
    if (val)
      throw std::domain_error("Cannot initialize DynamicSparseNumberArray with non-zero scalar");
#endif
  }

  template <typename T2>
  DynamicSparseNumberArray(const T2& val) {
    // This makes no sense unless val is 0!
#ifndef NDEBUG
    if (val)
      throw std::domain_error("Cannot initialize DynamicSparseNumberArray with non-zero scalar");
#endif
  }

  template <typename T2, typename I2>
  DynamicSparseNumberArray(DynamicSparseNumberArray<T2, I2> src)
    { _data.resize(src._data.size());
      _indices.resize(src._indices.size());
      std::copy(src._data.begin(), src._data.end(), _data.begin());
      std::copy(src._indices.begin(), src._indices.end(), _indices.begin()); }

  T* raw_data()
    { return size()?&_data[0]:NULL; }

  const T* raw_data() const
    { return size()?&_data[0]:NULL; }

  T& raw_at(unsigned int i)
    { return _data[i]; }

  const T& raw_at(unsigned int i) const
    { return _data[i]; }

  I& raw_index(unsigned int i)
    { return _indices[i]; }

  const I& raw_index(unsigned int i) const
    { return _indices[i]; }

  // FIXME: these encapsulation violations are necessary for std::pow
  // until I can figure out the right friend declaration.
  const std::vector<T>& nude_data() const
    { return _data; }

  std::vector<T>& nude_data()
    { return _data; }

  const std::vector<I>& nude_indices() const
    { return _indices; }

  std::vector<I>& nude_indices()
    { return _indices; }

  std::size_t runtime_index_of(index_value_type i) const
    {
      typename std::vector<I>::const_iterator it =
        std::lower_bound(_indices.begin(), _indices.end(), i);
      metaphysicl_assert(it != _indices.end());
      std::size_t offset = it - _indices.begin();
      metaphysicl_assert_equal_to(_indices[offset], i);
      return offset;
    }

  T& operator[](index_value_type i)
    { return _data[runtime_index_of(i)]; }

  const T& operator[](index_value_type i) const
    { return _data[runtime_index_of(i)]; }

  template <unsigned int i>
  typename entry_type<i>::type& get() {
    return _data[runtime_index_of(i)];
  }

  template <unsigned int i>
  const typename entry_type<i>::type& get() const {
    return _data[runtime_index_of(i)];
  }

  template <unsigned int i, typename T2>
  void set(const T2& val) {
    _data[runtime_index_of(i)] = val;
  }

  bool boolean_test() const {
    std::size_t index_size = size();
    for (unsigned int i=0; i != index_size; ++i)
      if (_data[i])
        return true;
    return false;
  }

  DynamicSparseNumberArray<T,I> operator- () const {
    std::size_t index_size = size();
    DynamicSparseNumberArray<T,I> returnval;
    returnval.resize(index_size);
    for (unsigned int i=0; i != index_size; ++i) returnval.raw_at(i) = -_data[i];
    return returnval;
  }

  // Since this is a dynamically allocated sparsity pattern, we can
  // increase it as needed to support e.g. operator+=
  template <typename I2>
  void sparsity_union (std::vector<I2> new_indices) {
    typename std::vector<I>::iterator index_it = _indices.begin();
    typename std::vector<I2>::const_iterator index2_it = new_indices.begin();

    typedef typename CompareTypes<I,I2>::supertype max_index_type;
    max_index_type unseen_indices = 0;

    I maxI = std::numeric_limits<I>::max();

    while (index2_it != new_indices.end()) {
      I idx1 = (index_it == _indices.end()) ? maxI : *index_it;
      I2 idx2 = *index2_it;

      while (idx1 < idx2) {
        ++index_it;
        idx1 = (index_it == _indices.end()) ? maxI : *index_it;
      }

      while ((idx1 == idx2) &&
             (idx1 != maxI)) {
        ++index_it;
        idx1 = (index_it == _indices.end()) ? maxI : *index_it;
        ++index2_it;
        idx2 = (index2_it == new_indices.end()) ? maxI : *index2_it;
      }

      while (idx2 < idx1) {
        ++unseen_indices;
        ++index2_it;
        if (index2_it == new_indices.end())
          break;
        idx2 = *index2_it;
      }
    }

    // The common case is cheap
    if (!unseen_indices)
      return;

    std::vector<T> merged_data(_data.size() + unseen_indices);
    std::vector<I> merged_indices(_indices.size() + unseen_indices);

    typename std::vector<T>::iterator md_it = merged_data.begin();
    typename std::vector<I>::iterator mi_it = merged_indices.begin();

    typename std::vector<T>::iterator d_it = _data.begin();
    typename std::vector<I>::iterator i_it = _indices.begin();
    typename std::vector<I2>::const_iterator i2_it = new_indices.begin();

    for (; i_it != _indices.end(); ++md_it, ++mi_it) {
      if (i2_it == new_indices.end()) {
        std::copy(i_it, _indices.end(), mi_it);
        std::copy(d_it, _data.end(), md_it);
      }
      if (*i2_it < *i_it) {
          ++i2_it;
      } else {
          *md_it = *d_it;
          *mi_it = *i_it;
          ++d_it;
          ++i_it;
      }
    }

    _indices.swap(merged_indices);
    _data.swap(merged_data);
  }


  // Since this is a dynamically allocated sparsity pattern, we can
  // decrease it when possible for efficiency
  template <typename I2>
  void sparsity_intersection (std::vector<I2> new_indices) {
    typename std::vector<I>::iterator index_it = _indices.begin();
    typename std::vector<I2>::const_iterator index2_it = new_indices.begin();

    typedef typename CompareTypes<I,I2>::supertype max_index_type;
    max_index_type shared_indices = 0;

    I maxI = std::numeric_limits<I>::max();

    while (index2_it != new_indices.end()) {
      I idx1 = (index_it == _indices.end()) ? maxI : *index_it;
      I2 idx2 = *index2_it;

      while (idx1 < idx2) {
        ++index_it;
        idx1 = (index_it == _indices.end()) ? maxI : *index_it;
      }

      while ((idx1 == idx2) &&
             (idx1 != maxI)) {
        ++index_it;
        idx1 = (index_it == _indices.end()) ? maxI : *index_it;
        ++index2_it;
        idx2 = (index2_it == new_indices.end()) ? maxI : *index2_it;
        ++shared_indices;
      }

      while (idx2 < idx1) {
        ++index2_it;
        if (index2_it == new_indices.end())
          break;
        idx2 = *index2_it;
      }
    }

    std::vector<T> merged_data(shared_indices);
    std::vector<I> merged_indices(shared_indices);

    typename std::vector<T>::iterator md_it = merged_data.begin();
    typename std::vector<I>::iterator mi_it = merged_indices.begin();

    typename std::vector<T>::iterator d_it = _data.begin();
    typename std::vector<I>::iterator i_it = _indices.begin();
    typename std::vector<I2>::const_iterator i2_it = new_indices.begin();

    for (; i_it != _indices.end() && i2_it != new_indices.end();
         ++md_it, ++mi_it, ++d_it, ++i_it, ++i2_it) {
      while (*i2_it < *i_it) {
        ++i2_it;
        if (i2_it == new_indices.end())
          break;
      }
      if (i2_it == new_indices.end())
        break;
      while (*i2_it > *i_it) {
          ++i_it;
        if (i_it == _indices.end())
          break;
      }
      if (i_it == _indices.end())
        break;

      *md_it = *d_it;
      *mi_it = *i_it;
    }

    _indices.swap(merged_indices);
    _data.swap(merged_data);
  }


  // Not defineable since !0 != 0
  // DynamicSparseNumberArray<T,I> operator! () const;


  template <typename T2, typename I2>
  DynamicSparseNumberArray<T,I>&
    operator+= (const DynamicSparseNumberArray<T2,I2>& a) {
    // Resize if necessary
    this->sparsity_union(a._indices);

    typename std::vector<T>::iterator data_it  = _data.begin();
    typename std::vector<I>::iterator index_it = _indices.begin();
    typename std::vector<T2>::const_iterator data2_it  = a._data.begin();
    typename std::vector<I2>::const_iterator index2_it = a._indices.begin();
    for (; data2_it != a._data.end(); ++data2_it, ++index2_it)
      {
        I idx1 = *index_it;
        I2 idx2 = *index2_it;

        while (idx1 < idx2) {
          ++index_it;
          ++data_it;
          metaphysicl_assert(index_it != _indices.end());
          idx1 = *index_it;
        }
        metaphysicl_assert_equal_to(idx1, idx2);

        *data_it += *data2_it;
      }

    return *this;
  }


  template <typename T2, typename I2>
  DynamicSparseNumberArray<T,I>&
    operator-= (const DynamicSparseNumberArray<T2,I2>& a) {
    // Resize if necessary
    this->sparsity_union(a._indices);

    typename std::vector<T>::iterator data_it  = _data.begin();
    typename std::vector<I>::iterator index_it = _indices.begin();
    typename std::vector<T2>::const_iterator data2_it  = a._data.begin();
    typename std::vector<I2>::const_iterator index2_it = a._indices.begin();
    for (; data2_it != a._data.end(); ++data2_it, ++index2_it)
      {
        I idx1 = *index_it;
        I2 idx2 = *index2_it;

        while (idx1 < idx2) {
          ++index_it;
          ++data_it;
          metaphysicl_assert(index_it != _indices.end());
          idx1 = *index_it;
        }
        metaphysicl_assert_equal_to(idx1, idx2);

        *data_it -= *data2_it;
      }

    return *this;
  }


  template <typename T2, typename I2>
  DynamicSparseNumberArray<T,I>&
    operator*= (const DynamicSparseNumberArray<T2,I2>& a) {
    // Resize if possible
    this->sparsity_intersection(a._indices);

    typename std::vector<T>::iterator data_it  = _data.begin();
    typename std::vector<I>::iterator index_it = _indices.begin();
    typename std::vector<T2>::const_iterator data2_it  = a._data.begin();
    typename std::vector<I2>::const_iterator index2_it = a._indices.begin();
    for (; data2_it != a._data.end(); ++data2_it, ++index2_it)
      {
        I idx1 = *index_it;
        I2 idx2 = *index2_it;

        while (idx1 < idx2) {
          ++index_it;
          ++data_it;
          metaphysicl_assert(index_it != _indices.end());
          idx1 = *index_it;
        }

        if (idx1 == idx2)
          *data_it *= *data2_it;
      }

    return *this;
  }


  template <typename T2, typename I2>
  DynamicSparseNumberArray<T,I>&
    operator/= (const DynamicSparseNumberArray<T2,I2>& a) {
    typename std::vector<T>::iterator data_it  = _data.begin();
    typename std::vector<I>::iterator index_it = _indices.begin();
    typename std::vector<T2>::const_iterator data2_it  = a._data.begin();
    typename std::vector<I2>::const_iterator index2_it = a._indices.begin();
    for (; data2_it != a._data.end(); ++data2_it, ++index2_it)
      {
        I idx1 = *index_it;
        I2 idx2 = *index2_it;

        while (idx1 < idx2) {
          ++index_it;
          ++data_it;
          metaphysicl_assert(index_it != _indices.end());
          idx1 = *index_it;
        }

        if (idx1 == idx2)
          *data_it /= *data2_it;
      }

    return *this;
  }


  template <typename T2>
  DynamicSparseNumberArray<T,I>& operator*= (const T2& a) {
    std::size_t index_size = size();
    for (unsigned int i=0; i != index_size; ++i)
      _data[i] *= a;
    return *this;
  }

  template <typename T2>
  DynamicSparseNumberArray<T,I>& operator/= (const T2& a) {
    std::size_t index_size = size();
    for (unsigned int i=0; i != index_size; ++i)
      _data[i] /= a;
    return *this;
  }

  template <typename T2, typename I2>
  DynamicSparseNumberArray
    <typename DotType<T,T2>::supertype,
     typename CompareTypes<I, I2>::supertype>
  dot (const DynamicSparseNumberArray<T2,I2>& a) const
  {
    typedef typename DotType<T,T2>::supertype TS;
    typedef typename CompareTypes<I, I2>::supertype IS;

    DynamicSparseNumberArray<TS, IS> returnval;

    // FIXME
    metaphysicl_not_implemented();

    return returnval;
  }

  template <typename T2, typename I2>
  DynamicSparseNumberArray<
    typename OuterProductType<T,T2>::supertype,
    typename CompareTypes<I, I2>::supertype>
  outerproduct (const DynamicSparseNumberArray<T2, I2>& a) const
  {
    typedef typename OuterProductType<T,T2>::supertype TS;
    typedef typename CompareTypes<I, I2>::supertype IS;
    DynamicSparseNumberArray<TS, IS> returnval;

    // FIXME
    metaphysicl_not_implemented();

    return returnval;
  }

// Not defined, because size is not fixed at compile time
//  static DynamicSparseNumberArray<DynamicSparseNumberArray<T, I>, I> identity()

private:

  std::vector<T> _data;
  std::vector<I> _indices;
};


//
// Non-member functions
//

template <unsigned int N,
          unsigned int index1=0, typename Data1=void,
          unsigned int index2=0, typename Data2=void,
          unsigned int index3=0, typename Data3=void,
          unsigned int index4=0, typename Data4=void,
          unsigned int index5=0, typename Data5=void,
          unsigned int index6=0, typename Data6=void,
          unsigned int index7=0, typename Data7=void,
          unsigned int index8=0, typename Data8=void>
struct DynamicSparseNumberArrayOf
{
  typedef
  typename SymmetricCompareTypes<Data1,
    typename SymmetricCompareTypes<Data2,
      typename SymmetricCompareTypes<Data3,
        typename SymmetricCompareTypes<Data4,
          typename SymmetricCompareTypes<Data5,
            typename SymmetricCompareTypes<Data6,
              typename SymmetricCompareTypes<Data7,Data8>::supertype
            >::supertype
          >::supertype
        >::supertype
      >::supertype
    >::supertype
  >::supertype supertype;

  typedef DynamicSparseNumberArray<supertype, unsigned int> type;
};



template <std::size_t N, unsigned int index, typename T>
struct DynamicSparseNumberArrayUnitVector
{
  typedef DynamicSparseNumberArray<T, unsigned int> type;

  static const type value() {
    type returnval;
    returnval.resize(1);
    returnval.raw_at(0) = 1;
    returnval.raw_index(0) = index;
    return returnval;
  }
};


template <std::size_t N, typename T>
struct DynamicSparseNumberArrayFullVector
{
  typedef DynamicSparseNumberArray<T,unsigned int> type;

  static const type value() {
    type returnval;
    returnval.resize(N);
    for (unsigned int i=0; i != N; ++i)
      {
        returnval.raw_at(i) = 1;
        returnval.raw_index(i) = i;
      }
    return returnval;
  }
};



template <typename T, typename I, typename I2>
inline
DynamicSparseNumberArray<DynamicSparseNumberArray<T, I>, I2>
transpose(const DynamicSparseNumberArray<DynamicSparseNumberArray<T, I2>, I>& a)
{
  DynamicSparseNumberArray<DynamicSparseNumberArray<T, I>, I2> returnval;

  metaphysicl_not_implemented();

  return returnval;
}


template <typename T, typename I>
DynamicSparseNumberArray<typename SumType<T>::supertype, I>
sum (const DynamicSparseNumberArray<T, I> &a)
{
  std::size_t index_size = a.size();

  DynamicSparseNumberArray<typename SumType<T>::supertype, I>
    returnval;
  returnval.resize(index_size);

  for (unsigned int i=0; i != index_size; ++i) {
    returnval.raw_at(i) = a.raw_at(i).sum();
    returnval.raw_index(i) = a.raw_index(i);
  }

  return returnval;
}



#define DynamicSparseNumberArray_op_ab(opname, atype, btype, functorname) \
template <typename T, typename T2, typename I, typename I2> \
inline \
typename Symmetric##functorname##Type<atype,btype>::supertype \
operator opname (const atype& a, const btype& b) \
{ \
  typedef typename Symmetric##functorname##Type<atype,btype>::supertype type; \
  type returnval = a; \
  returnval opname##= b; \
  return returnval; \
}

#define DynamicSparseNumberArray_op(opname, functorname) \
DynamicSparseNumberArray_op_ab(opname, DynamicSparseNumberArray<T MacroComma I>, DynamicSparseNumberArray<T2 MacroComma I2>, functorname)

DynamicSparseNumberArray_op(+, Plus)       // Union)
DynamicSparseNumberArray_op(-, Minus)      // Union)
DynamicSparseNumberArray_op(*, Multiplies) // Intersection)
DynamicSparseNumberArray_op(/, Divides)    // First)

// Let's also allow scalar times vector.
// Scalar plus vector, etc. remain undefined in the sparse context.

template <typename T, typename T2, typename I>
inline
typename MultipliesType<DynamicSparseNumberArray<T2,I>,T,true>::supertype
operator * (const T& a, const DynamicSparseNumberArray<T2,I>& b)
{
  static const unsigned int index_size = b.size();

  typename MultipliesType<DynamicSparseNumberArray<T2,I>,T,true>::supertype
    returnval;
  returnval.resize(index_size);

  for (unsigned int i=0; i != index_size; ++i) {
    returnval.raw_at(i) = a * b.raw_at(i);
    returnval.raw_index(i) = b.raw_index(i);
  }

  return returnval;
}

template <typename T, typename T2, typename I>
inline
typename MultipliesType<DynamicSparseNumberArray<T,I>,T2>::supertype
operator * (const DynamicSparseNumberArray<T,I>& a, const T2& b)
{
  static const unsigned int index_size = a.size();

  typename MultipliesType<DynamicSparseNumberArray<T,I>,T2>::supertype
    returnval;
  returnval.resize(index_size);

  for (unsigned int i=0; i != index_size; ++i) {
    returnval.raw_at(i) = a.raw_at(i) * b;
    returnval.raw_index(i) = a.raw_index(i);
  }
  return returnval;
}

template <typename T, typename T2, typename I>
inline
typename DividesType<DynamicSparseNumberArray<T,I>,T2>::supertype
operator / (const DynamicSparseNumberArray<T,I>& a, const T2& b)
{
  static const unsigned int index_size = a.size();

  typename DividesType<DynamicSparseNumberArray<T,I>,T2>::supertype returnval;
  returnval.resize(index_size);

  for (unsigned int i=0; i != index_size; ++i) {
    returnval.raw_at(i) = a.raw_at(i) / b;
    returnval.raw_index(i) = a.raw_index(i);
  }

  return returnval;
}


#define DynamicSparseNumberArray_operator_binary(opname, functorname) \
template <typename T, typename T2, typename I, typename I2> \
inline \
DynamicSparseNumberArray<bool, typename CompareTypes<I,I2>::supertype> \
operator opname (const DynamicSparseNumberArray<T,I>& a, \
                 const DynamicSparseNumberArray<T2,I2>& b) \
{ \
  typedef typename SymmetricCompareTypes<T,T2>::supertype TS; \
  typedef typename CompareTypes<I,I2>::supertype IS; \
  DynamicSparseNumberArray<bool, IS> returnval; \
  returnval._indices = a._indices; \
  returnval._data.resize(a._indices.size()); \
  returnval.sparsity_union(b._indices); \
 \
  typename std::vector<I>::const_iterator  index_a_it = a._indices.begin(); \
  typename std::vector<I2>::const_iterator index_b_it = b._indices.begin(); \
  typename std::vector<IS>::iterator     index_out_it = returnval._indices.begin(); \
 \
  typename std::vector<T>::const_iterator  data_a_it = a._data.begin(); \
  typename std::vector<T2>::const_iterator data_b_it = b._data.begin(); \
  typename std::vector<TS>::iterator     data_out_it = returnval._data.begin(); \
 \
  IS  maxIS  = std::numeric_limits<IS>::max(); \
 \
  for (; index_out_it != returnval._indices.end(); ++index_out_it, ++data_out_it) { \
    const IS index_a = (index_a_it == a._indices.end()) ? maxIS : *index_a_it; \
    const IS index_b = (index_b_it == b._indices.end()) ? maxIS : *index_b_it; \
    const IS index_out = *index_out_it; \
    const TS data_a  = (index_a_it == a._indices.end()) ? 0: *data_a_it; \
    const TS data_b  = (index_b_it == b._indices.end()) ? 0: *data_b_it; \
    TS &   data_out  = *data_out_it; \
 \
    if (index_a == index_out) { \
      if (index_b == index_out) { \
        data_out = data_a opname data_b; \
        index_b_it++; \
        data_b_it++; \
      } else { \
        data_out = data_a opname 0; \
      } \
      index_a_it++; \
      data_a_it++; \
    } else { \
      metaphysicl_assert_equal_to(index_b, index_out); \
      data_out = 0 opname data_b; \
      index_b_it++; \
      data_b_it++; \
    } \
  } \
 \
  return returnval; \
} \
template <typename T, typename T2, typename I> \
inline \
DynamicSparseNumberArray<bool, I> \
operator opname (const DynamicSparseNumberArray<T, I>& a, const T2& b) \
{ \
  DynamicSparseNumberArray<bool, I> returnval; \
 \
  std::size_t index_size = a.size(); \
  returnval.resize(index_size); \
  returnval._indices = a._indices; \
 \
  for (unsigned int i=0; i != index_size; ++i) \
    returnval.raw_at(i) = (a.raw_at(i) opname b); \
 \
  return returnval; \
} \
template <typename T, typename T2, typename I> \
inline \
DynamicSparseNumberArray<bool, I> \
operator opname (const T& a, const DynamicSparseNumberArray<T2,I>& b) \
{ \
  DynamicSparseNumberArray<bool, I> returnval; \
 \
  std::size_t index_size = a.size(); \
  returnval._indices = a._indices; \
  returnval._data.resize(index_size); \
 \
  for (unsigned int i=0; i != index_size; ++i) \
    returnval.raw_at(i) = (a opname b.raw_at(i)); \
 \
  return returnval; \
}

// NOTE: unary functions for which 0-op-0 is true are undefined compile-time
// errors, because there's no efficient way to have them make sense in
// the sparse context.

DynamicSparseNumberArray_operator_binary(<, less)
// DynamicSparseNumberArray_operator_binary(<=)
DynamicSparseNumberArray_operator_binary(>, greater)
// DynamicSparseNumberArray_operator_binary(>=)
// DynamicSparseNumberArray_operator_binary(==)
DynamicSparseNumberArray_operator_binary(!=, not_equal_to)

// FIXME - make && an intersection rather than a union for efficiency
DynamicSparseNumberArray_operator_binary(&&, logical_and)
DynamicSparseNumberArray_operator_binary(||, logical_or)

template <typename T, typename I>
inline
std::ostream&
operator<< (std::ostream& output, const DynamicSparseNumberArray<T, I>& a)
{
  // Enclose the entire output in braces
  output << '{';

  std::size_t index_size = a.size();

  // Output the first value from a non-empty set
  // All values are given as ordered (index, value) pairs
  if (index_size)
    output << '(' << a.raw_index(0) << ',' <<
              a.raw_at(0) << ')';

  // Output the comma-separated subsequent values from a non-singleton
  // set
  for (unsigned int i = 1; i < index_size; ++i)
    {
      output << ", (" << a.raw_index(i) << ',' << a.raw_data()[i] << ')';
    }
  output << '}';
  return output;
}


// CompareTypes, RawType, ValueType specializations

#define DynamicSparseNumberArray_comparisons(templatename, settype) \
template<typename T, typename I, bool reverseorder> \
struct templatename<DynamicSparseNumberArray<T,I>, DynamicSparseNumberArray<T,I>, reverseorder> { \
  typedef DynamicSparseNumberArray<T,I> supertype; \
}; \
 \
template<typename T, typename T2, typename I, typename I2, bool reverseorder> \
struct templatename<DynamicSparseNumberArray<T,I>, DynamicSparseNumberArray<T2,I2>, reverseorder> { \
  typedef DynamicSparseNumberArray<typename Symmetric##templatename<T, T2, reverseorder>::supertype, \
                            typename CompareTypes<I,I2>::supertype> supertype; \
}; \
 \
template<typename T, typename T2, typename I, bool reverseorder> \
struct templatename<DynamicSparseNumberArray<T, I>, T2, reverseorder, \
                    typename boostcopy::enable_if<BuiltinTraits<T2> >::type> { \
  typedef DynamicSparseNumberArray<typename Symmetric##templatename<T, T2, reverseorder>::supertype, I> supertype; \
}

DynamicSparseNumberArray_comparisons(CompareTypes, Union);
DynamicSparseNumberArray_comparisons(PlusType, Union);
DynamicSparseNumberArray_comparisons(MinusType, Union);
DynamicSparseNumberArray_comparisons(MultipliesType, Intersection);
DynamicSparseNumberArray_comparisons(DividesType, First);
DynamicSparseNumberArray_comparisons(AndType, Intersection);
DynamicSparseNumberArray_comparisons(OrType, Union);


template <typename T, typename I>
struct RawType<DynamicSparseNumberArray<T, I> >
{
  typedef DynamicSparseNumberArray<typename RawType<T>::value_type, I> value_type;

  static value_type value(const DynamicSparseNumberArray<T, I>& a)
    {
      value_type returnval;
      returnval.nude_indices() = a.nude_indices();

      std::size_t index_size = a.size();
      returnval.nude_data().resize(index_size);

      for (unsigned int i=0; i != index_size; ++i)
        returnval.raw_at(i) = RawType<T>::value(a.raw_at(i));
      return returnval;
    }
};

template <typename T, typename I>
struct ValueType<DynamicSparseNumberArray<T, I> >
{
  typedef typename ValueType<T>::type type;
};

} // namespace MetaPhysicL


namespace std {

using MetaPhysicL::CompareTypes;
using MetaPhysicL::DynamicSparseNumberArray;
using MetaPhysicL::SymmetricCompareTypes;

#define DynamicSparseNumberArray_std_unary(funcname) \
template <typename T, typename I> \
inline \
DynamicSparseNumberArray<T, I> \
funcname (DynamicSparseNumberArray<T, I> a) \
{ \
  std::size_t index_size = a.size(); \
  for (unsigned int i=0; i != index_size; ++i) \
    a.raw_at(i) = std::funcname(a.raw_at(i)); \
 \
  return a; \
}


#define DynamicSparseNumberArray_std_binary_union(funcname) \
template <typename T, typename T2, typename I, typename I2> \
inline \
DynamicSparseNumberArray<typename SymmetricCompareTypes<T,T2>::supertype, \
                         typename CompareTypes<I,I2>::supertype> \
funcname (const DynamicSparseNumberArray<T, I>& a, \
          const DynamicSparseNumberArray<T2, I2>& b) \
{ \
  typedef typename SymmetricCompareTypes<T,T2>::supertype TS; \
  typedef typename CompareTypes<I,I2>::supertype IS; \
  DynamicSparseNumberArray<TS, IS> returnval; \
 \
  std::size_t index_size = a._indices.size(); \
  returnval._indices = a._indices; \
  returnval._data.resize(index_size); \
  returnval.sparsity_union(b._indices); \
 \
  typename std::vector<I>::const_iterator  index_a_it = a._indices.begin(); \
  typename std::vector<I2>::const_iterator index_b_it = b._indices.begin(); \
  typename std::vector<IS>::iterator     index_out_it = returnval._indices.begin(); \
 \
  typename std::vector<T>::const_iterator  data_a_it = a._data.begin(); \
  typename std::vector<T2>::const_iterator data_b_it = b._data.begin(); \
  typename std::vector<TS>::iterator     data_out_it = returnval._data.begin(); \
 \
  IS  maxIS  = std::numeric_limits<IS>::max(); \
 \
  for (; index_out_it != returnval._indices.end(); ++index_out_it, ++data_out_it) { \
    const IS index_a = (index_a_it == a._indices.end()) ? maxIS : *index_a_it; \
    const IS index_b = (index_b_it == b._indices.end()) ? maxIS : *index_b_it; \
    const IS index_out = *index_out_it; \
    const TS data_a  = (index_a_it == a._indices.end()) ? 0: *data_a_it; \
    const TS data_b  = (index_b_it == b._indices.end()) ? 0: *data_b_it; \
    TS &   data_out  = *data_out_it; \
 \
    if (index_a == index_out) { \
      if (index_b == index_out) { \
        data_out = std::funcname(data_a, data_b); \
        index_b_it++; \
        data_b_it++; \
      } else { \
        data_out = std::funcname(data_a, 0); \
      } \
      index_a_it++; \
      data_a_it++; \
    } else { \
      metaphysicl_assert_equal_to(index_b, index_out); \
      data_out = std::funcname(0, data_b); \
      index_b_it++; \
      data_b_it++; \
    } \
  } \
 \
  return returnval; \
} \
 \
template <typename T, typename T2, typename I> \
inline \
DynamicSparseNumberArray<typename SymmetricCompareTypes<T,T2>::supertype, I> \
funcname (const DynamicSparseNumberArray<T, I>& a, const T2& b) \
{ \
  typedef typename SymmetricCompareTypes<T,T2>::supertype TS; \
  DynamicSparseNumberArray<TS, I> returnval; \
 \
  std::size_t index_size = a.size(); \
  returnval.resize(index_size); \
  returnval._indices = a._indices; \
 \
  for (unsigned int i=0; i != index_size; ++i) \
    returnval.raw_at(i) = std::funcname(a.raw_at(i), b); \
 \
  return returnval; \
} \
 \
template <typename T, typename T2, typename I> \
inline \
DynamicSparseNumberArray<typename SymmetricCompareTypes<T,T2>::supertype, I> \
funcname (const T& a, const DynamicSparseNumberArray<T2, I>& b) \
{ \
  typedef typename SymmetricCompareTypes<T,T2>::supertype TS; \
  DynamicSparseNumberArray<TS, I> returnval; \
 \
  std::size_t index_size = a.size(); \
  returnval.resize(index_size); \
  returnval._indices = b._indices; \
 \
  for (unsigned int i=0; i != index_size; ++i) \
    returnval.raw_at(i) = std::funcname(a, b.raw_at(i)); \
 \
  return returnval; \
}


// Pow needs its own specialization, both to avoid being confused by
// pow<T1,T2> and because pow(x,0) isn't 0.
template <typename T, typename T2, typename I>
inline
DynamicSparseNumberArray<typename SymmetricCompareTypes<T,T2>::supertype, I>
pow (const DynamicSparseNumberArray<T, I>& a, const T2& b)
{
  typedef typename SymmetricCompareTypes<T,T2>::supertype TS;
  DynamicSparseNumberArray<TS, I> returnval;

  std::size_t index_size = a.size();
  returnval.nude_indices() = a.nude_indices();
  returnval.nude_data().resize(index_size);

  for (unsigned int i=0; i != index_size; ++i)
    returnval.raw_at(i) = std::pow(a.raw_at(i), b);

  return returnval;
}


// NOTE: unary functions for which f(0) != 0 are undefined compile-time
// errors, because there's no efficient way to have them make sense in
// the sparse context.

// DynamicSparseNumberArray_std_binary(pow) // separate definition
// DynamicSparseNumberArray_std_unary(exp)
// DynamicSparseNumberArray_std_unary(log)
// DynamicSparseNumberArray_std_unary(log10)
DynamicSparseNumberArray_std_unary(sin)
// DynamicSparseNumberArray_std_unary(cos)
DynamicSparseNumberArray_std_unary(tan)
DynamicSparseNumberArray_std_unary(asin)
// DynamicSparseNumberArray_std_unary(acos)
DynamicSparseNumberArray_std_unary(atan)
DynamicSparseNumberArray_std_binary_union(atan2)
DynamicSparseNumberArray_std_unary(sinh)
// DynamicSparseNumberArray_std_unary(cosh)
DynamicSparseNumberArray_std_unary(tanh)
DynamicSparseNumberArray_std_unary(sqrt)
DynamicSparseNumberArray_std_unary(abs)
DynamicSparseNumberArray_std_unary(fabs)
DynamicSparseNumberArray_std_binary_union(max)
DynamicSparseNumberArray_std_binary_union(min)
DynamicSparseNumberArray_std_unary(ceil)
DynamicSparseNumberArray_std_unary(floor)
DynamicSparseNumberArray_std_binary_union(fmod) // TODO: optimize this


template <typename T, typename I>
class numeric_limits<DynamicSparseNumberArray<T, I> > :
  public MetaPhysicL::raw_numeric_limits<DynamicSparseNumberArray<T, I>, T> {};

} // namespace std


#endif // METAPHYSICL_DYNAMICSPARSENUMBERARRAY_H
