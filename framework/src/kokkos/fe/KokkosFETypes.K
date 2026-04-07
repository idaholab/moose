//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "libmesh/libmesh_config.h"

// When libMesh is built with Kokkos support these symbols are compiled into
// libmesh itself (src/kokkos/fe_types.C).  MOOSE must not define them a
// second time or the linker will see duplicate symbols.
#ifndef LIBMESH_HAVE_KOKKOS

#include "libmesh/kokkos/fe_types.h"

#include "libmesh/libmesh_common.h"

namespace libMesh::Kokkos
{

FEElemTopology
toKokkosTopology(libMesh::ElemType t)
{
  switch (t)
  {
    case libMesh::EDGE2:  return FEElemTopology::EDGE2;
    case libMesh::EDGE3:  return FEElemTopology::EDGE3;
    case libMesh::TRI3:   return FEElemTopology::TRI3;
    case libMesh::TRI6:   return FEElemTopology::TRI6;
    case libMesh::QUAD4:  return FEElemTopology::QUAD4;
    case libMesh::QUAD8:  return FEElemTopology::QUAD8;
    case libMesh::QUAD9:  return FEElemTopology::QUAD9;
    case libMesh::TET4:   return FEElemTopology::TET4;
    case libMesh::TET10:  return FEElemTopology::TET10;
    case libMesh::HEX8:   return FEElemTopology::HEX8;
    case libMesh::HEX20:  return FEElemTopology::HEX20;
    case libMesh::HEX27:  return FEElemTopology::HEX27;
    default:
      libmesh_error_msg("Element type " << t << " is not supported by the Kokkos native FE path");
  }
}

FEFamily
toKokkosFamily(libMesh::FEFamily f)
{
  switch (f)
  {
    case libMesh::LAGRANGE:     return FEFamily::LAGRANGE;
    case libMesh::LAGRANGE_VEC: return FEFamily::LAGRANGE_VEC;
    case libMesh::HERMITE:      return FEFamily::HERMITE;
    case libMesh::MONOMIAL:     return FEFamily::MONOMIAL;
    case libMesh::MONOMIAL_VEC: return FEFamily::MONOMIAL_VEC;
    default:
      // Return a sentinel rather than erroring — the shape-key table is populated
      // for every FE type in the problem, so an unrecognised family must not abort.
      // The family_supported gate in initShape() will route these types to the
      // libMesh phi-table fallback path.
      return FEFamily::UNKNOWN;
  }
}

FEElemTopology
getSideTopology(FEElemTopology parent)
{
  switch (parent)
  {
    // 1D: sides are vertex nodes, represented as EDGE2 (degenerate)
    case FEElemTopology::EDGE2:
    case FEElemTopology::EDGE3:
      return FEElemTopology::EDGE2;

    // 2D first-order: sides are linear edges
    case FEElemTopology::TRI3:
    case FEElemTopology::QUAD4:
      return FEElemTopology::EDGE2;

    // 2D second-order: sides are quadratic edges
    case FEElemTopology::TRI6:
    case FEElemTopology::QUAD8:
    case FEElemTopology::QUAD9:
      return FEElemTopology::EDGE3;

    // 3D first-order: sides are linear triangles / quads
    case FEElemTopology::TET4:
      return FEElemTopology::TRI3;
    case FEElemTopology::HEX8:
      return FEElemTopology::QUAD4;

    // 3D second-order: sides are quadratic triangles / quads
    case FEElemTopology::TET10:
      return FEElemTopology::TRI6;
    case FEElemTopology::HEX20:
      return FEElemTopology::QUAD8;
    case FEElemTopology::HEX27:
      return FEElemTopology::QUAD9;

    default:
      libmesh_error_msg("getSideTopology: unknown FEElemTopology value "
                 << static_cast<unsigned int>(parent));
  }
}

unsigned int
nDofs(FEFamily family, FEElemTopology topo)
{
  switch (family)
  {
    case FEFamily::LAGRANGE:
    {
      switch (topo)
      {
        case FEElemTopology::EDGE2:  return 2;
        case FEElemTopology::EDGE3:  return 3;
        case FEElemTopology::TRI3:   return 3;
        case FEElemTopology::TRI6:   return 6;
        case FEElemTopology::QUAD4:  return 4;
        case FEElemTopology::QUAD8:  return 8;
        case FEElemTopology::QUAD9:  return 9;
        case FEElemTopology::TET4:   return 4;
        case FEElemTopology::TET10:  return 10;
        case FEElemTopology::HEX8:   return 8;
        case FEElemTopology::HEX20:  return 20;
        case FEElemTopology::HEX27:  return 27;
        default:
          libmesh_error_msg("nDofs: unsupported topology " << static_cast<unsigned int>(topo)
                     << " for LAGRANGE family");
      }
    }

    case FEFamily::HERMITE:
      libmesh_error_msg("nDofs: HERMITE family is not supported by the native FE path; "
                 "use the libMesh fallback path instead");

    case FEFamily::LAGRANGE_VEC:
      libmesh_error_msg("nDofs: LAGRANGE_VEC requires a spatial dimension argument; "
                 "not yet supported by the native FE path");

    default:
      libmesh_error_msg("nDofs: unsupported FE family " << static_cast<unsigned int>(family));
  }
}

FEElemClass
classFromTopology(FEElemTopology topo)
{
  switch (topo)
  {
    case FEElemTopology::EDGE2:
    case FEElemTopology::EDGE3:
      return FEElemClass::EDGE;

    case FEElemTopology::TRI3:
    case FEElemTopology::TRI6:
      return FEElemClass::TRI;

    case FEElemTopology::QUAD4:
    case FEElemTopology::QUAD8:
    case FEElemTopology::QUAD9:
      return FEElemClass::QUAD;

    case FEElemTopology::TET4:
    case FEElemTopology::TET10:
      return FEElemClass::TET;

    case FEElemTopology::HEX8:
    case FEElemTopology::HEX20:
    case FEElemTopology::HEX27:
      return FEElemClass::HEX;

    default:
      libmesh_error_msg("classFromTopology: unsupported FEElemTopology value "
                 << static_cast<unsigned int>(topo));
  }
}

unsigned int
nDofs(FEShapeKey key)
{
  if (key.family == FEFamily::LAGRANGE)
  {
    // Map (class, order) to the canonical Lagrange topology, then return its node count.
    switch (key.cls)
    {
      case FEElemClass::EDGE:
        switch (key.order)
        {
          case 1: return 2;
          case 2: return 3;
          default: libmesh_error_msg("nDofs(FEShapeKey): unsupported LAGRANGE order " << key.order << " for EDGE");
        }
      case FEElemClass::TRI:
        switch (key.order)
        {
          case 1: return 3;
          case 2: return 6;
          default: libmesh_error_msg("nDofs(FEShapeKey): unsupported LAGRANGE order " << key.order << " for TRI");
        }
      case FEElemClass::QUAD:
        switch (key.order)
        {
          case 1: return 4;
          case 2: return 9;
          default: libmesh_error_msg("nDofs(FEShapeKey): unsupported LAGRANGE order " << key.order << " for QUAD");
        }
      case FEElemClass::TET:
        switch (key.order)
        {
          case 1: return 4;
          case 2: return 10;
          default: libmesh_error_msg("nDofs(FEShapeKey): unsupported LAGRANGE order " << key.order << " for TET");
        }
      case FEElemClass::HEX:
        switch (key.order)
        {
          case 1: return 8;
          case 2: return 27;
          default: libmesh_error_msg("nDofs(FEShapeKey): unsupported LAGRANGE order " << key.order << " for HEX");
        }
      case FEElemClass::PRISM:
        switch (key.order)
        {
          case 1: return 6;
          case 2: return 21;
          default: libmesh_error_msg("nDofs(FEShapeKey): unsupported LAGRANGE order " << key.order << " for PRISM");
        }
      case FEElemClass::PYRAMID:
        switch (key.order)
        {
          case 1: return 5;
          case 2: return 14;
          default: libmesh_error_msg("nDofs(FEShapeKey): unsupported LAGRANGE order " << key.order << " for PYRAMID");
        }
      default:
        libmesh_error_msg("nDofs(FEShapeKey): unsupported LAGRANGE element class "
                   << static_cast<unsigned int>(key.cls));
    }
  }
  else if (key.family == FEFamily::MONOMIAL)
  {
    // MONOMIAL uses the complete total-degree polynomial space P_p.
    // DOF count depends on spatial dimension only (TRI and QUAD share dim=2, etc.).
    const unsigned int p = key.order;
    switch (key.cls)
    {
      case FEElemClass::EDGE:
        return p + 1;
      case FEElemClass::TRI:
      case FEElemClass::QUAD:
        return (p + 1) * (p + 2) / 2;
      case FEElemClass::TET:
      case FEElemClass::HEX:
      case FEElemClass::PRISM:
      case FEElemClass::PYRAMID:
        return (p + 1) * (p + 2) * (p + 3) / 6;
      default:
        libmesh_error_msg("nDofs(FEShapeKey): unsupported MONOMIAL element class "
                   << static_cast<unsigned int>(key.cls));
    }
  }
  libmesh_error_msg("nDofs(FEShapeKey): unsupported FE family "
             << static_cast<unsigned int>(key.family));
}

} // namespace libMesh::Kokkos

#endif // !LIBMESH_HAVE_KOKKOS
