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
// $Id$
//
//--------------------------------------------------------------------------


#ifndef METAPHYSICL_GPUVECTOR_H
#define METAPHYSICL_GPUVECTOR_H

#include <algorithm>
#include <limits>
#include <ostream>

#include "metaphysicl/compare_types.h"
#include "metaphysicl/raw_type.h"

namespace MetaPhysicL {

template <typename T>
class GPUVector;

template<typename S, typename T, bool reverseorder>
struct DotType<GPUVector<S>, GPUVector<T>, reverseorder> {
  typedef typename MultipliesType<S,T,reverseorder>::supertype supertype;
};

// No nesting support yet!
/*
template<typename S, typename T, bool reverseorder>
struct OuterProductType<GPUVector<S>, GPUVector<T>, reverseorder> {
  typedef 
  GPUVector<GPUVector<
    typename MultipliesType<S,T,reverseorder>::supertype> > supertype;
};
*/

template<typename S>
struct SumType<GPUVector<S> > {
  typedef S supertype;
};

template <typename T>
class GPUVector
{
public:
  typedef T value_type;

  template <std::size_t i>
  struct entry_type {
    typedef value_type type;
  };

  template <typename T2>
  struct rebind {
    typedef GPUVector<T2> other;
  };

  // Not useful without a specified size
  // GPUVector() {}

  GPUVector(std::size_t N, const T& data) : data(N, data) {}

  /*
  // Not defineable without a specified size
  GPUVector(const T& val)
  */

  /*
  // Not defineable without a specified size
  GPUVector(const T* vals)
  */

  template <typename T2>
  GPUVector(GPUVector<T2> src) : _data (src._data) {}

  /*
  // Not defineable without a specified size
  template <typename T2>
  GPUVector(const T2& val)
  */

  T& operator[](std::size_t i)
    { return _data[i]; }

  const T& operator[](std::size_t i) const
    { return _data[i]; }

  template <std::size_t i>
  typename entry_type<i>::type& get()
    { return _data[i]; }

  template <std::size_t i>
  const typename entry_type<i>::type& get() const
    { return _data[i]; }

  std::size_t size() const
    { return _data.size(); }

  GPUVector<T> operator! () const {
    return GPUVector<T>
      (thrust::make_transform_iterator(_data.begin(), thrust::logical_not<T>()),
       thrust::make_transform_iterator(_data.end(), thrust::logical_not<T>()));
  }

  template <typename T2>
  GPUVector<T>& operator+= (const GPUVector<T2>& a) {
    thrust::transform(_data.begin(), _data.end(), a._data.begin(),
                      _data.begin(), thrust::plus<T>());
    return *this;
  }

  template <typename T2>
  GPUVector<T>& operator+= (const T2& a) { 
    thrust::transform(_data.begin(), _data.end(),
		      thrust::constant_iterator<T>(a),
                      _data.begin(), thrust::plus<T>());
    return *this;
  }


  template <typename T2>
  GPUVector<T>& operator-= (const GPUVector<T2>& a) {
    thrust::transform(_data.begin(), _data.end(), a._data.begin(),
                      _data.begin(), thrust::minus<T>());
    return *this;
  }

  template <typename T2>
  GPUVector<T>& operator-= (const T2& a) {
    thrust::transform(_data.begin(), _data.end(),
		      thrust::constant_iterator<T>(a),
                      this->begin(), thrust::minus<T>());
    return *this;
  }

  template <typename T2>
  GPUVector<T>& operator*= (const GPUVector<T2>& a) {
    thrust::transform(_data.begin(), _data.end(), a._data.begin(),
                      _data.begin(), thrust::multiplies<T>());
    return *this;
  }

  template <typename T2>
  GPUVector<T>& operator*= (const T2& a) {
    thrust::transform(_data.begin(), _data.end(),
		      thrust::constant_iterator<T>(a),
                      _data.begin(), thrust::multiplies<T>());
    return *this;
  }

  template <typename T2>
  GPUVector<T>& operator/= (const GPUVector<T2>& a) {
    thrust::transform(_data.begin(), _data.end(), a._data.begin(),
                      _data.begin(), thrust::divides<T>());
    return *this;
  }

  template <typename T2>
  GPUVector<T>& operator/= (const T2& a) {
    thrust::transform(_data.begin(), _data.end(),
		      thrust::constant_iterator<T>(a),
                      _data.begin(), thrust::divides<T>());
    return *this;
  }

