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


#ifndef METAPHYSICL_CT_SET_H
#define METAPHYSICL_CT_SET_H

#include <stdexcept>

#include "metaphysicl/compare_types.h"
#include "metaphysicl/ct_types.h"

#if  __cplusplus >= 201103L
#include <array>
#endif

// Compile-time editable list / set / map data structure

namespace MetaPhysicL {

// Forward Declarations
template <typename HeadType>
struct NullContainer;

template <typename T>
struct is_null_container
{
  const static bool value = false;
};

template <typename HeadType>
struct is_null_container<NullContainer<HeadType> >
{
  const static bool value = true;
};

// IntType is used as the equivalent of int i in a set<int> or
// vector<int>, or as the equivalent of pair<int,DataType> in a
// map<int,DataType>
template <int i, typename DataType=NullType>
struct IntType
{
  typedef int value_type;
  static const value_type value = i;

  typedef DataType data_type;
  DataType data;

  // name returns a string making it easier to print int container contents.
  std::string name() { std::stringstream ss; ss << value; return ss.str(); }

  // rebind creates another IntType with the same int but different data.
  // This is useful for "upgrading" the contents of containers.
  template <typename DataType2>
  struct rebind {
    typedef IntType<i, DataType2> other;
  };
};

// UnsignedIntType is used as the equivalent of unsigned int i in a
// set<unsigned int> or vector<unsigned int>, or as the equivalent of
// pair<unsigned int,DataType> in a map<unsigned int,DataType>
template <unsigned int i, typename DataType=NullType>
struct UnsignedIntType
{
  typedef unsigned int value_type;
  static const value_type value = i;

  typedef DataType data_type;
  DataType data;

  // name returns a string making it easier to print int container contents.
  static std::string name() { std::stringstream ss; ss << value; return ss.str(); }

  // rebind creates another UnsignedIntType with the same uint but different
  // data.
  // This is useful for "upgrading" the contents of containers.
  template <typename DataType2>
  struct rebind {
    typedef UnsignedIntType<i, DataType2> other;
  };
};


template <long unsigned int i, typename DataType=NullType>
struct UnsignedLongType
{
  typedef long unsigned int value_type;
  static const value_type value = i;

  typedef DataType data_type;
  DataType data;

  // name returns a string making it easier to print int container contents.
  static std::string name() { std::stringstream ss; ss << value; return ss.str(); }

  // rebind creates another UnsignedLongType with the same ulong but
  // different data.
  // This is useful for "upgrading" the contents of containers.
  template <typename DataType2>
  struct rebind {
    typedef UnsignedLongType<i, DataType2> other;
  };
};


// CombinedType gets defined to enable type upgrading in
// Container::Insert.  It needs to be defined for any pair of types,
// but only needs to make sense for those types which compare
// equivalent.
template <typename S, typename T>
struct CombinedType
{
  typedef S type;
};

template <int I, typename S, typename T>
struct CombinedType<IntType<I,S>, T>
{
  typedef IntType<I, 
    typename SymmetricCompareTypes<
      S, typename T::data_type
    >::supertype
  > type;
};

template <int I, typename S, typename T>
struct CombinedType<UnsignedIntType<I,S>, T>
{
  typedef UnsignedIntType<I, 
    typename SymmetricCompareTypes<
      S, typename T::data_type
    >::supertype
  > type;
};

// ValueLessThan is a comparison functor which uses operator< to
// produce an ordering of any two types that have static const value
// members.
struct ValueLessThan
{
  template <typename S, typename T>
  struct LessThan {
    static const bool value = (S::value < T::value);
  };

  template <typename S, typename T>
  struct Equal {
    static const bool value = (S::value == T::value);
  };
};


// ValueLessThan is a comparison functor which merely checks for
// exact equality between any two types.  Its LessThan member struct
// is defined, but is *not* antisymmetric, so sorting done by
// TypeLessThan will not be well-defined.
struct TypeLessThan
{
  template <typename S, typename T>
  struct LessThan {
    static const bool value = !TypesEqual<S,T>::value;
  };

  template <typename S, typename T>
  struct Equal {
    static const bool value = TypesEqual<S,T>::value;
  };
};


// This Container can be used, depending on what types it is fed, as
// the equivalent of std::set, std::vector, std::map...  It can hold
// heterogenously indexed contents, so long as all the typenames it is
// given have ::value data members comparable by operator<
template <typename HeadType,
          typename TailContainer=NullContainer<HeadType>,
          typename Comparison=ValueLessThan>
struct Container
{
//  typedef typename HeadType::value_type value_type;

//  static const value_type head_value = HeadType::value;

  typedef HeadType      head_type;
  typedef TailContainer tail_set;
  typedef Comparison    comparison;

