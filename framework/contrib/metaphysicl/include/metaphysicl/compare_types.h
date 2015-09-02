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

#ifndef METAPHYSICL_COMPARE_TYPES_H
#define METAPHYSICL_COMPARE_TYPES_H

// System includes
#include <complex>

namespace MetaPhysicL {

// Compile-time assertions:
// Use of ctassert<E>, where E is a constant expression,
// will cause a compile-time error unless E evaulates to
// a nonzero integral value.

template <bool t>
struct ctassert {
  enum { N = 1 - 2 * int(!t) };
    // 1 if t is true, -1 if t is false.
  static char A[N];

  static void apply() {}
};

template <typename T1=void,
          typename T2=void,
          typename T3=void,
          typename T4=void,
          typename T5=void,
          typename T6=void,
          typename T7=void,
          typename T8=void,
          typename T9=void>
struct ctprint
{
  typedef typename T9::compiler_should_print_types force_an_error;
  force_an_error f;
  static void apply() {}
};

template <>
struct ctprint<>
{
  static void apply() {}
};

template <bool t>
char ctassert<t>::A[N];

// Copy of boost's enable_if_c

namespace boostcopy {
  template <bool B, class T = void>
    struct enable_if_c {
      typedef T type;
    };

  template <class T>
    struct enable_if_c<false, T> {};

  template <class Cond, class T = void>
    struct enable_if : public enable_if_c<Cond::value, T> {};

  template <class B, class T>
    struct enable_if_class {
      typedef T type;
    };

  template <class T>
    struct remove_const {
      typedef T type;
    };

  template <class T>
    struct remove_const<const T> {
      typedef T type;
    };

  template <class T>
    struct remove_reference {
      typedef T type;
    };

  template <class T>
    struct remove_reference<T&> {
      typedef T type;
    };
}


//
// Should we store a value or a reference, in ``pass-through''
// expression cases where we have the option?
// 
// Copying underlying storage unnecessarily is wasteful in the best
// case, and leads to segfaults if we copy into a temporary that gets
// just referenced in outer expressions.
//
// Creating references leads to segfaults if we create references to
// expression template objects in temporaries.
//
// To determine what's got underlying storage and what's a expression
// template, we'll try just enumerating ``underlying storage'' types
// to reference, and default everything else to copy
//

template <typename T, typename Enable=void>
struct copy_or_reference {
  typedef typename boostcopy::remove_reference<T>::type type;

  static const bool copy = true;
};



// We can pass commas for template arguments through one level of
// macro expansion by "escaping" them this way:
#define MacroComma ,


// List of scalar and builtin classes, useful for disambiguation
template <typename T, typename Enable=void>
struct ScalarTraits {
      static const bool value = false;
};

template <typename T, typename Enable=void>
struct BuiltinTraits {
      static const bool value = false;
};

#define ScalarBuiltin_true(type) \
template<> \
struct ScalarTraits<type> { static const bool value = true; }; \
template<> \
struct BuiltinTraits<type> { static const bool value = true; }

ScalarBuiltin_true(bool);
ScalarBuiltin_true(char);
ScalarBuiltin_true(short);
ScalarBuiltin_true(int);
ScalarBuiltin_true(long);
ScalarBuiltin_true(unsigned char);
ScalarBuiltin_true(unsigned short);
ScalarBuiltin_true(unsigned int);
ScalarBuiltin_true(unsigned long);
ScalarBuiltin_true(float);
ScalarBuiltin_true(double);
ScalarBuiltin_true(long double);

template<typename T>
struct ScalarTraits<std::complex<T> > { static const bool value = ScalarTraits<T>::value; };

template<typename T>
struct BuiltinTraits<std::complex<T> > { static const bool value = BuiltinTraits<T>::value; };



// Operators using different but compatible types need a return value
// based on whichever type the other can be upconverted into.  For
// instance, the proper return type for
// TypeVector<float>::operator*(double) is TypeVector<double>.  In
// general, an operation using types S and T should return a value
// based on CompareTypes<S,T>::supertype

template<typename S, typename T, bool reverseorder=false, typename Enable=void>
struct CompareTypes {

// All specializations need to define supertype.  But compilers give
// better error messages for forgot-to-specialize-CompareTypes bugs if
// we leave supertype undefined in the unspecialized template
// definition.

// typedef void supertype;

//   typedef S nosupertype;
};


// A tricky SFINAE idiom for testing whether a particular CompareTypes
// combination has been properly specialized:

template<typename T>
struct DefinesSupertype
{
private:
    typedef char                      yes;
    typedef struct { char array[2]; } no;

