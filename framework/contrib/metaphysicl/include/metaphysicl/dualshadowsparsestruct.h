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

#ifndef METAPHYSICL_DUALSHADOWSPARSESTRUCT_H
#define METAPHYSICL_DUALSHADOWSPARSESTRUCT_H


// Order of declarations is important here?
#include "metaphysicl/dualshadow.h"
#include "metaphysicl/dualsparsenumberstruct.h"

// ShadowNumber is subordinate to SparseNumberStruct,
// NullType is subordinate to ShadowNumber,
// ShadowNumber is subordinate to IndexSet,

// FIXME: need to be more specialized on the Container::UpgradeType

#define DualShadowSparseStruct_comparisons(templatename) \
template<typename T, typename S, typename IndexSet, bool reverseorder> \
struct templatename<SparseNumberStruct<IndexSet>, ShadowNumber<T, S>, reverseorder> { \
  typedef SparseNumberStruct<typename Symmetric##templatename<IndexSet, ShadowNumber<T, S>, reverseorder>::supertype> supertype; \
}; \
 \
template<typename T, typename S, bool reverseorder> \
struct templatename<ShadowNumber<T, S>, MetaPhysicL::NullType, reverseorder> { \
  typedef ShadowNumber<T, S> supertype; \
}; \
 \
template<typename T, typename S, typename HeadType, typename TailSet, typename Comparison, bool reverseorder> \
struct templatename<MetaPhysicL::Container<HeadType,TailSet,Comparison>, ShadowNumber<T, S>, reverseorder> { \
  typedef typename \
    MetaPhysicL::Container<HeadType,TailSet,Comparison>::template UpgradeType<ShadowNumber<T, S> >::type \
      supertype; \
}; \
 \
template<typename NullHeadType, typename T, typename S, bool reverseorder> \
struct templatename<MetaPhysicL::NullContainer<NullHeadType>, ShadowNumber<T, S>, reverseorder> { \
  typedef MetaPhysicL::NullContainer<NullHeadType> supertype; \
}

namespace MetaPhysicL {

DualShadowSparseStruct_comparisons(CompareTypes);
DualShadowSparseStruct_comparisons(PlusType);
DualShadowSparseStruct_comparisons(MinusType);
DualShadowSparseStruct_comparisons(MultipliesType);
DualShadowSparseStruct_comparisons(DividesType);
DualShadowSparseStruct_comparisons(AndType);
DualShadowSparseStruct_comparisons(OrType);

} // namespace MetaPhysicL

#endif // METAPHYSICL_DUALSHADOWSPARSESTRUCT_H
