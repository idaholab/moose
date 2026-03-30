//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosFETypes.h"

#include "MooseError.h"

namespace Moose::Kokkos
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
      mooseError("Element type ", t, " is not supported by the Kokkos native FE path");
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
    default:
      mooseError("FE family ", f, " is not supported by the Kokkos native FE path");
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
      mooseError("getSideTopology: unknown FEElemTopology value ",
                 static_cast<unsigned int>(parent));
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
          mooseError("nDofs: unsupported topology ", static_cast<unsigned int>(topo),
                     " for LAGRANGE family");
      }
    }

    case FEFamily::HERMITE:
      mooseError("nDofs: HERMITE family is not supported by the native FE path; "
                 "use the libMesh fallback path instead");

    case FEFamily::LAGRANGE_VEC:
      mooseError("nDofs: LAGRANGE_VEC requires a spatial dimension argument; "
                 "not yet supported by the native FE path");

    default:
      mooseError("nDofs: unsupported FE family ", static_cast<unsigned int>(family));
  }
}

} // namespace Moose::Kokkos
