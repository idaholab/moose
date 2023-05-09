//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseConfig.h"

#include "libmesh/libmesh_common.h"
#include "libmesh/compare_types.h"

#include "metaphysicl/metaphysicl_version.h"

namespace MetaPhysicL
{
#if METAPHYSICL_MAJOR_VERSION < 1
template <typename, typename>
class DualNumber;
#else
#include "metaphysicl/dualnumber_forward.h"
#endif
template <typename, typename, typename>
class SemiDynamicSparseNumberArray;
template <std::size_t, typename>
class NumberArray;
template <std::size_t N>
struct NWrapper;
}

using libMesh::Real;
using MetaPhysicL::DualNumber;
using MetaPhysicL::NumberArray;
using MetaPhysicL::NWrapper;
using MetaPhysicL::SemiDynamicSparseNumberArray;

typedef SemiDynamicSparseNumberArray<Real,
                                     libMesh::dof_id_type,
                                     NWrapper<MOOSE_AD_MAX_DOFS_PER_ELEM>>
    DNDerivativeType;

template <std::size_t N>
using DNDerivativeSize = SemiDynamicSparseNumberArray<Real, libMesh::dof_id_type, NWrapper<N>>;

typedef DualNumber<Real, DNDerivativeType, /*allow_skiping_derivatives=*/true> DualReal;
