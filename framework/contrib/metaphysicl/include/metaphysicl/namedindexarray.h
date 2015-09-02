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


#ifndef METAPHYSICL_NAMEDINDEXARRAY_H
#define METAPHYSICL_NAMEDINDEXARRAY_H

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <ostream>
#include <type_traits>

#include "metaphysicl/compare_types.h"
#include "metaphysicl/ct_set.h"
#include "metaphysicl/metaprogramming.h"
#include "metaphysicl/metaphysicl_asserts.h"
#include "metaphysicl/raw_type.h"

namespace MetaPhysicL {

template <typename DataType, typename SizeType, typename PermType>
const typename boostcopy::enable_if<
  BuiltinTraits<DataType>, DataType&>::type
reshape(DataType& data,
        const SizeType& sizes,
        const PermType& /* perm */)
{
#ifndef NDEBUG
  for (unsigned int i = 0; i != sizes.size(); ++i)
    metaphysicl_assert_equal_to(sizes[i], 1);
#endif
  return data;
}

template <typename DataType, typename SizeType, typename PermType>
const typename boostcopy::enable_if<
  BuiltinTraits<DataType>, const DataType&>::type
reshape(const DataType& data,
        const SizeType& sizes,
        const PermType& /* perm */)
{
#ifndef NDEBUG
  for (unsigned int i = 0; i != sizes.size(); ++i)
    metaphysicl_assert_equal_to(sizes[i], 1);
#endif
  return data;
}

template <typename DataVector, typename SparseSizeVector>
class NamedIndexArray
{
public:
  typedef typename SparseSizeVector::index_set index_set;

  typedef typename index_set::head_type index_type;

  static const size_t index_size = index_set::size;

  std::size_t size() const
    { return _data_vector.size(); }

  NamedIndexArray() :
    _data_vector(), _size_vector() {}

  NamedIndexArray(DataVector vec_in, SparseSizeVector size_in) :
    _data_vector(vec_in), _size_vector(size_in) {}

  template <typename DataVector2, typename SparseSizeVector2>
  NamedIndexArray(const NamedIndexArray<DataVector2,SparseSizeVector2>& arr_in) :
    _data_vector(arr_in.raw_data()), _size_vector(arr_in.raw_sizes()) {}

  template <typename DataVector2, typename SparseSizeVector2>
  NamedIndexArray<DataVector, SparseSizeVector>&
  operator= (const NamedIndexArray<DataVector2,SparseSizeVector2>& arr_in) {
    _data_vector = arr_in.raw_data();
    _size_vector = arr_in.raw_sizes();
    return *this;
  }

  DataVector& raw_data()
    { return _data_vector; }

  const DataVector& raw_data() const
    { return _data_vector; }

  SparseSizeVector& raw_sizes()
    { return _size_vector; }

  const SparseSizeVector& raw_sizes() const
    { return _size_vector; }

  auto
  operator! () const 
  -> NamedIndexArray <decltype(!this->raw_data()), SparseSizeVector>
  {
    return NamedIndexArray
      <decltype(!_data_vector), SparseSizeVector>
        (!_data_vector, _size_vector);
  }

  auto
  operator- () const 
  -> NamedIndexArray <decltype(-this->raw_data()), SparseSizeVector>
  {
    return NamedIndexArray
      <decltype(-_data_vector), SparseSizeVector>
        (-_data_vector, _size_vector);
  }


#define NamedIndexArray_opequals(opname) \
  template <typename DataVector2, typename SparseSizeVector2> \
  NamedIndexArray<DataVector,SparseSizeVector>& \
    operator opname (const NamedIndexArray<DataVector2,SparseSizeVector2>& a) { \
    typedef typename SparseSizeVector::index_set IndexSet; \
    typedef typename SparseSizeVector2::index_set IndexSet2; \
    using MetaPhysicL::PermutationArray; \
    ctassert<IndexSet2::template Difference<IndexSet>::type::size == 0>::apply(); \
    _data_vector opname reshape(a.raw_data(), \
                            _size_vector.raw_data_array(), \
                            PermutationArray<IndexSet2,IndexSet>::value()); \
    return *this; \
  } \
 \
  template <typename T2> \
  NamedIndexArray<DataVector,SparseSizeVector>& \
  operator opname (const T2& a) \
    { _data_vector opname a; return *this; }

