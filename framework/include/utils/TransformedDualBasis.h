//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/enum_elem_type.h"

namespace Moose
{
namespace Mortar
{
/**
 * Whether libMesh biorthogonalizes the dual basis against the locally quadratic transformed basis
 * on this lower-dimensional face element type.
 *
 * libMesh applies that transform in FEGenericBase::compute_dual_shape_coeffs, automatically and
 * without an option, on exactly the second-order Lagrange faces whose untransformed weight
 * integral(N_k) is not positive: TRI6 vertices integrate to zero and QUAD8 corners to -1/3. QUAD9
 * is excluded because its weights are already positive, and every other type keeps the standard
 * dual construction. libMesh documents the consequence on FEBase::get_dual_phi().
 *
 * This predicate mirrors that element-type rule so MOOSE can report the two user-visible
 * consequences of the transform -- that the coupling matrix D is no longer diagonal, and that the
 * Petrov-Galerkin approach cannot be combined with it. Keep it in step with libMesh if the set of
 * transformed types there ever changes.
 */
inline bool
transformedDualBasisSupported(const libMesh::ElemType elem_type)
{
  return elem_type == libMesh::TRI6 || elem_type == libMesh::QUAD8;
}
}
}
