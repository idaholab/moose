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

#ifndef METAPHYSICL_DUALSHADOWDYNAMICSPARSEVECTOR_H
#define METAPHYSICL_DUALSHADOWDYNAMICSPARSEVECTOR_H


// Order of declarations is important here?
#include "metaphysicl/dualshadow.h"
#include "metaphysicl/dualdynamicsparsenumbervector.h"

// ShadowNumber is subordinate to DynamicSparseNumberVector
// T-vs-ShadowNumber CompareTypes specializations...

#define DualShadowDynamicSparseVector_comparisons(templatename) \
template<typename T, typename T2, typename S, typename IndexSet, bool reverseorder> \
struct templatename<DynamicSparseNumberVector<T2, IndexSet>, ShadowNumber<T, S>, reverseorder> { \
  typedef DynamicSparseNumberVector<typename Symmetric##templatename<T2, ShadowNumber<T, S>, reverseorder>::supertype, IndexSet> supertype; \
}

namespace MetaPhysicL {

DualShadowDynamicSparseVector_comparisons(CompareTypes);
DualShadowDynamicSparseVector_comparisons(PlusType);
DualShadowDynamicSparseVector_comparisons(MinusType);
DualShadowDynamicSparseVector_comparisons(MultipliesType);
DualShadowDynamicSparseVector_comparisons(DividesType);
DualShadowDynamicSparseVector_comparisons(AndType);
DualShadowDynamicSparseVector_comparisons(OrType);

} // namespace MetaPhysicL

#endif // METAPHYSICL_DUALSHADOWDYNAMICSPARSEVECTOR_H