  NamedIndexArray_opequals(+=)
  NamedIndexArray_opequals(-=)
  NamedIndexArray_opequals(*=)
  NamedIndexArray_opequals(/=)

private:
  DataVector       _data_vector;
  SparseSizeVector _size_vector;
};


//
// Non-member functions
//


#define NamedIndexArray_op_ab(opname) \
template <typename DataVector, typename DataVector2, \
          typename SparseSizeVector, typename SparseSizeVector2> \
inline \
auto \
operator opname (const NamedIndexArray<DataVector,  SparseSizeVector>&  a, \
                 const NamedIndexArray<DataVector2, SparseSizeVector2>& b) \
{ \
  typedef typename SparseSizeVector::index_set IndexSet; \
  typedef typename SparseSizeVector2::index_set IndexSet2; \
  typedef typename IndexSet2::template Union<IndexSet>::type UnionSet; \
  using std::max; \
  using MetaPhysicL::PermutationArray; \
  const auto final_sizes = max(a.raw_sizes(), b.raw_sizes()); \
  return NamedIndexArray<decltype( \
    reshape(a.raw_data(), \
            final_sizes.raw_data_array(), \
            PermutationArray<IndexSet,UnionSet>::value()) \
    opname \
    reshape(b.raw_data(), \
            final_sizes.raw_data_array(), \
            PermutationArray<IndexSet2,UnionSet>::value())), \
    typename std::remove_const<decltype(final_sizes)>::type> (  \
    reshape(a.raw_data(), \
            final_sizes.raw_data_array(), \
            PermutationArray<IndexSet,UnionSet>::value()) \
    opname \
    reshape(b.raw_data(), \
            final_sizes.raw_data_array(), \
            PermutationArray<IndexSet2,UnionSet>::value()), \
    final_sizes); \
} \
 \
template <typename DataVector, typename SparseSizeVector, typename T2> \
inline \
auto \
operator opname (const NamedIndexArray<DataVector, SparseSizeVector>& a, \
                 const T2& b) \
-> typename enable_if_c<DefinesSupertype<CompareTypes< \
         NamedIndexArray<DataVector, SparseSizeVector>, \
         T2 \
       > \
     >::value, \
     NamedIndexArray<decltype(a.raw_data() opname b), SparseSizeVector> \
   >::type \
{ \
  return NamedIndexArray<decltype(a.raw_data() opname b), SparseSizeVector> \
    (a.raw_data() opname b, a.raw_sizes()); \
} \
\
template <typename DataVector, typename SparseSizeVector, typename T> \
inline \
auto \
operator opname (const T& a, \
                 const NamedIndexArray<DataVector, SparseSizeVector>& b) \
-> typename enable_if_c<DefinesSupertype<CompareTypes< \
         NamedIndexArray<DataVector, SparseSizeVector>, \
         T \
       > \
     >::value, \
     NamedIndexArray<decltype(a opname b.raw_data()), SparseSizeVector> \
   >::type \
{ \
  return NamedIndexArray<decltype(a opname b.raw_data()), SparseSizeVector> \
    (a opname b.raw_data(), b.raw_sizes()); \
}

NamedIndexArray_op_ab(+)
NamedIndexArray_op_ab(-)
NamedIndexArray_op_ab(*)
NamedIndexArray_op_ab(/)
NamedIndexArray_op_ab(<)
NamedIndexArray_op_ab(<=)
NamedIndexArray_op_ab(>)
NamedIndexArray_op_ab(>=)
NamedIndexArray_op_ab(==)
NamedIndexArray_op_ab(!=)


template <typename DataVector, typename SparseSizeVector>
inline
std::ostream&      
operator<< (std::ostream& output,
            const NamedIndexArray<DataVector, SparseSizeVector>& a)
{
  output << "Sizes: " << a.raw_sizes() << '\n';
  output << "Data: " << a.raw_data() << '\n';
  return output;
}

// ScalarTraits, RawType, CompareTypes specializations

template <typename DataVector, typename SparseSizeVector>
struct ScalarTraits<NamedIndexArray<DataVector, SparseSizeVector> >
{
  static const bool value = ScalarTraits<DataVector>::value;
};

template <typename DataVector, typename SparseSizeVector>
struct RawType<NamedIndexArray<DataVector, SparseSizeVector> >
{
  typedef typename RawType<DataVector>::value_type value_type;

