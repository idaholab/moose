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

#ifndef METAPHYSICL_DUALSHADOWVECTOR_H
#define METAPHYSICL_DUALSHADOWVECTOR_H


// Order of declarations is important here?
#include "metaphysicl/dualnumber_decl.h"
#include "metaphysicl/numbervector.h"
#include "metaphysicl/dualshadow.h"
#include "metaphysicl/dualnumbervector.h"

// ShadowNumber is subordinate to NumberVector:

#define DualShadowVector_comparisons(templatename) \
template<std::size_t size, typename T, typename T2, typename S, bool reverseorder> \
struct templatename<NumberVector<size, T2>, ShadowNumber<T, S>, reverseorder> { \
  typedef NumberVector<size, typename Symmetric##templatename<T2, ShadowNumber<T, S>, reverseorder>::supertype> supertype; \
}

namespace MetaPhysicL {

DualShadowVector_comparisons(CompareTypes);
DualShadowVector_comparisons(PlusType);
DualShadowVector_comparisons(MinusType);
DualShadowVector_comparisons(MultipliesType);
DualShadowVector_comparisons(DividesType);
DualShadowVector_comparisons(AndType);
DualShadowVector_comparisons(OrType);

} // namespace MetaPhysicL

#endif // METAPHYSICL_DUALSHADOWVECTOR_H
