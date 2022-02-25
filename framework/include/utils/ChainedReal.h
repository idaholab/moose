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
#include "metaphysicl/metaphysicl_version.h"

namespace MetaPhysicL
{
#if METAPHYSICL_MAJOR_VERSION < 1
template <typename, typename>
class DualNumber;
#else
#include "metaphysicl/dualnumber_forward.h"
#endif
}

using MetaPhysicL::DualNumber;

typedef DualNumber<Real, Real> ChainedReal;

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
  template <bool reverseorder, typename Enable>                                                    \
  struct CompareTypes<a, b, reverseorder, Enable>                                                  \
  {                                                                                                \
    typedef super supertype;                                                                       \
  };                                                                                               \
  CompareTypes_default_Types(, a, b, void)

CompareTypes_super(Real, ChainedReal, ChainedReal);

#undef CompareTypes_default_Types
#undef CompareTypes_super
}