  // These may be empty types or may have data
  HeadType       head;
  TailContainer  tail;

  // rebind always produces a homogenous data type...
  template <typename DataType2>
  struct rebind {
    typedef Container<typename HeadType::template rebind<DataType2>::other,
                      typename TailContainer::template rebind<DataType2>::other,
                      Comparison> other;
  };
  

  // Container::size holds the number of elements within the Container
  static const std::size_t size = tail_set::size + 1;

  // When considering a Container as a sequence, Container::IndexOf<ValueType>::value
  // gives the index (starting at 0) of ValueType::value within that sequence
  template <typename ValueType>
  struct IndexOf
  {
    static const int index = (Comparison::template Equal<ValueType, head_type>::value) ?
      0 : (tail_set::template IndexOf<ValueType>::index + 1); 
  };


  // When considering a Container as a *sorted* sequence whose key types have
  // values, Container::runtime_index_of(t) gives the index (starting at 0)
  // of the value t within that sequence.

  // FIXME: This is a linear search; it needs to be replaced with a
  // binary search!

  // FIXME: Ought to assert(sorted) here
  template <typename T>
  inline static unsigned int runtime_index_of(T& t) {
    if (head_type::value < t)
      return (tail_set::runtime_index_of(t) + 1);
#ifndef NDEBUG
    if (t != head_type::value)
      throw std::domain_error("Container::runtime_index_of argument not found");
#endif
    return 0;
  }


  // When considering a Container as a sequence,
  // Container::runtime_value_of(t) gives the value at the index i
  // (starting at 0) within that sequence.

  // FIXME: this is an O(N) linear search; it should be replaced with
  // an O(1) table lookup!

  // Warning: this will fail with heterogenous value_type Containers

/*
  template <typename T>
  static typename head_type::value_type 
  runtime_value_of(const T& i) {
#ifndef NDEBUG
    if (i >= size)
      throw std::domain_error("Container::runtime_value_of index is out of bounds");
#endif
    if (i)
      return tail_set::runtime_value_of(i-1);
    return head_type::value;
  }
*/


  struct HeadOf {
    static const HeadType& value(const Container& s) {
      return s.head;
    }

    static HeadType& value(Container& s) {
      return s.head;
    }
  };


  struct ContainerOf {
    static Container& value(Container& s) { return s; }

    static const Container& value(const Container& s) { return s; }
  };


  struct TailOf {
    static TailContainer& value(Container& s) { return s.tail; }

    static const TailContainer& value(const Container& s) { return s.tail; }
  };


  template <typename ValueType>
  struct ElementOf {
    typedef typename 
      IfElse<
        (Comparison::template Equal<ValueType,head_type>::value),
        HeadType,
        typename TailContainer::template ElementOf<ValueType>::type
      >::type type;

    static const type& value(const Container& s) {
      return IfElse<
        (Comparison::template Equal<ValueType,head_type>::value),
        HeadOf,
        typename TailContainer::template ElementOf<ValueType>
      >::type::value
        (IfElse<
          (Comparison::template Equal<ValueType,head_type>::value),
          ContainerOf, TailOf>::type::value(s));
    }
 
    static type& value(Container& s) {
      return IfElse<
        (Comparison::template Equal<ValueType,head_type>::value),
        HeadOf,
        typename TailContainer::template ElementOf<ValueType>
      >::type::value
        (IfElse<
          (Comparison::template Equal<ValueType,head_type>::value),
          ContainerOf, TailOf>::type::value(s));
    }
  };


  // Given an instantiated Container s, and a ValueType, s.element<ValueType>()
  // returns a (possibly const) reference to the ValueType object contained within s.
  // This is probably only useful if ValueType contains non-static data.
  template <typename ValueType>
  const typename ElementOf<ValueType>::type& element() const {
    return ElementOf<ValueType>::value(*this);
  }

  template <typename ValueType>
  typename ElementOf<ValueType>::type& element() {
    return ElementOf<ValueType>::value(*this);
  }

  // Given an instantiated Container s, and a ValueType, s.data<ValueType>()
  // returns a (possibly const) reference to the ValueType::data
  // object contained within s.
  // This is only defined if ValueType contains a "data" member, and
  // probably only useful if that data is non-static.
  template <typename ValueType>
  const typename ElementOf<ValueType>::type::data_type& data() const {
    return this->element<ValueType>().data;
  }

  template <typename ValueType>
  typename ElementOf<ValueType>::type::data_type& data() {
    return this->element<ValueType>().data;
  }

  // Given an instantiated Container s, and a compile-time-known value,
  // s.var<value>() returns a (possibly const) reference to the data
  // variable contained within s whose ValueType::value matches.
  // This is only defined if ValueType contains a "data" member, and
  // probably only useful if that data is non-static.
  template <unsigned int value>
  const typename ElementOf<UnsignedIntType<value> >::type::data_type& var() const {
    return this->element<UnsignedIntType<value> >().data;
  }

