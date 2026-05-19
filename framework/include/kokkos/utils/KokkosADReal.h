//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADRealForward.h"

namespace Moose::Kokkos
{

using MetaPhysicL::KokkosSemiDynamicSparseNumberArray;

typedef KokkosSemiDynamicSparseNumberArray<Real,
                                           libMesh::dof_id_type,
                                           NWrapper<MOOSE_AD_MAX_DOFS_PER_ELEM>>
    DNDerivativeType;

template <std::size_t N>
using DNDerivativeSize =
    KokkosSemiDynamicSparseNumberArray<Real, libMesh::dof_id_type, NWrapper<N>>;

typedef DualNumber<Real, DNDerivativeType, false> ADReal;

} // namespace Moose::Kokkos