    template<typename C> static yes test(typename C::supertype*);
    template<typename C> static no  test(...);
public:
    static const bool value = (sizeof(test<T>(0)) == sizeof(yes));
};


// PlusType, MinusType, MultipliesType, and DividesType are usually just the
// same as CompareTypes, but user types may want to specialize further for
// efficiency.
// FIXME: AndType and OrType should probably be boolean types where possible.
template<typename S, typename T, bool reverseorder=false, typename Enable=void>
struct PlusType {
};

template<typename S, typename T, bool reverseorder=false, typename Enable=void>
struct MinusType {
};

template<typename S, typename T, bool reverseorder=false, typename Enable=void>
struct MultipliesType {
};

template<typename S, typename T, bool reverseorder=false, typename Enable=void>
struct DividesType {
};

template<typename S, typename T, bool reverseorder=false, typename Enable=void>
struct AndType {
};

template<typename S, typename T, bool reverseorder=false, typename Enable=void>
struct OrType {
};

// DotType, OuterProductType, and SumType are only defined for vector
// and tensor objects.
template<typename S, typename T, bool reverseorder=false, typename Enable=void>
struct DotType {
};

template<typename S, typename T, bool reverseorder=false, typename Enable=void>
struct OuterProductType {
};

template<typename S>
struct SumType {
};


// typenames may need a MacroComma...
#define CompareTypes_default_Type(functor, typenames, typename1, typename2, enabletype) \
template<typenames bool reverseorder> \
struct functor##Type<typename1, typename2, reverseorder, enabletype> \
{ \
   typedef typename CompareTypes<typename1,typename2,reverseorder>::supertype supertype; \
}

#define CompareTypes_default_Types(typenames,typename1,typename2, enabletype) \
CompareTypes_default_Type(Plus,typenames,typename1,typename2, enabletype); \
CompareTypes_default_Type(Minus,typenames,typename1,typename2, enabletype); \
CompareTypes_default_Type(Multiplies,typenames,typename1,typename2, enabletype); \
CompareTypes_default_Type(Divides,typenames,typename1,typename2, enabletype); \
CompareTypes_default_Type(And,typenames,typename1,typename2, enabletype); \
CompareTypes_default_Type(Or,typenames,typename1,typename2, enabletype) \


template<bool reverseorder, typename Enable>
struct CompareTypes<void, void, reverseorder, Enable> {
  typedef void supertype;
};

template<typename T, bool reverseorder, typename Enable>
struct CompareTypes<T, void, reverseorder, Enable> {
  typedef T supertype;
};

template<typename T, bool reverseorder, typename Enable>
struct CompareTypes<T, T, reverseorder, Enable> {
  typedef T supertype;
};

template<typename T, bool reverseorder, typename Enable>
struct CompareTypes<T, std::complex<T>, reverseorder, Enable> {
  typedef std::complex<T> supertype;
};

template<typename T, bool reverseorder, typename Enable>
struct CompareTypes<std::complex<T>, T, reverseorder, Enable> {
  typedef std::complex<T> supertype;
};

// There's got to be some magic template way to do these better - but the best
// thing on the net requires a bunch of Alexandrescu's code and doesn't work
// with older compilers

#define CompareTypes_super(a,b,super) \
	template<bool reverseorder, typename Enable> \
	struct CompareTypes<a, b, reverseorder, Enable> { \
	  typedef super supertype; \
	}; \
        CompareTypes_default_Types(,a,b,void)

#define CompareTypes_all(mysub,mysuper) \
        CompareTypes_super(mysub, mysuper, mysuper); \
        CompareTypes_super(mysuper, mysub, mysuper); \
        CompareTypes_super(std::complex<mysub>, mysuper, std::complex<mysuper>); \
        CompareTypes_super(mysuper, std::complex<mysub>, std::complex<mysuper>); \
        CompareTypes_super(mysub, std::complex<mysuper>, std::complex<mysuper>); \
        CompareTypes_super(std::complex<mysuper>, mysub, std::complex<mysuper>); \
        CompareTypes_super(std::complex<mysub>, std::complex<mysuper>, std::complex<mysuper>); \
        CompareTypes_super(std::complex<mysuper>, std::complex<mysub>, std::complex<mysuper>)

#define CompareTypes_single(mytype) \
        CompareTypes_super(mytype, mytype, mytype)

CompareTypes_single(unsigned char);
CompareTypes_single(unsigned short);
CompareTypes_single(unsigned int);
CompareTypes_single(bool);
CompareTypes_single(char);
CompareTypes_single(short);
CompareTypes_single(int);
CompareTypes_single(float);
CompareTypes_single(double);
CompareTypes_single(long double);

CompareTypes_all(unsigned char, short);
CompareTypes_all(unsigned char, int);
CompareTypes_all(unsigned char, float);
CompareTypes_all(unsigned char, double);
CompareTypes_all(unsigned char, long double);
CompareTypes_all(unsigned short, int);
CompareTypes_all(unsigned short, float);
CompareTypes_all(unsigned short, double);
CompareTypes_all(unsigned short, long double);
CompareTypes_all(unsigned int, float);
CompareTypes_all(unsigned int, double);
CompareTypes_all(unsigned int, long double);
CompareTypes_all(bool, char);
CompareTypes_all(bool, unsigned char);
CompareTypes_all(bool, short);
CompareTypes_all(bool, int);
CompareTypes_all(bool, float);
CompareTypes_all(bool, double);
CompareTypes_all(bool, long double);
CompareTypes_all(char, short);
CompareTypes_all(char, int);
CompareTypes_all(char, float);
CompareTypes_all(char, double);
CompareTypes_all(char, long double);
CompareTypes_all(short, int);
CompareTypes_all(short, float);
CompareTypes_all(short, double);
CompareTypes_all(short, long double);
CompareTypes_all(int, float);
CompareTypes_all(int, double);
CompareTypes_all(int, long double);
CompareTypes_all(float, double);
CompareTypes_all(float, long double);
CompareTypes_all(double, long double);

// gcc can't tell which of the following is the most specialized?  Weak.
/*
template<typename S, typename T>
struct CompareTypes<std::complex<S>, std::complex<T> > {
  typedef std::complex<typename CompareTypes<S, T>::supertype> supertype;
};

template<typename S, typename T>
struct CompareTypes<std::complex<S>, T> {
  typedef std::complex<typename CompareTypes<S, T>::supertype> supertype;
};

template<typename S, typename T>
struct CompareTypes<S, std::complex<T> > {
  typedef std::complex<typename CompareTypes<S, T>::supertype> supertype;
};
*/

/*
// These are ambiguous:

#define CompareTypes_stripped(rawT1, rawT2) \
template<typename T1, typename T2, bool reverseorder> \
struct CompareTypes<rawT1, rawT2, reverseorder, \
	            typename boostcopy::enable_if_c<DefinesSupertype<CompareTypes<T1,T2,reverseorder> >::value>::type> \
{ \
  typedef typename CompareTypes<T1,T2,reverseorder>::supertype supertype; \
};
*/

template <typename T1, typename T2, bool reverseorder, typename Enable=void>
struct CompareTypesEnabler {};

template <typename T1, typename T2, bool reverseorder>
struct CompareTypesEnabler<T1,T2,reverseorder,
  typename boostcopy::enable_if_c<
    DefinesSupertype<CompareTypes<T1,T2,reverseorder> >::value
  >::type>
{
  typedef void type;
};

#define CompareType_stripped(rawT1) \
template<typename T1, bool reverseorder> \
struct CompareTypes<rawT1, rawT1, reverseorder, \
typename CompareTypesEnabler< \
typename boostcopy::remove_const<typename boostcopy::remove_reference<rawT1>::type>::type, \
typename boostcopy::remove_const<typename boostcopy::remove_reference<rawT1>::type>::type, \
reverseorder>::type> \
{ \
  typedef typename CompareTypes<T1,T1,reverseorder>::supertype supertype; \
};

#define CompareTypes_stripped(rawT1, rawT2) \
template<typename T1, typename T2, bool reverseorder> \
struct CompareTypes<rawT1, rawT2, reverseorder, \
typename CompareTypesEnabler< \
typename boostcopy::remove_const<typename boostcopy::remove_reference<rawT1>::type>::type, \
typename boostcopy::remove_const<typename boostcopy::remove_reference<rawT2>::type>::type, \
reverseorder>::type> \
{ \
  typedef typename CompareTypes<T1,T2,reverseorder>::supertype supertype; \
};

CompareTypes_stripped(const T1 ,       T2 )
CompareTypes_stripped(      T1 , const T2 )
CompareTypes_stripped(const T1 , const T2 )
CompareTypes_stripped(const T1&,       T2 )
CompareTypes_stripped(      T1 , const T2&)
CompareTypes_stripped(const T1&, const T2&)
CompareTypes_stripped(const T1 , const T2&)
CompareTypes_stripped(const T1&, const T2 )

CompareType_stripped(const T1 )
CompareType_stripped(const T1&)


// We can define CompareTypes template specializations with user types
// asymmetrically, to assist in disambiguation of templated functions
// or classes.  But sometimes we just need the supertype:

// FIXME: this won't work yet for cases where CompareTypes depends on
// reverseorder

#define Symmetric_definition(templatename) \
template<typename S, typename T, bool reverseorder=false, \
	 bool Simple=DefinesSupertype<templatename<S,T> >::value> \
struct Symmetric##templatename { \
  typedef typename templatename<S,T,reverseorder>::supertype supertype; \
}; \
 \