  template <unsigned int value>
  typename ElementOf<UnsignedIntType<value> >::type::data_type& var() {
    return this->element<UnsignedIntType<value> >().data;
  }

  // Given an instantiated Container s, and a value, s.runtime_data(value)
  // returns a (possibly const) reference to the ValueType::data
  // object contained within s with that ValueType::value.
  // This is probably only useful if ValueType constains non-static data.
  //
  // FIXME: this is an O(N) linear search; it should be replaced with
  // an O(log(N) binary search!
  //
  // This will fail entirely in a sufficiently heterogenous Container, so I'm just
  // commenting it out entirely for now.
/*
  const typename HeadType::data_type& runtime_data(const value_type& i) const {
    if (head_value < i)
      return *reinterpret_cast<const typename HeadType::data_type*>(&tail.runtime_data(i));
#ifndef NDEBUG
    if (i < head_value)
      throw std::domain_error("Container::runtime_data argument not found");
#endif
    return head.data;
  }

  typename HeadType::data_type& runtime_data(const value_type& i) {
    if (head_value < i)
      return *reinterpret_cast<typename HeadType::data_type*>(&tail.runtime_data(i));
#ifndef NDEBUG
    if (i < head_value)
      throw std::domain_error("Container::runtime_data argument not found");
#endif
    return head.data;
  }
*/

  // Container::UpgradeType<T> upgrades any data in the Container type
  // from data_type to CompareTypes<T,data_type>::supertype.  This is
  // typically only used indirectly through CompareTypes<Container,T>
  // specializations.
  template <typename T>
  struct UpgradeType
  {
    typedef MetaPhysicL::Container<
      typename HeadType::template rebind<
        typename SymmetricCompareTypes<
          T, typename HeadType::data_type
        >::supertype
      >::other,
      typename SymmetricCompareTypes<T,TailContainer>::supertype,
      Comparison
    > type;
  };


  // Container::Contains<ValueType>::value is true iff the Container
  // has an entry with a value equivalent to ValueType::value
  // Container::Contains<ValueType>::type returns the type in this
  // Container with that value, which may differ from ValueType
  template <typename ValueType>
  struct Contains
  {
    static const bool value = 
      IfElse<
        (Comparison::template Equal<ValueType,head_type>::value),
        TrueType,
        typename tail_set::template Contains<ValueType>
      >::type::value;

    typedef typename 
      IfElse<
        (Comparison::template Equal<ValueType,head_type>::value),
        ValueType,
        typename tail_set::template Contains<ValueType>::type
      >::type type;
  };


  // When considering a Container as a sequence,
  // Container::ValueAtPosition<i>::type gives the typename with
  // position i (counting from 0) within that sequence, and
  // Container::ValueAtPosition<i>::value() returns its index value.
  template <unsigned int index>
  struct ValueAtPosition
  {
    typedef typename
      IfElse<
        index,
        typename tail_set::template ValueAtPosition<(index-1)>,
        HeadType
      >::type type;

    static const typename type::value_type value = type::value;
  };


  // Container::Sort::type returns a type corresponding to a
  // sorted Container which contains all the contents of the old Container.
  template <typename NewComparison>
  struct Sort
  {
    typedef typename
      tail_set::template Sort<NewComparison>::type::template Insert<head_type,NewComparison>::type type;
  };

  typedef typename Sort<Comparison>::type Sorted;

  // Container::Prepend<ValueType> returns a type corresponding to a
  // new *potentially unsorted* Container which contains all the contents of
  // the old Container as well as a ValueType at the beginning.
  template <typename ValueType, typename NewComparison=Comparison>
  struct Prepend
  {
    typedef Container<ValueType, Container<head_type, tail_set, NewComparison>, NewComparison> type;
  };


  // Container::Append<ValueType> returns a type corresponding to a
  // new *potentially unsorted* Container which contains all the contents of
  // the old Container as well as a ValueType at the end.
  template <typename ValueType, typename NewComparison=Comparison>
  struct Append
  {
    typedef Container<head_type,
                      typename tail_set::template Append<ValueType,NewComparison>::type,
                      NewComparison> type;
  };