  template <typename T2>
  typename SymmetricMultipliesType<T,T2>::supertype
  dot (const GPUVector<T2>& a) const
  {
    typedef typename SymmetricMultipliesType<T,T2>::supertype
		    returntype;
    returntype init = 0;

    return thrust::inner_product(_data.begin(), _data.end(),
				 a._data.begin(), init);
  }

// No nesting support yet!
  /*
  template <typename T2>
  GPUVector<N, GPUVector<N, typename SymmetricMultipliesType<T,T2>::supertype> >
  outerproduct (const GPUVector<N,T2>& a) const
  {
    GPUVector<N, GPUVector<N, typename SymmetricMultipliesType<T,T2>::supertype> > returnval;

    for (std::size_t i=0; i != N; ++i)
      for (std::size_t j=0; j != N; ++j)
        returnval[i][j] = _data[i] * a[j];

    return returnval;
  }

  static GPUVector<N, GPUVector<N, T> > identity()
  {
    GPUVector<N, GPUVector<N, T> > returnval(0);
  
    for (std::size_t i=0; i != N; ++i)
      returnval[i][i] = 1;

    return returnval;
  }
  */

  T sum () const
  {
    return thrust::reduce(_data.begin(), _data.end());
  }


private:
  thrust::device_vector<T> _data;
};



//
// Non-member functions
//

template <std::size_t N,
          unsigned int index1=0, typename Data1=void,
          unsigned int index2=0, typename Data2=void,
          unsigned int index3=0, typename Data3=void,
          unsigned int index4=0, typename Data4=void,
          unsigned int index5=0, typename Data5=void,
          unsigned int index6=0, typename Data6=void,
          unsigned int index7=0, typename Data7=void,
          unsigned int index8=0, typename Data8=void>
struct GPUVectorOf
{
  typedef
  typename CompareTypes<Data1,
    typename CompareTypes<Data2,
      typename CompareTypes<Data3,
        typename CompareTypes<Data4,
          typename CompareTypes<Data5,
            typename CompareTypes<Data6,
              typename CompareTypes<Data7,Data8>::supertype
            >::supertype
          >::supertype
        >::supertype
      >::supertype
    >::supertype
  >::supertype supertype;

  typedef GPUVector<supertype> type;
};



template <std::size_t N, std::size_t index, typename T>
struct GPUVectorUnitVector
{
  typedef GPUVector<T> type;

  static const type value() {
    type returnval(N,0);
    returnval[index] = 1;
    return returnval;
  }
};


template <std::size_t N, typename T>
struct GPUVectorFullVector
{
  typedef GPUVector<T> type;

