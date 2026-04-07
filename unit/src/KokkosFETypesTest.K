//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"

#include "libmesh/kokkos/fe_types.h"

#include "libmesh/enum_elem_type.h"
#include "libmesh/enum_fe_family.h"

using namespace libMesh::Kokkos;

// ── toKokkosTopology ──────────────────────────────────────────────────────────

TEST(KokkosFETypesTest, ToKokkosTopology_Edge)
{
  EXPECT_EQ(toKokkosTopology(libMesh::EDGE2), FEElemTopology::EDGE2);
  EXPECT_EQ(toKokkosTopology(libMesh::EDGE3), FEElemTopology::EDGE3);
}

TEST(KokkosFETypesTest, ToKokkosTopology_2D)
{
  EXPECT_EQ(toKokkosTopology(libMesh::TRI3),  FEElemTopology::TRI3);
  EXPECT_EQ(toKokkosTopology(libMesh::TRI6),  FEElemTopology::TRI6);
  EXPECT_EQ(toKokkosTopology(libMesh::QUAD4), FEElemTopology::QUAD4);
  EXPECT_EQ(toKokkosTopology(libMesh::QUAD8), FEElemTopology::QUAD8);
  EXPECT_EQ(toKokkosTopology(libMesh::QUAD9), FEElemTopology::QUAD9);
}

TEST(KokkosFETypesTest, ToKokkosTopology_3D)
{
  EXPECT_EQ(toKokkosTopology(libMesh::TET4),  FEElemTopology::TET4);
  EXPECT_EQ(toKokkosTopology(libMesh::TET10), FEElemTopology::TET10);
  EXPECT_EQ(toKokkosTopology(libMesh::HEX8),  FEElemTopology::HEX8);
  EXPECT_EQ(toKokkosTopology(libMesh::HEX20), FEElemTopology::HEX20);
  EXPECT_EQ(toKokkosTopology(libMesh::HEX27), FEElemTopology::HEX27);
}

// ── toKokkosFamily ────────────────────────────────────────────────────────────

TEST(KokkosFETypesTest, ToKokkosFamily)
{
  EXPECT_EQ(toKokkosFamily(libMesh::LAGRANGE),     FEFamily::LAGRANGE);
  EXPECT_EQ(toKokkosFamily(libMesh::LAGRANGE_VEC), FEFamily::LAGRANGE_VEC);
  EXPECT_EQ(toKokkosFamily(libMesh::HERMITE),      FEFamily::HERMITE);
}

// ── getSideTopology ───────────────────────────────────────────────────────────

TEST(KokkosFETypesTest, GetSideTopology_1D)
{
  // 1D edge "sides" are vertex nodes; represented as EDGE2
  EXPECT_EQ(getSideTopology(FEElemTopology::EDGE2), FEElemTopology::EDGE2);
  EXPECT_EQ(getSideTopology(FEElemTopology::EDGE3), FEElemTopology::EDGE2);
}

TEST(KokkosFETypesTest, GetSideTopology_2D_FirstOrder)
{
  // First-order 2D elements have linear edge sides
  EXPECT_EQ(getSideTopology(FEElemTopology::TRI3),  FEElemTopology::EDGE2);
  EXPECT_EQ(getSideTopology(FEElemTopology::QUAD4), FEElemTopology::EDGE2);
}

TEST(KokkosFETypesTest, GetSideTopology_2D_SecondOrder)
{
  // Second-order 2D elements have quadratic edge sides
  EXPECT_EQ(getSideTopology(FEElemTopology::TRI6),  FEElemTopology::EDGE3);
  EXPECT_EQ(getSideTopology(FEElemTopology::QUAD8), FEElemTopology::EDGE3);
  EXPECT_EQ(getSideTopology(FEElemTopology::QUAD9), FEElemTopology::EDGE3);
}

TEST(KokkosFETypesTest, GetSideTopology_3D_FirstOrder)
{
  EXPECT_EQ(getSideTopology(FEElemTopology::TET4), FEElemTopology::TRI3);
  EXPECT_EQ(getSideTopology(FEElemTopology::HEX8), FEElemTopology::QUAD4);
}

TEST(KokkosFETypesTest, GetSideTopology_3D_SecondOrder)
{
  EXPECT_EQ(getSideTopology(FEElemTopology::TET10), FEElemTopology::TRI6);
  EXPECT_EQ(getSideTopology(FEElemTopology::HEX20), FEElemTopology::QUAD8);
  EXPECT_EQ(getSideTopology(FEElemTopology::HEX27), FEElemTopology::QUAD9);
}

// ── nDofs ─────────────────────────────────────────────────────────────────────

TEST(KokkosFETypesTest, NDofs_Lagrange_1D)
{
  EXPECT_EQ(nDofs(FEFamily::LAGRANGE, FEElemTopology::EDGE2), 2u);
  EXPECT_EQ(nDofs(FEFamily::LAGRANGE, FEElemTopology::EDGE3), 3u);
}