  // Container::Insert<ValueType> returns a type corresponding to a
  // new *sorted* Container which contains all the contents of the old
  // Container as well as a ValueType.
  // If Container::Contains<ValueType>::value is already true, the
  // existing type with that value may still be "upgraded" to the
  // supertype of ValueType and the existing type.
  // In cases where Insert may be called on a NullContainer, the full
  // Insert<ValueType,NewComparison> must be called to specify a
  // comparison template functor
  template <typename ValueType, typename NewComparison=Comparison>
  struct Insert
  {
    typedef
      typename IfElse<
        (is_null_container<ValueType>::value),
        Sorted,
        typename IfElse<
          (Comparison::template LessThan<ValueType,typename Sorted::head_type>::value),
          Container<ValueType,
            Container<typename Sorted::head_type,
              typename Sorted::tail_set,
              Comparison
            >,
            Comparison
          >,
          typename IfElse<
            (Comparison::template LessThan<typename Sorted::head_type,ValueType>::value),
            Container<typename Sorted::head_type,
              typename Sorted::tail_set::template Insert<ValueType,NewComparison>::type,
              Comparison
            >,
            Container<
              typename CombinedType<
                ValueType, typename Sorted::head_type
              >::type,
              typename Sorted::tail_set,
              Comparison
            >
          >::type
        >::type
      >::type type;
  };


  // Set1::Union<Set2>::type returns a *sorted* set type which contains the union
  // of the two input sets.  For any differing ValueTypes with the same values
  // which already exist in both input sets, the corresponding output set value
  // will be "upgraded" to the supertype of the two.
  template <typename Set2>
  struct Union
  {
    typedef typename
      tail_set::Sorted::template Union<
        typename Set2::Sorted::template Insert<
          head_type,
          Comparison
        >::type
      >::type type;
  };


  // Set1::Intersection<Set2>::type returns a set type which contains the
  // intersection of the two input sets.  For any differing ValueTypes with the same
  // values which already exist in both input sets, the corresponding output
  // set value will be "upgraded" to the supertype of the two.
  //
  // The comparison functor of Set1 is used in the intersection set.
  // If both sets are sorted, so is their intersection.
  template <typename Set2>
  struct Intersection
  {
    typedef typename
      IfElse<Set2::template Contains<head_type>::value,
        Container<
          typename SymmetricCompareTypes<
            head_type,typename Set2::template Contains<head_type>::type
          >::supertype,
          typename tail_set::template Intersection<Set2>::type,
          Comparison
        >,
        typename tail_set::template Intersection<Set2>::type
      >::type type;
  };


  // Set1::Difference<Set2>::type returns a containertype which
  // contains the asymmetric difference of the two input containers: 
  // Set1 \ Set2
  template <typename Set2>
  struct Difference
  {
    typedef typename
      IfElse<Set2::template Contains<head_type>::value,
        typename tail_set::template Difference<Set2>::type,
        Container<head_type,
                  typename tail_set::template Difference<Set2>::type,
                  Comparison
        >
      >::type type;
  };


  // Set1::First<Set2>::type returns Set1.
  // This might only be useful for stupid macro tricks.
  template <typename Set2>
  struct First
  {
    typedef Container<HeadType,TailContainer,Comparison> type;
  };


  // Container::RuntimeForEach()(f) applies the standard functor
  // object f to every value in the set.
  struct RuntimeForEach
  {
    template <typename Functor>
    void operator()(const Functor &f) {
      // f might want a reference, but head_type::value is a static const
      // that has been declared in a template and may have never been
      // defined in namespace scope.  So we make a non-static variable
      // here to pass in.
      const typename head_type::value_type v = head_type::value;
      f(v);
      typename tail_set::RuntimeForEach()(f);
    }
  };


  // Container::ForEach()(f) applies the template functor object f to every
  // value type in the set.
  struct ForEach
  {
    template <typename Functor>
    void operator()(const Functor &f) {
      f.template operator()<HeadType>();
      typename tail_set::ForEach()(f);
    }
  };
};


// NullContainer is the sequence terminator object at the end of each
// real set.  It also represents an empty set.  It contains
// specializations for many of the above "member classes" to make
// their recursive magic work.
template <typename HeadType>
struct NullContainer
{
  typedef HeadType head_type;

  // If we need to return a reference to data from this set, then
  // that's a serious enough error that:
  static int we_are_already_hosed;

  template <typename DataType2>
  struct rebind {
    typedef 
      NullContainer<typename HeadType::
        template rebind<DataType2>::other> other;
  };
  
  static const std::size_t size = 0;

  template <typename ValueType>
  struct IndexOf {
    static const int index = -65536;
  };

  template <typename NewComparison>
  struct Sort
  {
    typedef NullContainer type;
  };

  typedef NullContainer Sorted;

  template <typename ValueType, typename NewComparison>
  struct Prepend
  {
    typedef Container<ValueType, NullContainer, NewComparison> type;
  };

  template <typename ValueType, typename NewComparison>
  struct Append
  {
    typedef Container<ValueType, NullContainer, NewComparison> type;
  };