  static value_type value(const NamedIndexArray<DataVector, SparseSizeVector>& a)
  { return raw_value(a.raw_data()); }
};

// NamedIndexArray CompareTypes values aren't accurate, but we can
// still test for their existence to determine what types are
// comparable
template <typename DataVector, typename SparseSizeVector, typename T2, bool reverseorder>
struct CompareTypes<NamedIndexArray<DataVector, SparseSizeVector>, T2, reverseorder,
                    typename boostcopy::enable_if<BuiltinTraits<T2> >::type> {
  typedef bool supertype;
};

template <typename DataVector, typename SparseSizeVector,
          typename DataVector2, typename SparseSizeVector2, bool reverseorder>
struct CompareTypes<NamedIndexArray<DataVector, SparseSizeVector>,
                    NamedIndexArray<DataVector2, SparseSizeVector2>,
                    reverseorder> {
  typedef bool supertype;
};

template <typename DataVector, typename SparseSizeVector, bool reverseorder>
struct CompareTypes<NamedIndexArray<DataVector, SparseSizeVector>,
                    NamedIndexArray<DataVector, SparseSizeVector>,
                    reverseorder> {
  typedef bool supertype;
};



template <typename DataVector, typename SparseSizeVector>
struct
copy_or_reference<NamedIndexArray<DataVector, SparseSizeVector> &>
{
  typedef typename
    IfElse<copy_or_reference<DataVector&>::copy,
           NamedIndexArray<DataVector, SparseSizeVector>,
           NamedIndexArray<DataVector, SparseSizeVector>&>::type type;

  static const bool copy = copy_or_reference<DataVector&>::copy;
};


template <typename DataVector, typename SparseSizeVector>
struct
copy_or_reference<const NamedIndexArray<DataVector, SparseSizeVector> &>
{
  typedef typename
    IfElse<copy_or_reference<DataVector&>::copy,
           NamedIndexArray<DataVector, SparseSizeVector>,
           const NamedIndexArray<DataVector, SparseSizeVector>&>::type type;

  static const bool copy = copy_or_reference<DataVector&>::copy;
};



} // namespace MetaPhysicL


