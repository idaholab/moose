//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_common.h"
#include "metaphysicl/metaphysicl_version.h"

#include <array>

#if METAPHYSICL_MAJOR_VERSION < 1
namespace MetaPhysicL
{
template <typename, typename>
class DualNumber;
}
#else
#include "metaphysicl/dualnumber_forward.h"
#endif

#include "metaphysicl/semidynamicsparsenumberarray_decl.h"

using libMesh::Real;
using MetaPhysicL::DualNumber;
using MetaPhysicL::NWrapper;
using MetaPhysicL::SemiDynamicSparseNumberArray;

typedef SemiDynamicSparseNumberArray<Real,
                                     libMesh::dof_id_type,
                                     NWrapper<MOOSE_AD_MAX_DOFS_PER_ELEM>>
    DNDerivativeType;

template <std::size_t N>
using DNDerivativeSize = SemiDynamicSparseNumberArray<Real, libMesh::dof_id_type, NWrapper<N>>;

typedef DualNumber<Real, DNDerivativeType, /*allow_skiping_derivatives=*/true> ADReal;
