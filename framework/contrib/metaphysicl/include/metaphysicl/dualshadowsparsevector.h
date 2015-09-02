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

#ifndef METAPHYSICL_DUALSHADOWSPARSEVECTOR_H
#define METAPHYSICL_DUALSHADOWSPARSEVECTOR_H


// Order of declarations is important here?
#include "metaphysicl/dualshadow.h"
#include "metaphysicl/dualsparsenumbervector.h"

// ShadowNumber is subordinate to SparseNumberVector
// T-vs-ShadowNumber CompareTypes specializations...

#define DualShadowSparseVector_comparisons(templatename) \
template<typename T, typename T2, typename S, typename IndexSet, bool reverseorder> \
struct templatename<SparseNumberVector<T2, IndexSet>, ShadowNumber<T, S>, reverseorder> { \
  typedef SparseNumberVector<typename Symmetric##templatename<T2, ShadowNumber<T, S>, reverseorder>::supertype, IndexSet> supertype; \
}

namespace MetaPhysicL {

DualShadowSparseVector_comparisons(CompareTypes);
DualShadowSparseVector_comparisons(PlusType);
DualShadowSparseVector_comparisons(MinusType);
DualShadowSparseVector_comparisons(MultipliesType);
DualShadowSparseVector_comparisons(DividesType);
DualShadowSparseVector_comparisons(AndType);
DualShadowSparseVector_comparisons(OrType);

} // namespace MetaPhysicL

#endif // METAPHYSICL_DUALSHADOWSPARSEVECTOR_H
