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
#include "KokkosFEMonomial.h"

namespace Moose::Kokkos
{

// ── On-device helpers: element class → spatial dimension ─────────────────────

KOKKOS_INLINE_FUNCTION unsigned int
dimFromClass(FEElemClass cls)
{
  switch (cls)
  {
    case FEElemClass::EDGE:
      return 1;
    case FEElemClass::TRI:
    case FEElemClass::QUAD:
      return 2;
    case FEElemClass::TET:
    case FEElemClass::HEX:
    case FEElemClass::PRISM:
    case FEElemClass::PYRAMID:
      return 3;
    default:
      return 0;
  }
}

// ── On-device helper: (class, order) → canonical Lagrange topology ────────────
// Returns the FEElemTopology whose FEEvaluator<LagrangeTag, ...> implements the
// Lagrange space of the given order on the given element class.
// Must be KOKKOS_INLINE_FUNCTION because it is called from nativeShape() on device.

KOKKOS_INLINE_FUNCTION FEElemTopology
lagrangeTopologyForClassAndOrder(FEElemClass cls, unsigned int order)
{
  switch (cls)
  {
    case FEElemClass::EDGE:
      switch (order)
      {
        case 1: return FEElemTopology::EDGE2;
        case 2: return FEElemTopology::EDGE3;
        default: return FEElemTopology::EDGE2; // unsupported — caller returns 0
      }
    case FEElemClass::TRI:
      switch (order)
      {
        case 1: return FEElemTopology::TRI3;
        case 2: return FEElemTopology::TRI6;
        default: return FEElemTopology::TRI3; // unsupported
      }
    case FEElemClass::QUAD:
      switch (order)
      {
        case 1: return FEElemTopology::QUAD4;
        case 2: return FEElemTopology::QUAD9;
        default: return FEElemTopology::QUAD4; // unsupported
      }
    case FEElemClass::TET:
      switch (order)
      {
        case 1: return FEElemTopology::TET4;
        case 2: return FEElemTopology::TET10;
        default: return FEElemTopology::TET4; // unsupported
      }
    case FEElemClass::HEX:
      switch (order)
      {
        case 1: return FEElemTopology::HEX8;
        case 2: return FEElemTopology::HEX27;
        default: return FEElemTopology::HEX8; // unsupported
      }
    default:
      return FEElemTopology::EDGE2; // PRISM/PYRAMID unsupported yet
  }
}

// ── Geometry-only shape dispatch (topology-based) ─────────────────────────────
//
// Replaces the old nativeShape(FEElemTopology, ...).  Used internally by the
// CPU-side mapFaceQpToParent() to perform the isoparametric mapping from face
// reference coordinates to parent reference coordinates.  Always native Lagrange
// at the element's own order — independent of the physics FE space.

/// Evaluate the i-th isoparametric Lagrange shape function at (xi, eta, zeta).
KOKKOS_INLINE_FUNCTION Real
nativeMapShape(FEElemTopology topo, unsigned int i, Real xi, Real eta, Real zeta)
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

/// Evaluate the reference-space gradient of the i-th isoparametric Lagrange shape function.
KOKKOS_INLINE_FUNCTION Real3
nativeGradMapShape(FEElemTopology topo, unsigned int i, Real xi, Real eta, Real zeta)
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

// ── Physics shape dispatch (FEShapeKey-based) ─────────────────────────────────
//
// These replace the old nativeShape / nativeGradShape overloads.  They dispatch
// on FEShapeKey, which encodes (family, element class, polynomial order).
//
// For LAGRANGE: resolves (class, order) to the canonical topology and delegates
//   to the existing FEEvaluator<LagrangeTag, TopoTag> specialisations.
// For MONOMIAL: dispatches on spatial dimension and order to
//   FEEvaluator<MonomialTag, DimNTag, Order> specialisations.

/// Evaluate the i-th physics shape function at (xi, eta, zeta).
KOKKOS_INLINE_FUNCTION Real
nativeShape(FEShapeKey key, unsigned int i, Real xi, Real eta, Real zeta)
{
  switch (key.family)
  {
    case FEFamily::LAGRANGE:
    {
      switch (lagrangeTopologyForClassAndOrder(key.cls, key.order))
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

    case FEFamily::MONOMIAL:
    {
      // TRI and QUAD share Dim2Tag; TET/HEX/PRISM/PYRAMID share Dim3Tag.
      switch (dimFromClass(key.cls))
      {
        case 1:
          switch (key.order)
          {
            case 0: return FEEvaluator<MonomialTag, Dim1Tag, 0>::shape(i, xi, eta, zeta);
            case 1: return FEEvaluator<MonomialTag, Dim1Tag, 1>::shape(i, xi, eta, zeta);
            case 2: return FEEvaluator<MonomialTag, Dim1Tag, 2>::shape(i, xi, eta, zeta);
            case 3: return FEEvaluator<MonomialTag, Dim1Tag, 3>::shape(i, xi, eta, zeta);
            case 4: return FEEvaluator<MonomialTag, Dim1Tag, 4>::shape(i, xi, eta, zeta);
            case 5: return FEEvaluator<MonomialTag, Dim1Tag, 5>::shape(i, xi, eta, zeta);
            default: return Real(0);
          }
        case 2:
          switch (key.order)
          {
            case 0: return FEEvaluator<MonomialTag, Dim2Tag, 0>::shape(i, xi, eta, zeta);
            case 1: return FEEvaluator<MonomialTag, Dim2Tag, 1>::shape(i, xi, eta, zeta);
            case 2: return FEEvaluator<MonomialTag, Dim2Tag, 2>::shape(i, xi, eta, zeta);
            case 3: return FEEvaluator<MonomialTag, Dim2Tag, 3>::shape(i, xi, eta, zeta);
            case 4: return FEEvaluator<MonomialTag, Dim2Tag, 4>::shape(i, xi, eta, zeta);
            case 5: return FEEvaluator<MonomialTag, Dim2Tag, 5>::shape(i, xi, eta, zeta);
            default: return Real(0);
          }
        case 3:
          switch (key.order)
          {
            case 0: return FEEvaluator<MonomialTag, Dim3Tag, 0>::shape(i, xi, eta, zeta);
            case 1: return FEEvaluator<MonomialTag, Dim3Tag, 1>::shape(i, xi, eta, zeta);
            case 2: return FEEvaluator<MonomialTag, Dim3Tag, 2>::shape(i, xi, eta, zeta);
            case 3: return FEEvaluator<MonomialTag, Dim3Tag, 3>::shape(i, xi, eta, zeta);
            case 4: return FEEvaluator<MonomialTag, Dim3Tag, 4>::shape(i, xi, eta, zeta);
            case 5: return FEEvaluator<MonomialTag, Dim3Tag, 5>::shape(i, xi, eta, zeta);
            default: return Real(0);
          }
        default:
          return Real(0);
      }
    }

    default:
      return Real(0);
  }
}

/// Evaluate the reference-space gradient of the i-th physics shape function.
/// The physical gradient is obtained by: grad_u_physical = J * nativeGradShape(key, ...)
KOKKOS_INLINE_FUNCTION Real3
nativeGradShape(FEShapeKey key, unsigned int i, Real xi, Real eta, Real zeta)
{
  switch (key.family)
  {
    case FEFamily::LAGRANGE:
    {
      switch (lagrangeTopologyForClassAndOrder(key.cls, key.order))
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

    case FEFamily::MONOMIAL:
    {
      switch (dimFromClass(key.cls))
      {
        case 1:
          switch (key.order)
          {
            case 0: return FEEvaluator<MonomialTag, Dim1Tag, 0>::grad_shape(i, xi, eta, zeta);
            case 1: return FEEvaluator<MonomialTag, Dim1Tag, 1>::grad_shape(i, xi, eta, zeta);
            case 2: return FEEvaluator<MonomialTag, Dim1Tag, 2>::grad_shape(i, xi, eta, zeta);
            case 3: return FEEvaluator<MonomialTag, Dim1Tag, 3>::grad_shape(i, xi, eta, zeta);
            case 4: return FEEvaluator<MonomialTag, Dim1Tag, 4>::grad_shape(i, xi, eta, zeta);
            case 5: return FEEvaluator<MonomialTag, Dim1Tag, 5>::grad_shape(i, xi, eta, zeta);
            default: return Real3(0, 0, 0);
          }
        case 2:
          switch (key.order)
          {
            case 0: return FEEvaluator<MonomialTag, Dim2Tag, 0>::grad_shape(i, xi, eta, zeta);
            case 1: return FEEvaluator<MonomialTag, Dim2Tag, 1>::grad_shape(i, xi, eta, zeta);
            case 2: return FEEvaluator<MonomialTag, Dim2Tag, 2>::grad_shape(i, xi, eta, zeta);
            case 3: return FEEvaluator<MonomialTag, Dim2Tag, 3>::grad_shape(i, xi, eta, zeta);
            case 4: return FEEvaluator<MonomialTag, Dim2Tag, 4>::grad_shape(i, xi, eta, zeta);
            case 5: return FEEvaluator<MonomialTag, Dim2Tag, 5>::grad_shape(i, xi, eta, zeta);
            default: return Real3(0, 0, 0);
          }
        case 3:
          switch (key.order)
          {
            case 0: return FEEvaluator<MonomialTag, Dim3Tag, 0>::grad_shape(i, xi, eta, zeta);
            case 1: return FEEvaluator<MonomialTag, Dim3Tag, 1>::grad_shape(i, xi, eta, zeta);
            case 2: return FEEvaluator<MonomialTag, Dim3Tag, 2>::grad_shape(i, xi, eta, zeta);
            case 3: return FEEvaluator<MonomialTag, Dim3Tag, 3>::grad_shape(i, xi, eta, zeta);
            case 4: return FEEvaluator<MonomialTag, Dim3Tag, 4>::grad_shape(i, xi, eta, zeta);
            case 5: return FEEvaluator<MonomialTag, Dim3Tag, 5>::grad_shape(i, xi, eta, zeta);
            default: return Real3(0, 0, 0);
          }
        default:
          return Real3(0, 0, 0);
      }
    }

    default:
      return Real3(0, 0, 0);
  }
}

} // namespace Moose::Kokkos

#endif // MOOSE_KOKKOS_SCOPE