  template <typename ValueType, typename NewComparison>
  struct Insert
  {
    typedef
      typename IfElse<
        (is_null_container<ValueType>::value),
        NullContainer,
        Container<ValueType, NullContainer, NewComparison> 
      >::type type;
  };

  template <typename T>
  inline static unsigned int runtime_index_of(T&) {
    throw std::domain_error("RuntimeIndexOf argument not found");
    return static_cast<unsigned int>(-1);
  }

  template <typename T>
  int& runtime_data(const T&) const {
    throw std::domain_error("Container::runtime_data argument not found");
    return we_are_already_hosed;
  }

  template <typename T>
  static int runtime_value_of(unsigned int /*i*/) {
    throw std::domain_error("Container::runtime_value_of argument not found");
    return static_cast<unsigned int>(-1);
  }


  struct HeadOf {
    template <typename T>
    static const int& value(const T&) {
      throw std::domain_error("NullContainer::HeadOf makes no sense");
      return we_are_already_hosed;
    }

    template <typename T>
    static int& value(T&) {
      throw std::domain_error("NullContainer::HeadOf makes no sense");
      return we_are_already_hosed;
    }
  };


  template <typename ValueType>
  struct ElementOf {
    typedef NullType type;
  };


  template <typename ValueType>
  struct Contains
  {
    static const bool value = false;

    typedef NullType type;
  };

  template <typename Set2>
  struct Union
  {
    typedef Set2 type;
  };

  template <typename Set2>
  struct Intersection
  {
    typedef NullContainer type;
  };

  template <typename Set2>
  struct Difference
  {
    typedef NullContainer type;
  };

  template <typename Set2>
  struct First
  {
    typedef NullContainer type;
  };

  struct RuntimeForEach
  {
    template <typename Functor>
    void operator()(const Functor &) {
    }
  };

