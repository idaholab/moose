#ifdef MFEM_ENABLED

#include "gtest/gtest.h"
#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"
#include "ScaleIntegrator.h"

TEST(CheckData, ScaleIntegratorTestLegacy)
{
  mfem::Mesh mesh = mfem::Mesh::MakeCartesian3D(1, 1, 1, mfem::Element::HEXAHEDRON);
  mfem::H1_FECollection fec(2, mesh.Dimension());
  mfem::FiniteElementSpace fes(&mesh, &fec);

  mfem::SumIntegrator integ_sum(true);
  integ_sum.AddIntegrator(new mfem::MassIntegrator);
  integ_sum.AddIntegrator(new mfem::MassIntegrator);

  platypus::ScaleIntegrator integ_scale(new mfem::MassIntegrator, 2);

  const mfem::FiniteElement & el1 = *fes.GetFE(0);
  const mfem::FiniteElement & el2 = *fes.GetFE(0);
  mfem::ElementTransformation & T = *mesh.GetElementTransformation(0);
  mfem::DenseMatrix m1, m2;

  // AssembleElementMatrix
  integ_scale.AssembleElementMatrix(el1, T, m1);
  integ_sum.AssembleElementMatrix(el1, T, m2);
  m1 -= m2;
  EXPECT_NEAR(m1.MaxMaxNorm(), 0, 1e-12);

  // AssembleElementMatrix2
  integ_scale.AssembleElementMatrix2(el1, el2, T, m1);
  integ_sum.AssembleElementMatrix2(el1, el2, T, m2);
  m1 -= m2;
  EXPECT_NEAR(m1.MaxMaxNorm(), 0, 1e-12);

  // Test SetIntRule
  mfem::QuadratureSpace qs(&mesh, 1);
  const mfem::IntegrationRule & ir = qs.GetIntRule(0);

  EXPECT_NO_THROW({ integ_scale.SetIntRule(&ir); });
}

TEST(CheckData, ScaleIntegratorTestFaceAssembly)
{
  mfem::Mesh mesh = mfem::Mesh::MakeCartesian3D(2, 1, 1, mfem::Element::HEXAHEDRON);
  mfem::DG_FECollection fec(2, mesh.Dimension(), mfem::BasisType::GaussLobatto);
  mfem::FiniteElementSpace fes(&mesh, &fec);

  mfem::Vector v(mesh.Dimension());
  v = 1.0;
  mfem::VectorConstantCoefficient v_coeff(v);

  mfem::DGTraceIntegrator integ(v_coeff, 1.0, 2.0);
  mfem::SumIntegrator integ_sum(false);
  integ_sum.AddIntegrator(&integ);
  integ_sum.AddIntegrator(&integ);

  platypus::ScaleIntegrator integ_scale(&integ, 2, false);

  mfem::DenseMatrix m1, m2;

  // AssembleFaceMatrix
  int nfaces = mesh.GetNumFaces();
  for (int i = 0; i < nfaces; i++)
  {
    mfem::FaceElementTransformations * tr = mesh.GetFaceElementTransformations(i);
    const mfem::FiniteElement & el0 = *fes.GetFE(tr->Elem1No);
    const mfem::FiniteElement & el1 = (tr->Elem2No >= 0) ? *fes.GetFE(tr->Elem2No) : el0;
    integ_scale.AssembleFaceMatrix(el0, el1, *tr, m1);
    integ_sum.AssembleFaceMatrix(el0, el1, *tr, m2);
    m1 -= m2;

    EXPECT_NEAR(m1.MaxMaxNorm(), 0, 1e-12);
  }

  EXPECT_NO_THROW({
    integ_scale.AssemblePAInteriorFaces(fes);
    integ_scale.AssemblePABoundaryFaces(fes);
  });
}

TEST(CheckData, ScaleIntegratorTestPartial)
{
  mfem::Mesh mesh = mfem::Mesh::MakeCartesian3D(1, 1, 1, mfem::Element::HEXAHEDRON);
  mfem::H1_FECollection fec(2, mesh.Dimension());
  mfem::FiniteElementSpace fes(&mesh, &fec);

  mfem::SumIntegrator integ_sum(true);
  integ_sum.AddIntegrator(new mfem::MassIntegrator);
  integ_sum.AddIntegrator(new mfem::MassIntegrator);

  platypus::ScaleIntegrator integ_scale(new mfem::MassIntegrator, 2);

  mfem::DenseMatrix m1, m2;

  // Partial Assembly
  integ_sum.AssemblePA(fes);
  integ_scale.AssemblePA(fes);

  int n = fes.GetTrueVSize();
  mfem::Vector x(n), y1(n), y2(n);
  mfem::Vector diag1(n), diag2(n);
  x.Randomize(1);

  // AddMultPA
  y1 = 0.0;
  y2 = 0.0;
  integ_scale.AddMultPA(x, y1);
  integ_sum.AddMultPA(x, y2);
  y1 -= y2;
  EXPECT_NEAR(y1.Normlinf(), 0, 1e-12);

  // AddMultTransposePA
  y1 = 0.0;
  y2 = 0.0;
  integ_scale.AddMultTransposePA(x, y1);
  integ_sum.AddMultTransposePA(x, y2);
  y1 -= y2;
  EXPECT_NEAR(y1.Normlinf(), 0, 1e-12);

  // AssembleDiagonalPA
  diag1 = 0.0;
  diag2 = 0.0;
  integ_scale.AssembleDiagonalPA(diag1);
  integ_sum.AssembleDiagonalPA(diag2);
  diag1 -= diag2;
  EXPECT_NEAR(diag1.Normlinf(), 0, 1e-12);
}

#endif
