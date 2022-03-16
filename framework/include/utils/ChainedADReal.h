//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ChainedReal.h"
#include "ADRealForward.h"

typedef DualNumber<ADReal, ADReal> ChainedADReal;

/**
 * We need to instantiate the following CompareTypes to tell the compiler that ADReal is a subtype
 * of ChainedADReal. Otherwise specialization of operators on DualNumber<T, D> and DualNumber<T2,
 * D2> is ambiguous.
 */
namespace MetaPhysicL
{
#define CompareTypes_default_Types(typenames, typename1, typename2, enabletype)                    \
  CompareTypes_default_Type(Plus, typenames, typename1, typename2, enabletype);                    \
  CompareTypes_default_Type(Minus, typenames, typename1, typename2, enabletype);                   \
  CompareTypes_default_Type(Multiplies, typenames, typename1, typename2, enabletype);              \
  CompareTypes_default_Type(Divides, typenames, typename1, typename2, enabletype);                 \
  CompareTypes_default_Type(And, typenames, typename1, typename2, enabletype);                     \
  CompareTypes_default_Type(Or, typenames, typename1, typename2, enabletype)

#define CompareTypes_super(a, b, super)                                                            \
  template <bool reverse_order, typename Enable>                                                   \
  struct CompareTypes<a, b, reverse_order, Enable>                                                 \
  {                                                                                                \
    typedef super supertype;                                                                       \
  };                                                                                               \
  CompareTypes_default_Types(, a, b, void)

CompareTypes_super(ADReal, ChainedADReal, ChainedADReal);

#undef CompareTypes_default_Types
#undef CompareTypes_super
}