TEST(KokkosFETypesTest, NDofs_Lagrange_2D)
{
  EXPECT_EQ(nDofs(FEFamily::LAGRANGE, FEElemTopology::TRI3),  3u);
  EXPECT_EQ(nDofs(FEFamily::LAGRANGE, FEElemTopology::TRI6),  6u);
  EXPECT_EQ(nDofs(FEFamily::LAGRANGE, FEElemTopology::QUAD4), 4u);
  EXPECT_EQ(nDofs(FEFamily::LAGRANGE, FEElemTopology::QUAD8), 8u);
  EXPECT_EQ(nDofs(FEFamily::LAGRANGE, FEElemTopology::QUAD9), 9u);
}

TEST(KokkosFETypesTest, NDofs_Lagrange_3D)
{
  EXPECT_EQ(nDofs(FEFamily::LAGRANGE, FEElemTopology::TET4),  4u);
  EXPECT_EQ(nDofs(FEFamily::LAGRANGE, FEElemTopology::TET10), 10u);
  EXPECT_EQ(nDofs(FEFamily::LAGRANGE, FEElemTopology::HEX8),  8u);
  EXPECT_EQ(nDofs(FEFamily::LAGRANGE, FEElemTopology::HEX20), 20u);
  EXPECT_EQ(nDofs(FEFamily::LAGRANGE, FEElemTopology::HEX27), 27u);
}

// ── toKokkosFamily — MONOMIAL and UNKNOWN sentinel ────────────────────────────

TEST(KokkosFETypesTest, ToKokkosFamily_Monomial)
{
  EXPECT_EQ(toKokkosFamily(libMesh::MONOMIAL),     FEFamily::MONOMIAL);
  EXPECT_EQ(toKokkosFamily(libMesh::MONOMIAL_VEC), FEFamily::MONOMIAL_VEC);
}

TEST(KokkosFETypesTest, ToKokkosFamily_UnknownReturnsUnknownSentinel)
{
  // Unrecognized libMesh families must return UNKNOWN rather than abort.
  // HIERARCHIC is a well-known alternative that we do not natively support.
  EXPECT_EQ(toKokkosFamily(libMesh::HIERARCHIC), FEFamily::UNKNOWN);
  // UNKNOWN must not equal any real family
  EXPECT_NE(toKokkosFamily(libMesh::HIERARCHIC), FEFamily::LAGRANGE);
  EXPECT_NE(toKokkosFamily(libMesh::HIERARCHIC), FEFamily::MONOMIAL);
}

// ── classFromTopology ─────────────────────────────────────────────────────────

TEST(KokkosFETypesTest, ClassFromTopology_1D)
{
  EXPECT_EQ(classFromTopology(FEElemTopology::EDGE2), FEElemClass::EDGE);
  EXPECT_EQ(classFromTopology(FEElemTopology::EDGE3), FEElemClass::EDGE);
}

TEST(KokkosFETypesTest, ClassFromTopology_2D)
{
  EXPECT_EQ(classFromTopology(FEElemTopology::TRI3),  FEElemClass::TRI);
  EXPECT_EQ(classFromTopology(FEElemTopology::TRI6),  FEElemClass::TRI);
  EXPECT_EQ(classFromTopology(FEElemTopology::QUAD4), FEElemClass::QUAD);
  EXPECT_EQ(classFromTopology(FEElemTopology::QUAD8), FEElemClass::QUAD);
  EXPECT_EQ(classFromTopology(FEElemTopology::QUAD9), FEElemClass::QUAD);
}

TEST(KokkosFETypesTest, ClassFromTopology_3D)
{
  EXPECT_EQ(classFromTopology(FEElemTopology::TET4),  FEElemClass::TET);
  EXPECT_EQ(classFromTopology(FEElemTopology::TET10), FEElemClass::TET);
  EXPECT_EQ(classFromTopology(FEElemTopology::HEX8),  FEElemClass::HEX);
  EXPECT_EQ(classFromTopology(FEElemTopology::HEX20), FEElemClass::HEX);
  EXPECT_EQ(classFromTopology(FEElemTopology::HEX27), FEElemClass::HEX);
}

// ── nDofs(FEShapeKey) — LAGRANGE ─────────────────────────────────────────────

TEST(KokkosFETypesTest, NDofs_FEShapeKey_Lagrange_1D)
{
  EXPECT_EQ(nDofs(FEShapeKey{FEFamily::LAGRANGE, FEElemClass::EDGE, 1}), 2u);
  EXPECT_EQ(nDofs(FEShapeKey{FEFamily::LAGRANGE, FEElemClass::EDGE, 2}), 3u);
}

