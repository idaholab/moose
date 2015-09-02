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

#ifndef METAPHYSICL_DUALSHADOW_H
#define METAPHYSICL_DUALSHADOW_H

// Order of declarations is important here?
#include "metaphysicl/shadownumber.h"
#include "metaphysicl/dualnumber.h"

// ShadowNumber is subordinate to DualNumber:

#define DualShadow_comparisons(templatename) \
template<typename T, typename D, typename T2, typename S, bool reverseorder> \
struct templatename<DualNumber<T, D>, ShadowNumber<T2, S>, reverseorder> { \
  typedef DualNumber<typename Symmetric##templatename<T, ShadowNumber<T2, S>, reverseorder>::supertype, \
                     typename Symmetric##templatename<D, ShadowNumber<T2, S>, reverseorder>::supertype> supertype; \
}

namespace MetaPhysicL {

DualShadow_comparisons(CompareTypes);
DualShadow_comparisons(PlusType);
DualShadow_comparisons(MinusType);
DualShadow_comparisons(MultipliesType);
DualShadow_comparisons(DividesType);
DualShadow_comparisons(AndType);
DualShadow_comparisons(OrType);

} // namespace MetaPhysicL

#endif // METAPHYSICL_DUALSHADOW_H