  static const type value() {
    return type(N,1);
  }
};


/*
// No nesting support yet!
template <std::size_t N, typename T>
inline
GPUVector<N, GPUVector<N, T> >
transpose(GPUVector<N, GPUVector<N, T> > a)
{
  for (std::size_t i=0; i != N; ++i)
    for (std::size_t j=i+1; j != N; ++j)
      std::swap(a[i][j], a[j][i]);

  return a;
}
*/



#define GPUVector_op_ab(atype, btype, newtype) \
template <std::size_t N, typename T, typename T2> \
inline \
typename newtype::supertype \
operator opname (const atype& a, const btype& b) \
{ \
  typedef typename newtype::supertype TS; \
  TS returnval(a); \
  returnval opname##= b; \
  return returnval; \
}

#define GPUVector_op(opname, typecomparison) \
GPUVector_op_ab(opname, GPUVector<T>, GPUVector<T2>, \
                  typecomparison##Type<GPUVector<T> MacroComma GPUVector<T2> >) \
GPUVector_op_ab(opname,           T , GPUVector<T2>, \
                  typecomparison##Type<GPUVector<T2> MacroComma T MacroComma true>) \
GPUVector_op_ab(opname, GPUVector<T>,           T2 , \
                  typecomparison##Type<GPUVector<T> MacroComma T2>)

GPUVector_op(+,Plus)
GPUVector_op(-,Minus)
GPUVector_op(*,Multiplies)
GPUVector_op(/,Divides)


#define GPUVector_operator_binary_abab(opname, atype, btype, aarg, barg) \
template <typename T, typename T2> \
inline \
GPUVector<bool> \
operator opname (const atype& a, const btype& b) \
{ \
  GPUVector<bool> returnval(a.size(), false); \
 \
  for (std::size_t i=0; i != N; ++i) \
    returnval[i] = (aarg opname barg); \
 \
  return returnval; \
}

#define GPUVector_operator_binary(opname) \
GPUVector_operator_binary_abab(opname, GPUVector<T>, GPUVector<T2>, a[i], b[i]) \
GPUVector_operator_binary_abab(opname,           T , GPUVector<T2>, a,    b[i]) \
GPUVector_operator_binary_abab(opname, GPUVector<T>,           T2 , a[i], b)

GPUVector_operator_binary(<)
GPUVector_operator_binary(<=)
GPUVector_operator_binary(>)
GPUVector_operator_binary(>=)
GPUVector_operator_binary(==)
GPUVector_operator_binary(!=)
GPUVector_operator_binary(&&)
GPUVector_operator_binary(||)

template <typename T>
inline
std::ostream&      
operator<< (std::ostream& output, const GPUVector<T>& a)
{
  output << '{';
  const std::size_t N = a.size();
  if (N)
    output << a[0];
  for (std::size_t i=1; i<N; ++i)
    output << ',' << a[i];
  output << '}';
  return output;
}


// CompareTypes, RawType, ValueType specializations

#define GPUVector_comparisons(templatename) \
template<typename T, bool reverseorder> \
struct templatename<GPUVector<T>, GPUVector<T>, reverseorder> { \
  typedef GPUVector<T> supertype; \
}; \
 \
template<typename T, typename T2, bool reverseorder> \
struct templatename<GPUVector<T>, GPUVector<T2>, reverseorder> { \
  typedef GPUVector<typename Symmetric##templatename<T, T2, reverseorder>::supertype> supertype; \
}; \
 \
template<typename T, typename T2, bool reverseorder> \
struct templatename<GPUVector<T>, T2, reverseorder, \
                    typename boostcopy::enable_if<BuiltinTraits<T2> >::type> { \
  typedef GPUVector<typename Symmetric##templatename<T, T2, reverseorder>::supertype> supertype; \
}

GPUVector_comparisons(CompareTypes);
GPUVector_comparisons(PlusType);
GPUVector_comparisons(MinusType);
GPUVector_comparisons(MultipliesType);
GPUVector_comparisons(DividesType);
GPUVector_comparisons(AndType);
GPUVector_comparisons(OrType);

template <typename T>
struct RawType<GPUVector<T> >
{
  typedef GPUVector<typename RawType<T>::value_type> value_type;

  static value_type value(const GPUVector<T>& a)
    {
      value_type returnval(a.size(),0);
      thrust::transform(a._data.begin(), a._data.end(),
			returnval._data.begin(),
			ptr_fun(RawType<T>::value));
      return returnval;
    }
};

template <typename T>
struct ValueType<GPUVector<T> >
{
  typedef typename ValueType<T>::type type;
};

} // namespace MetaPhysicL


namespace std {

using MetaPhysicL::GPUVector;
using MetaPhysicL::CompareTypes;

#define GPUVector_std_unary(funcname) \
template <typename T> \
inline \
GPUVector<T> \
funcname (const GPUVector<T> &a) \
{ \
  GPUVector<T> returnval(a.size(),0); \
  thrust::transform(a._data.begin(), a._data.end(), \
                    returnval._data.begin(), \
                    ptr_fun(std::funcname)); \
 \
  return a; \
}


#define GPUVector_std_binary_abab(funcname, atype, btype, abtypes, aarg, barg) \
template <typename T, typename T2> \
inline \
typename CompareTypes<abtypes>::supertype \
funcname (const atype& a, const btype& b) \
{ \
  typedef typename CompareTypes<abtypes>::supertype TS; \
  TS returnval(a.size(), 0); \
 \
  thrust::transform(a._data.begin(), a._data.end(), \
                    b._data.begin(), \
                    returnval._data.begin(), \
                    ptr_fun(std::funcname)); \
 \
  return returnval; \
}

#define GPUVector_std_binary_aa(funcname, atype) \
template <typename T> \
inline \
atype \
funcname (const atype& a, const atype& b) \
{ \
  atype returnval; \
  thrust::transform(a._data.begin(), a._data.end(), \
                    b._data.begin(), \
                    returnval._data.begin(), \
                    ptr_fun(std::funcname)); \
 \
  return returnval; \
}


#define GPUVector_std_binary(funcname) \
GPUVector_std_binary_abab(funcname, GPUVector<T>, GPUVector<T2>, \
                          GPUVector<T>  MacroComma GPUVector<T2>, a[i], b[i]) \
GPUVector_std_binary_abab(funcname,           T , GPUVector<T2>, \
                          GPUVector<T2> MacroComma T,             a,    b[i]) \
GPUVector_std_binary_abab(funcname, GPUVector<T>,           T2 , \
                          GPUVector<T> MacroComma T2,             a[i],    b) \
GPUVector_std_binary_aa(funcname, GPUVector<T>)

GPUVector_std_binary(pow)
GPUVector_std_unary(exp)
GPUVector_std_unary(log)
GPUVector_std_unary(log10)
GPUVector_std_unary(sin)
GPUVector_std_unary(cos)
GPUVector_std_unary(tan)
GPUVector_std_unary(asin)
GPUVector_std_unary(acos)
GPUVector_std_unary(atan)
GPUVector_std_binary(atan2)
GPUVector_std_unary(sinh)
GPUVector_std_unary(cosh)
GPUVector_std_unary(tanh)
GPUVector_std_unary(sqrt)
GPUVector_std_unary(abs)
GPUVector_std_unary(fabs)
GPUVector_std_binary(max)
GPUVector_std_binary(min)
GPUVector_std_unary(ceil)
GPUVector_std_unary(floor)
GPUVector_std_binary(fmod)


template <typename T>
class numeric_limits<GPUVector<T> > : 
  public MetaPhysicL::raw_numeric_limits<GPUVector<T>, T> {};

} // namespace std


#endif // METAPHYSICL_GPUVECTOR_H