TEST(KokkosFETypesTest, NDofs_FEShapeKey_Lagrange_2D)
{
  EXPECT_EQ(nDofs(FEShapeKey{FEFamily::LAGRANGE, FEElemClass::TRI,  1}),  3u);
  EXPECT_EQ(nDofs(FEShapeKey{FEFamily::LAGRANGE, FEElemClass::TRI,  2}),  6u);
  EXPECT_EQ(nDofs(FEShapeKey{FEFamily::LAGRANGE, FEElemClass::QUAD, 1}),  4u);
  EXPECT_EQ(nDofs(FEShapeKey{FEFamily::LAGRANGE, FEElemClass::QUAD, 2}),  9u);
}

TEST(KokkosFETypesTest, NDofs_FEShapeKey_Lagrange_3D)
{
  EXPECT_EQ(nDofs(FEShapeKey{FEFamily::LAGRANGE, FEElemClass::TET, 1}),  4u);
  EXPECT_EQ(nDofs(FEShapeKey{FEFamily::LAGRANGE, FEElemClass::TET, 2}), 10u);
  EXPECT_EQ(nDofs(FEShapeKey{FEFamily::LAGRANGE, FEElemClass::HEX, 1}),  8u);
  EXPECT_EQ(nDofs(FEShapeKey{FEFamily::LAGRANGE, FEElemClass::HEX, 2}), 27u);
}

// ── nDofs(FEShapeKey) — MONOMIAL ─────────────────────────────────────────────
// 1D: p+1  |  2D: (p+1)(p+2)/2  |  3D: (p+1)(p+2)(p+3)/6

TEST(KokkosFETypesTest, NDofs_FEShapeKey_Monomial_1D)
{
  for (unsigned int p = 0; p <= 5; ++p)
    EXPECT_EQ(nDofs(FEShapeKey{FEFamily::MONOMIAL, FEElemClass::EDGE, p}), p + 1) << "p=" << p;
}

TEST(KokkosFETypesTest, NDofs_FEShapeKey_Monomial_2D_TRI)
{
  for (unsigned int p = 0; p <= 5; ++p)
    EXPECT_EQ(nDofs(FEShapeKey{FEFamily::MONOMIAL, FEElemClass::TRI, p}),
              (p + 1) * (p + 2) / 2) << "p=" << p;
}

TEST(KokkosFETypesTest, NDofs_FEShapeKey_Monomial_2D_QUAD)
{
  // TRI and QUAD share the same Dim2 formula (total-degree polynomial space)
  for (unsigned int p = 0; p <= 5; ++p)
    EXPECT_EQ(nDofs(FEShapeKey{FEFamily::MONOMIAL, FEElemClass::QUAD, p}),
              (p + 1) * (p + 2) / 2) << "p=" << p;
}

TEST(KokkosFETypesTest, NDofs_FEShapeKey_Monomial_3D_TET)
{
  for (unsigned int p = 0; p <= 5; ++p)
    EXPECT_EQ(nDofs(FEShapeKey{FEFamily::MONOMIAL, FEElemClass::TET, p}),
              (p + 1) * (p + 2) * (p + 3) / 6) << "p=" << p;
}

TEST(KokkosFETypesTest, NDofs_FEShapeKey_Monomial_3D_HEX)
{
  // TET and HEX share the same Dim3 formula
  for (unsigned int p = 0; p <= 5; ++p)
    EXPECT_EQ(nDofs(FEShapeKey{FEFamily::MONOMIAL, FEElemClass::HEX, p}),
              (p + 1) * (p + 2) * (p + 3) / 6) << "p=" << p;
}

// ── Enum values are contiguous and usable as array indices ───────────────────

TEST(KokkosFETypesTest, EnumValuesAreContiguous)
{
  // FEElemTopology values must be contiguous from 0 so they can be used as
  // array indices in initShapeNative().
  EXPECT_EQ(static_cast<unsigned int>(FEElemTopology::EDGE2),  0u);
  EXPECT_EQ(static_cast<unsigned int>(FEElemTopology::EDGE3),  1u);
  EXPECT_EQ(static_cast<unsigned int>(FEElemTopology::TRI3),   2u);
  EXPECT_EQ(static_cast<unsigned int>(FEElemTopology::TRI6),   3u);
  EXPECT_EQ(static_cast<unsigned int>(FEElemTopology::QUAD4),  4u);
  EXPECT_EQ(static_cast<unsigned int>(FEElemTopology::QUAD8),  5u);
  EXPECT_EQ(static_cast<unsigned int>(FEElemTopology::QUAD9),  6u);
  EXPECT_EQ(static_cast<unsigned int>(FEElemTopology::TET4),   7u);
  EXPECT_EQ(static_cast<unsigned int>(FEElemTopology::TET10),  8u);
  EXPECT_EQ(static_cast<unsigned int>(FEElemTopology::HEX8),   9u);
  EXPECT_EQ(static_cast<unsigned int>(FEElemTopology::HEX20), 10u);
  EXPECT_EQ(static_cast<unsigned int>(FEElemTopology::HEX27), 11u);
  EXPECT_EQ(static_cast<unsigned int>(FEElemTopology::N_TYPES), 12u);
}
