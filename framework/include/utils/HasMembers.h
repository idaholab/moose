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

#define BuiltInSpecializations(Type, BuiltinType)                                                  \
  template <>                                                                                      \
  class HasMemberType_##Type<BuiltinType>                                                          \
  {                                                                                                \
  public:                                                                                          \
    static constexpr bool value = false;                                                           \
  }

// Idiom taken from https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Member_Detector
#define GENERATE_HAS_MEMBER_TYPE0(Type)                                                            \
  template <class T>                                                                               \
  class HasMemberType_##Type                                                                       \
  {                                                                                                \
  private:                                                                                         \
    using Yes = char[2];                                                                           \
    using No = char[1];                                                                            \
                                                                                                   \
    struct Fallback                                                                                \
    {                                                                                              \
      struct Type                                                                                  \
      {                                                                                            \
      };                                                                                           \
    };                                                                                             \
    struct Derived : T, Fallback                                                                   \
    {                                                                                              \
    };                                                                                             \
                                                                                                   \
    template <class U>                                                                             \
    static No & test(typename U::Type *);                                                          \
    template <typename U>                                                                          \
    static Yes & test(U *);                                                                        \
                                                                                                   \
  public:                                                                                          \
    static constexpr bool value = sizeof(test<Derived>(nullptr)) == sizeof(Yes);                   \
  };                                                                                               \
  BuiltInSpecializations(Type, char);                                                              \
  BuiltInSpecializations(Type, short);                                                             \
  BuiltInSpecializations(Type, int);                                                               \
  BuiltInSpecializations(Type, long);                                                              \
  BuiltInSpecializations(Type, unsigned char);                                                     \
  BuiltInSpecializations(Type, unsigned short);                                                    \
  BuiltInSpecializations(Type, unsigned int);                                                      \
  BuiltInSpecializations(Type, unsigned long);                                                     \
  BuiltInSpecializations(Type, float);                                                             \
  BuiltInSpecializations(Type, double);                                                            \
  BuiltInSpecializations(Type, long double)

#ifdef LIBMESH_DEFAULT_QUADRUPLE_PRECISION
#define GENERATE_HAS_MEMBER_TYPE(Type)                                                             \
  GENERATE_HAS_MEMBER_TYPE0(Type);                                                                 \
  BuiltInSpecializations(Type, Real)
#else
#define GENERATE_HAS_MEMBER_TYPE(Type) GENERATE_HAS_MEMBER_TYPE0(Type)
#endif

GENERATE_HAS_MEMBER_TYPE(OutputShape);
GENERATE_HAS_MEMBER_TYPE(value_type);
