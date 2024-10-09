#include "ScaleIntegrator.h"

void ScaleIntegrator::SetIntRule(const mfem::IntegrationRule *ir)
{
  IntRule = ir;
  _integrator->SetIntRule(ir);
}

void ScaleIntegrator::AssembleElementMatrix(
   const mfem::FiniteElement &el, mfem::ElementTransformation &Trans, mfem::DenseMatrix &elmat)
{
  CheckIntegrator();
  _integrator->AssembleElementMatrix(el, Trans, elmat);
  elmat *= _scale;
}

void ScaleIntegrator::AssembleElementMatrix2(
   const mfem::FiniteElement &el1, const mfem::FiniteElement &el2,
   mfem::ElementTransformation &Trans, mfem::DenseMatrix &elmat)
{
  CheckIntegrator();
  _integrator->AssembleElementMatrix2(el1, el2, Trans, elmat);
  elmat *= _scale;
}

void ScaleIntegrator::AssembleFaceMatrix(
   const mfem::FiniteElement &el1, const mfem::FiniteElement &el2,
   mfem::FaceElementTransformations &Trans, mfem::DenseMatrix &elmat)
{
  CheckIntegrator();
  _integrator->AssembleFaceMatrix(el1, el2, Trans, elmat);
  elmat *= _scale;
}

void ScaleIntegrator::AssembleFaceMatrix(
   const mfem::FiniteElement &tr_fe,
   const mfem::FiniteElement &te_fe1, const mfem::FiniteElement &te_fe2,
   mfem::FaceElementTransformations &Trans, mfem::DenseMatrix &elmat)
{
  CheckIntegrator();
  _integrator->AssembleFaceMatrix(tr_fe, te_fe1, te_fe2, Trans, elmat);
  elmat *= _scale;
}

void ScaleIntegrator::AssemblePA(const mfem::FiniteElementSpace& fes)
{
  _integrator->AssemblePA(fes);
}

void ScaleIntegrator::AssembleDiagonalPA(mfem::Vector &diag)
{
  _integrator->AssembleDiagonalPA(diag); 
}

void ScaleIntegrator::AssemblePAInteriorFaces(const mfem::FiniteElementSpace &fes)
{
  _integrator->AssemblePAInteriorFaces(fes);
}

void ScaleIntegrator::AssemblePABoundaryFaces(const mfem::FiniteElementSpace &fes)
{
  _integrator->AssemblePABoundaryFaces(fes);
}

void ScaleIntegrator::AddMultPA(const mfem::Vector& x, mfem::Vector& y) const
{
  _integrator->AddMultPA(x, y);
}

void ScaleIntegrator::AddMultTransposePA(const mfem::Vector &x, mfem::Vector &y) const
{
  _integrator->AddMultTransposePA(x, y);
}

void ScaleIntegrator::AssembleMF(const mfem::FiniteElementSpace &fes)
{
  _integrator->AssembleMF(fes);
}

void ScaleIntegrator::AddMultMF(const mfem::Vector& x, mfem::Vector& y) const
{
  _integrator->AddMultTransposeMF(x, y);
}

void ScaleIntegrator::AddMultTransposeMF(const mfem::Vector &x, mfem::Vector &y) const
{
  _integrator->AddMultMF(x, y);
}

void ScaleIntegrator::AssembleDiagonalMF(mfem::Vector &diag)
{
  _integrator->AssembleDiagonalMF(diag);
}

void ScaleIntegrator::AssembleEA(const mfem::FiniteElementSpace &fes, mfem::Vector &emat,
                               const bool add)
{
  _integrator->AssembleEA(fes, emat, add);
}

void ScaleIntegrator::AssembleEAInteriorFaces(const mfem::FiniteElementSpace &fes,
                                            mfem::Vector &ea_data_int,
                                            mfem::Vector &ea_data_ext,
                                            const bool add)
{
  _integrator->AssembleEAInteriorFaces(fes,ea_data_int,ea_data_ext,add);
}

void ScaleIntegrator::AssembleEABoundaryFaces(const mfem::FiniteElementSpace &fes,
                                            mfem::Vector &ea_data_bdr,
                                            const bool add)
{
  _integrator->AssembleEABoundaryFaces(fes, ea_data_bdr, add);
}

ScaleIntegrator::~ScaleIntegrator()
{
  if (_own_integrator)
    delete _integrator;
}