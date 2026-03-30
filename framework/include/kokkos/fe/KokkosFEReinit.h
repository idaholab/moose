//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Only compiled when device-native FE evaluators are active.
// Include this header only when MOOSE_KOKKOS_ONDEMAND_FE is defined, or
// from files that are compiled with the device compiler and have native FE
// evaluators available.

#ifdef MOOSE_KOKKOS_SCOPE

#include "KokkosFEBase.h"
#include "KokkosFETypes.h"
#include "KokkosFELagrange1D.h"
#include "KokkosFELagrange2D.h"
#include "KokkosFELagrange3D.h"

namespace Moose::Kokkos
{

// ── On-device Lagrange shape function dispatch ────────────────────────────────
//
// These two free functions replace table lookups (assembly.getPhi()(i, qp) etc.)
// with direct calls to FEEvaluator::shape / grad_shape.  They are called inside
// GPU kernels when MOOSE_KOKKOS_ONDEMAND_FE is defined, eliminating the need
// for the large _phi / _grad_phi / _phi_face / _grad_phi_face device arrays.
//
// topo  — element topology for the current element (from Assembly::getElemTopology)
// i     — element-local DOF index
// xi, eta, zeta — reference-element coordinates of the quadrature point
//                 (volume: from _q_points; face: from _q_points_face_parent)

/// Evaluate the i-th Lagrange shape function at (xi, eta, zeta).
KOKKOS_INLINE_FUNCTION Real
nativeShape(FEElemTopology topo, unsigned int i, Real xi, Real eta, Real zeta)
{
  switch (topo)
  {
    case FEElemTopology::EDGE2:
      return FEEvaluator<LagrangeTag, Edge2Tag>::shape(i, xi, eta, zeta);
    case FEElemTopology::EDGE3:
      return FEEvaluator<LagrangeTag, Edge3Tag>::shape(i, xi, eta, zeta);
    case FEElemTopology::TRI3:
      return FEEvaluator<LagrangeTag, Tri3Tag>::shape(i, xi, eta, zeta);
    case FEElemTopology::TRI6:
      return FEEvaluator<LagrangeTag, Tri6Tag>::shape(i, xi, eta, zeta);
    case FEElemTopology::QUAD4:
      return FEEvaluator<LagrangeTag, Quad4Tag>::shape(i, xi, eta, zeta);
    case FEElemTopology::QUAD8:
      return FEEvaluator<LagrangeTag, Quad8Tag>::shape(i, xi, eta, zeta);
    case FEElemTopology::QUAD9:
      return FEEvaluator<LagrangeTag, Quad9Tag>::shape(i, xi, eta, zeta);
    case FEElemTopology::TET4:
      return FEEvaluator<LagrangeTag, Tet4Tag>::shape(i, xi, eta, zeta);
    case FEElemTopology::TET10:
      return FEEvaluator<LagrangeTag, Tet10Tag>::shape(i, xi, eta, zeta);
    case FEElemTopology::HEX8:
      return FEEvaluator<LagrangeTag, Hex8Tag>::shape(i, xi, eta, zeta);
    case FEElemTopology::HEX20:
      return FEEvaluator<LagrangeTag, Hex20Tag>::shape(i, xi, eta, zeta);
    case FEElemTopology::HEX27:
      return FEEvaluator<LagrangeTag, Hex27Tag>::shape(i, xi, eta, zeta);
    default:
      return Real(0);
  }
}

/// Evaluate the reference-space gradient of the i-th Lagrange shape function.
/// The physical gradient is obtained by multiplying by the inverse Jacobian:
///   grad_u_physical = J * nativeGradShape(...)
/// which is the same transform applied by VariableTestGradient / VariablePhiGradient.
KOKKOS_INLINE_FUNCTION Real3
nativeGradShape(FEElemTopology topo, unsigned int i, Real xi, Real eta, Real zeta)
{
  switch (topo)
  {
    case FEElemTopology::EDGE2:
      return FEEvaluator<LagrangeTag, Edge2Tag>::grad_shape(i, xi, eta, zeta);
    case FEElemTopology::EDGE3:
      return FEEvaluator<LagrangeTag, Edge3Tag>::grad_shape(i, xi, eta, zeta);
    case FEElemTopology::TRI3:
      return FEEvaluator<LagrangeTag, Tri3Tag>::grad_shape(i, xi, eta, zeta);
    case FEElemTopology::TRI6:
      return FEEvaluator<LagrangeTag, Tri6Tag>::grad_shape(i, xi, eta, zeta);
    case FEElemTopology::QUAD4:
      return FEEvaluator<LagrangeTag, Quad4Tag>::grad_shape(i, xi, eta, zeta);
    case FEElemTopology::QUAD8:
      return FEEvaluator<LagrangeTag, Quad8Tag>::grad_shape(i, xi, eta, zeta);
    case FEElemTopology::QUAD9:
      return FEEvaluator<LagrangeTag, Quad9Tag>::grad_shape(i, xi, eta, zeta);
    case FEElemTopology::TET4:
      return FEEvaluator<LagrangeTag, Tet4Tag>::grad_shape(i, xi, eta, zeta);
    case FEElemTopology::TET10:
      return FEEvaluator<LagrangeTag, Tet10Tag>::grad_shape(i, xi, eta, zeta);
    case FEElemTopology::HEX8:
      return FEEvaluator<LagrangeTag, Hex8Tag>::grad_shape(i, xi, eta, zeta);
    case FEElemTopology::HEX20:
      return FEEvaluator<LagrangeTag, Hex20Tag>::grad_shape(i, xi, eta, zeta);
    case FEElemTopology::HEX27:
      return FEEvaluator<LagrangeTag, Hex27Tag>::grad_shape(i, xi, eta, zeta);
    default:
      return Real3(0, 0, 0);
  }
}

} // namespace Moose::Kokkos

#endif // MOOSE_KOKKOS_SCOPE