template<typename S, typename T, bool reverseorder> \
struct Symmetric##templatename<S, T, reverseorder, false> \
{ \
  typedef typename templatename<T,S,!reverseorder>::supertype supertype; \
}



Symmetric_definition(CompareTypes);
Symmetric_definition(PlusType);
Symmetric_definition(MinusType);
Symmetric_definition(MultipliesType);
Symmetric_definition(DividesType);
Symmetric_definition(AndType);
Symmetric_definition(OrType);

// Undefine our local macros to avoid polluting the namespace

#undef ScalarBuiltin_true
#undef CompareTypes_default_Types
#undef CompareTypes_super
#undef CompareTypes_all
#undef CompareTypes_single
#undef CompareTypes_stripped
#undef Symmetric_definition

// But keep macros that are used elsewhere

// #undef MacroComma
// #undef CompareTypes_default_Type

// Define an overloadable "ternary" operator.  No short-circuiting
// here I'm afraid.

template <typename B, typename T, typename T2>
inline
typename boostcopy::enable_if_c<
  ScalarTraits<B>::value,
  typename CompareTypes<T,T2>::supertype
>::type
if_else (const B & condition, const T & if_true, const T2 & if_false)
{
  if (condition)
    return if_true;
  return if_false;
}


} // namespace MetaPhysicL

#endif // METAPHYSICL_COMPARE_TYPES_H