namespace std {

using MetaPhysicL::NamedIndexArray;
using MetaPhysicL::CompareTypes;
using MetaPhysicL::enable_if_c;
using MetaPhysicL::DefinesSupertype;

#define NamedIndexArray_std_unary(funcname) \
template <typename DataVector, typename SparseSizeVector> \
inline \
auto \
funcname (const NamedIndexArray<DataVector, SparseSizeVector> &a) \
{ \
  return NamedIndexArray<decltype(std::funcname(a.raw_data())), \
                         SparseSizeVector> \
    (std::funcname(a.raw_data()), a.raw_sizes()); \
}


#define NamedIndexArray_std_binary(funcname) \
template <typename DataVector, typename DataVector2, \
          typename SparseSizeVector, typename SparseSizeVector2> \
inline \
auto \
funcname (const NamedIndexArray<DataVector , SparseSizeVector >& a, \
          const NamedIndexArray<DataVector2, SparseSizeVector2>& b) \
{ \
  typedef typename SparseSizeVector::index_set IndexSet; \
  typedef typename SparseSizeVector2::index_set IndexSet2; \
  typedef typename IndexSet2::template Union<IndexSet>::type UnionSet; \
  using std::max; \
  const auto final_sizes = max(a.raw_sizes(), b.raw_sizes()); \
 \
  using std::funcname; \
  using MetaPhysicL::PermutationArray; \
  return NamedIndexArray<decltype( \
funcname( \
    reshape(a.raw_data(), \
            final_sizes, \
            PermutationArray<IndexSet,UnionSet>::value()), \
    reshape(b.raw_data(), \
            final_sizes, \
            PermutationArray<IndexSet2,UnionSet>::value()))), \
                         decltype(final_sizes)> (\
funcname( \
    reshape(a.raw_data(), \
            final_sizes, \
            PermutationArray<IndexSet,UnionSet>::value()), \
    reshape(b.raw_data(), \
            final_sizes, \
            PermutationArray<IndexSet2,UnionSet>::value())) \
, final_sizes); \
} \
 \
template <typename DataVector, typename SparseSizeVector, typename T2> \
inline \
auto \
funcname (const NamedIndexArray<DataVector, SparseSizeVector>& a, \
          const T2& b) \
-> typename enable_if_c<DefinesSupertype<CompareTypes< \
         NamedIndexArray<DataVector, SparseSizeVector>, \
         T2 \
       > \
     >::value, \
     NamedIndexArray<decltype(funcname(a.raw_data(), b)), SparseSizeVector> \
   >::type \
{ \
  using std::funcname; \
  return NamedIndexArray<decltype(funcname(a.raw_data(), b)), SparseSizeVector> \
    (funcname(a.raw_data(), b), a.raw_sizes()); \
} \
 \
template <typename DataVector, typename SparseSizeVector, typename T> \
inline \
auto \
funcname (const T& a, \
          const NamedIndexArray<DataVector, SparseSizeVector>& b) \
-> typename enable_if_c<DefinesSupertype<CompareTypes< \
         NamedIndexArray<DataVector, SparseSizeVector>, \
         T \
       > \
     >::value, \
     NamedIndexArray<decltype(funcname(a, b.raw_data())), SparseSizeVector> \
   >::type \
{ \
  using std::funcname; \
  return NamedIndexArray<decltype(funcname(a, b.raw_data())), SparseSizeVector> \
    (funcname(a, b.raw_data()), b.raw_sizes()); \
}


NamedIndexArray_std_binary(pow)
NamedIndexArray_std_unary(exp)
NamedIndexArray_std_unary(log)
NamedIndexArray_std_unary(log10)
NamedIndexArray_std_unary(sin)
NamedIndexArray_std_unary(cos)
NamedIndexArray_std_unary(tan)
NamedIndexArray_std_unary(asin)
NamedIndexArray_std_unary(acos)
NamedIndexArray_std_unary(atan)
NamedIndexArray_std_binary(atan2)
NamedIndexArray_std_unary(sinh)
NamedIndexArray_std_unary(cosh)
NamedIndexArray_std_unary(tanh)
NamedIndexArray_std_unary(sqrt)
NamedIndexArray_std_unary(abs)
NamedIndexArray_std_unary(fabs)
NamedIndexArray_std_binary(max)
NamedIndexArray_std_binary(min)
NamedIndexArray_std_unary(ceil)
NamedIndexArray_std_unary(floor)
NamedIndexArray_std_binary(fmod)


template <typename DataVector, typename SparseSizeVector>
class numeric_limits<NamedIndexArray<DataVector, SparseSizeVector> > : 
  public MetaPhysicL::raw_numeric_limits<NamedIndexArray<DataVector, SparseSizeVector>, DataVector> {};

} // namespace std


#endif // METAPHYSICL_NAMEDINDEXARRAY_H