  struct ForEach
  {
    template <typename Functor>
    void operator()(const Functor &) {
    }
  };
};

#if  __cplusplus >= 201103L
template <unsigned int... Args>
struct UIntList {
  static constexpr 
  std::array<unsigned int, sizeof...(Args)>
  value() 
  { return {{Args...}}; }
};

template <long unsigned int... Args>
struct ULongList {
  static constexpr
  std::array<long unsigned int, sizeof...(Args)>
  value()
  { return {{Args...}}; }
};

template <typename T, T i, class List>
struct ListPrepend;

template <typename T,
          T i, 
          template <T... Args> class ListT,
          T... Args>
struct ListPrepend<T, i, ListT<Args...> >
{
  typedef ListT<i, Args...> type;
};

template <typename Set, typename IndexType=typename Set::head_type::value_type>
struct SetAsList
{
  typedef typename
    ListPrepend<typename Set::head_type::value_type,
                Set::head_type::value,
                typename SetAsList<typename Set::tail_set, IndexType>::type
               >::type type;
};

template <typename NullHeadType>
struct SetAsList<NullContainer<NullHeadType>, unsigned int>
{
  typedef UIntList<> type;
};

template <typename NullHeadType>
struct SetAsList<NullContainer<NullHeadType>, long unsigned int>
{
  typedef ULongList<> type;
};


template <typename Set>
struct SetAsArray
{
  static constexpr
  std::array<typename Set::head_type::value_type, Set::size>
  value()
  { return SetAsList<Set>::type::value(); }
};

template <typename NullHeadType>
struct SetAsArray<NullContainer<NullHeadType> >
{
  static constexpr
  std::array<bool, 0>
  value()
  { return std::array<bool, 0>(); }
};



template <typename Set1, typename Set2, typename IndexType=typename Set2::head_type::value_type>
struct PermutationList
{
  typedef typename
    ListPrepend<IndexType,
                Set2::template IndexOf<typename Set1::head_type>::index,
                typename PermutationList<
                  typename Set1::tail_set,
                  Set2, IndexType>::type
               >::type type;
};


template <typename NullHeadType, typename Set2>
struct PermutationList<NullContainer<NullHeadType>, Set2, unsigned int>
{
  typedef UIntList<> type;
};


template <typename NullHeadType, typename Set2>
struct PermutationList<NullContainer<NullHeadType>, Set2, long unsigned int>
{
  typedef ULongList<> type;
};


template <typename Set1, typename Set2>
struct PermutationArray
{
  static constexpr
  std::array<typename Set2::head_type::value_type, Set1::size>
  value()
  { return PermutationList<Set1, Set2>::type::value(); }
};


#endif


template <typename Set>
struct SetOfSetsUnion
{
  typedef typename Set::head_type::data_type::template Union<
    typename SetOfSetsUnion<typename Set::tail_set>::type
  >::type type;
};


template <typename NullHeadType>
struct SetOfSetsUnion<NullContainer<NullHeadType> >
{
  typedef NullContainer<NullHeadType> type;
};


template <typename Set>
struct SetOfSetsIntersection
{
  typedef typename Set::head_type::data_type::template Intersection<
    typename SetOfSetsIntersection<typename Set::tail_set>::type
  >::type type;
};


template <typename T, typename NullHeadType, typename Comparison>
struct SetOfSetsIntersection<Container<T, NullContainer<NullHeadType>, Comparison> >
{
  typedef typename T::data_type type;
};


template <typename NullHeadType>
struct SetOfSetsIntersection<NullContainer<NullHeadType> >
{
  typedef NullContainer<NullHeadType> type;
};



// We make some things a separate class because they can't be
// instantiated for all Containers, only those whose contents define a
// data_type.

// The ContainerSupertype is the supertype of all data_types contained
// by a container.
template <typename Set>
struct ContainerSupertype
{
  typedef typename
    SymmetricCompareTypes<
      typename Set::head_type::data_type,
      typename ContainerSupertype<typename Set::tail_set>::type
    >::supertype type;
};


template <typename NullHeadType>
struct ContainerSupertype<NullContainer<NullHeadType> >
{
  typedef NullType type;
};


// SetConstructor is some syntactic sugar to make it easier to
// manually construct short (16 element or less) set types.
// I don't want to require C++11, so no variable-length template
// argument lists here
template <typename T0=NullType,
          typename T1=NullType,
          typename T2=NullType,
          typename T3=NullType,
          typename T4=NullType,
          typename T5=NullType,
          typename T6=NullType,
          typename T7=NullType,
          typename T8=NullType,
          typename T9=NullType,
          typename T10=NullType,
          typename T11=NullType,
          typename T12=NullType,
          typename T13=NullType,
          typename T14=NullType,
          typename T15=NullType >
struct SetConstructor
{
  typedef typename
    SetConstructor<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15>::
      type::template Insert<T0,ValueLessThan>::type
      type;
};

template<>
struct SetConstructor<>
{
  typedef NullContainer<NullType> type;
};


// ULongSetConstructor is some syntactic sugar to make it easier to
// manually construct short (16 element or less) set-of-uint types.
template <int I0=-1,
          int I1=-1, 
          int I2=-1, 
          int I3=-1, 
          int I4=-1, 
          int I5=-1, 
          int I6=-1, 
          int I7=-1,
          int I8=-1,
          int I9=-1,
          int I10=-1,
          int I11=-1,
          int I12=-1,
          int I13=-1,
          int I14=-1,
          int I15=-1>
struct ULongSetConstructor
{
  typedef typename
    ULongSetConstructor<I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12,I13,I14,I15>::type::
      template Insert<UnsignedLongType<I0>,ValueLessThan>::type
      type;
};

template<>
struct ULongSetConstructor<>
{
  typedef NullContainer<UnsignedLongType<0> > type;
};



// UIntSetConstructor is some syntactic sugar to make it easier to
// manually construct short (16 element or less) set-of-uint types.
template <int I0=-1,
          int I1=-1, 
          int I2=-1, 
          int I3=-1, 
          int I4=-1, 
          int I5=-1, 
          int I6=-1, 
          int I7=-1,
          int I8=-1,
          int I9=-1,
          int I10=-1,
          int I11=-1,
          int I12=-1,
          int I13=-1,
          int I14=-1,
          int I15=-1>
struct UIntSetConstructor
{
  typedef typename
    UIntSetConstructor<I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12,I13,I14,I15>::type::
      template Insert<UnsignedIntType<I0>,ValueLessThan>::type
      type;
};

template<>
struct UIntSetConstructor<>
{
  typedef NullContainer<UnsignedIntType<0> > type;
};


// UIntVectorConstructor is some syntactic sugar to make it easier to
// manually construct short (16 element or less) vector-of-uint types.
template <int I0=-1,
          int I1=-1, 
          int I2=-1, 
          int I3=-1, 
          int I4=-1, 
          int I5=-1, 
          int I6=-1, 
          int I7=-1,
          int I8=-1,
          int I9=-1,
          int I10=-1,
          int I11=-1,
          int I12=-1,
          int I13=-1,
          int I14=-1,
          int I15=-1>
struct UIntVectorConstructor
{
  typedef typename
    UIntVectorConstructor<I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12,I13,I14,I15>::type::
      template Insert<UnsignedIntType<I0>,TypeLessThan>::type
      type;
};

template<>
struct UIntVectorConstructor<>
{
  typedef NullContainer<UnsignedIntType<0> > type;
};


// UIntStructConstructor is some syntactic sugar to make it easier to
// manually construct short (16 element or less) struct-indexed-by-uint types.
template <int I0=-1,
          typename T0=NullType,
          int I1=-1, 
          typename T1=NullType,
          int I2=-1, 
          typename T2=NullType,
          int I3=-1, 
          typename T3=NullType,
          int I4=-1, 
          typename T4=NullType,
          int I5=-1, 
          typename T5=NullType,
          int I6=-1, 
          typename T6=NullType,
          int I7=-1,
          typename T7=NullType,
          int I8=-1,
          typename T8=NullType,
          int I9=-1,
          typename T9=NullType,
          int I10=-1,
          typename T10=NullType,
          int I11=-1,
          typename T11=NullType,
          int I12=-1,
          typename T12=NullType,
          int I13=-1,
          typename T13=NullType,
          int I14=-1,
          typename T14=NullType,
          int I15=-1,
          typename T15=NullType>
struct UIntStructConstructor
{
  typedef typename
    UIntStructConstructor<I1,T1,I2,T2,I3,T3,I4,T4,I5,T5,I6,T6,I7,T7,I8,T8,I9,T9,
                          I10,T10,I11,T11,I12,T12,I13,T13,I14,T14,I15,T15>::type::
      template Insert<UnsignedIntType<I0,T0>,ValueLessThan>::type
      type;
};

template<>
struct UIntStructConstructor<>
{
  typedef NullContainer<UnsignedIntType<0> > type;
};


template <typename T0=NullType,
          typename T1=NullType,
          typename T2=NullType,
          typename T3=NullType,
          typename T4=NullType,
          typename T5=NullType,
          typename T6=NullType,
          typename T7=NullType,
          typename T8=NullType,
          typename T9=NullType,
          typename T10=NullType,
          typename T11=NullType,
          typename T12=NullType,
          typename T13=NullType,
          typename T14=NullType,
          typename T15=NullType >
struct VectorConstructor
{
  typedef typename
    VectorConstructor<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15>::
      type::template Prepend<T0,TypeLessThan>::type
      type;
};

template<>
struct VectorConstructor<>
{
  typedef NullContainer<NullType> type;
};


// CompareTypes specializations are useful for sets-with-data
template<typename Head1, typename Tail1, typename Comparison1,
         typename Head2, typename Tail2, typename Comparison2, bool reverseorder>
struct CompareTypes<MetaPhysicL::Container<Head1,Tail1,Comparison1>,
                    MetaPhysicL::Container<Head2,Tail2,Comparison2>,
                    reverseorder>
{
  typedef typename
    MetaPhysicL::Container<Head1,Tail1,Comparison1>::template
      Union<MetaPhysicL::Container<Head2,Tail2,Comparison2> >::type
      supertype;
};

template<typename T, typename HeadType, typename TailSet, typename Comparison, bool reverseorder>
struct CompareTypes<MetaPhysicL::Container<HeadType,TailSet,Comparison>,T,reverseorder,
                    typename boostcopy::enable_if<BuiltinTraits<T> >::type>
{
  typedef typename
    MetaPhysicL::Container<HeadType,TailSet,Comparison>::template UpgradeType<T>::type
      supertype;
};


template<int i, typename T, bool reverseorder>
struct CompareTypes<MetaPhysicL::IntType<i,T>,
                    MetaPhysicL::NullType,
                    reverseorder>
{
  typedef MetaPhysicL::IntType<i,T> supertype;
};


template<unsigned int i, typename T, bool reverseorder>
struct CompareTypes<MetaPhysicL::UnsignedIntType<i,T>,
                    MetaPhysicL::NullType,
                    reverseorder>
{
  typedef MetaPhysicL::UnsignedIntType<i,T> supertype;
};


template<long unsigned int i, typename T, bool reverseorder>
struct CompareTypes<MetaPhysicL::UnsignedLongType<i,T>,
                    MetaPhysicL::NullType,
                    reverseorder>
{
  typedef MetaPhysicL::UnsignedLongType<i,T> supertype;
};


template<typename H, typename T, typename C, bool reverseorder>
struct CompareTypes<MetaPhysicL::Container<H,T,C>,
                    MetaPhysicL::NullType,
                    reverseorder>
{
  typedef MetaPhysicL::Container<H,T,C> supertype;
};


template<typename T, bool reverseorder>
struct CompareTypes<MetaPhysicL::NullType,T,reverseorder,
                    typename boostcopy::enable_if<BuiltinTraits<T> >::type>
{
  typedef T supertype;
};


template<typename NullHeadType, typename T, bool reverseorder>
struct CompareTypes<MetaPhysicL::NullContainer<NullHeadType>,T,reverseorder,
                    typename boostcopy::enable_if<BuiltinTraits<T> >::type>
{
  typedef MetaPhysicL::NullContainer<NullHeadType> supertype;
};


CompareTypes_default_Type(Plus,
                          typename T MacroComma typename HeadType MacroComma typename TailSet MacroComma typename Comparison MacroComma,
                          MetaPhysicL::Container<HeadType MacroComma TailSet MacroComma Comparison>, T,
                          typename boostcopy::enable_if<BuiltinTraits<T> >::type);
CompareTypes_default_Type(Minus,
                          typename T MacroComma typename HeadType MacroComma typename TailSet MacroComma typename Comparison MacroComma,
                          MetaPhysicL::Container<HeadType MacroComma TailSet MacroComma Comparison>, T,
                          typename boostcopy::enable_if<BuiltinTraits<T> >::type);
CompareTypes_default_Type(Multiplies,
                          typename T MacroComma typename HeadType MacroComma typename TailSet MacroComma typename Comparison MacroComma,
                          MetaPhysicL::Container<HeadType MacroComma TailSet MacroComma Comparison>, T,
                          typename boostcopy::enable_if<BuiltinTraits<T> >::type);
CompareTypes_default_Type(Divides,
                          typename T MacroComma typename HeadType MacroComma typename TailSet MacroComma typename Comparison MacroComma,
                          MetaPhysicL::Container<HeadType MacroComma TailSet MacroComma Comparison>, T,
                          typename boostcopy::enable_if<BuiltinTraits<T> >::type);
CompareTypes_default_Type(And,
                          typename T MacroComma typename HeadType MacroComma typename TailSet MacroComma typename Comparison MacroComma,
                          MetaPhysicL::Container<HeadType MacroComma TailSet MacroComma Comparison>, T,
                          typename boostcopy::enable_if<BuiltinTraits<T> >::type);
CompareTypes_default_Type(Or,
                          typename T MacroComma typename HeadType MacroComma typename TailSet MacroComma typename Comparison MacroComma,
                          MetaPhysicL::Container<HeadType MacroComma TailSet MacroComma Comparison>, T,
                          typename boostcopy::enable_if<BuiltinTraits<T> >::type);
CompareTypes_default_Type(Plus,
                          typename T MacroComma, MetaPhysicL::NullType, T,
                          typename boostcopy::enable_if<BuiltinTraits<T> >::type);
CompareTypes_default_Type(Minus,
                          typename T MacroComma, MetaPhysicL::NullType, T,
                          typename boostcopy::enable_if<BuiltinTraits<T> >::type);
CompareTypes_default_Type(Multiplies,
                          typename T MacroComma, MetaPhysicL::NullType, T,
                          typename boostcopy::enable_if<BuiltinTraits<T> >::type);
CompareTypes_default_Type(Divides,
                          typename T MacroComma, MetaPhysicL::NullType, T,
                          typename boostcopy::enable_if<BuiltinTraits<T> >::type);
CompareTypes_default_Type(And,
                          typename T MacroComma, MetaPhysicL::NullType, T,
                          typename boostcopy::enable_if<BuiltinTraits<T> >::type);
CompareTypes_default_Type(Or,
                          typename T MacroComma, MetaPhysicL::NullType, T,
                          typename boostcopy::enable_if<BuiltinTraits<T> >::type);
CompareTypes_default_Type(Plus,
                          typename NullHeadType MacroComma typename T MacroComma,
                          MetaPhysicL::NullContainer<NullHeadType>, T,
                          typename boostcopy::enable_if<BuiltinTraits<T> >::type);
CompareTypes_default_Type(Minus,
                          typename NullHeadType MacroComma typename T MacroComma,
                          MetaPhysicL::NullContainer<NullHeadType>, T,
                          typename boostcopy::enable_if<BuiltinTraits<T> >::type);
CompareTypes_default_Type(Multiplies,
                          typename NullHeadType MacroComma typename T MacroComma,
                          MetaPhysicL::NullContainer<NullHeadType>, T,
                          typename boostcopy::enable_if<BuiltinTraits<T> >::type);
CompareTypes_default_Type(Divides,
                          typename NullHeadType MacroComma typename T MacroComma,
                          MetaPhysicL::NullContainer<NullHeadType>, T,
                          typename boostcopy::enable_if<BuiltinTraits<T> >::type);
CompareTypes_default_Type(And,
                          typename NullHeadType MacroComma typename T MacroComma,
                          MetaPhysicL::NullContainer<NullHeadType>, T,
                          typename boostcopy::enable_if<BuiltinTraits<T> >::type);
CompareTypes_default_Type(Or,
                          typename NullHeadType MacroComma typename T MacroComma,
                          MetaPhysicL::NullContainer<NullHeadType>, T,
                          typename boostcopy::enable_if<BuiltinTraits<T> >::type);

} // namespace MetaPhysicL

#endif // METAPHYSICL_CT_SET_H
