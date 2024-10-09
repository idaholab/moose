#include "ScaleIntegrator.h"

void ScaleIntegrator::SetIntRule(const IntegrationRule *ir)
{
  IntRule = ir;
  _integrator->SetIntRule(ir);
}

void ScaleIntegrator::AssembleElementMatrix(
   const FiniteElement &el, ElementTransformation &Trans, DenseMatrix &elmat)
{
  CheckIntegrator();
  _integrator->AssembleElementMatrix(el, Trans, elmat);
  _integrator->AssembleElementMatrix(el, Trans, elem_mat);
  elmat += elem_mat;
  elmat *= _scale;
}

void ScaleIntegrator::AssembleElementMatrix2(
   const FiniteElement &el1, const FiniteElement &el2,
   ElementTransformation &Trans, DenseMatrix &elmat)
{
  CheckIntegrator();
  _integrator->AssembleElementMatrix2(el1, el2, Trans, elmat);
  _integrator->AssembleElementMatrix2(el1, el2, Trans, elem_mat);
  elmat += elem_mat;
  elmat *= _scale;
}

void ScaleIntegrator::AssembleFaceMatrix(
   const FiniteElement &el1, const FiniteElement &el2,
   FaceElementTransformations &Trans, DenseMatrix &elmat)
{
  CheckIntegrator();
  _integrator->AssembleFaceMatrix(el1, el2, Trans, elmat);
  _integrator->AssembleFaceMatrix(el1, el2, Trans, elem_mat);
  elmat += elem_mat;
  elmat *= _scale;
}

void ScaleIntegrator::AssembleFaceMatrix(
   const FiniteElement &tr_fe,
   const FiniteElement &te_fe1, const FiniteElement &te_fe2,
   FaceElementTransformations &Trans, DenseMatrix &elmat)
{
  CheckIntegrator();
  _integrator->AssembleFaceMatrix(tr_fe, te_fe1, te_fe2, Trans, elmat);
  _integrator->AssembleFaceMatrix(tr_fe, te_fe1, te_fe2, Trans, elem_mat);
  elmat += elem_mat;
  elmat *= _scale;
}

void ScaleIntegrator::AssemblePA(const FiniteElementSpace& fes)
{
  _integrator->AssemblePA(fes);
}

void ScaleIntegrator::AssembleDiagonalPA(Vector &diag)
{
  _integrator->AssembleDiagonalPA(diag); 
}

void ScaleIntegrator::AssemblePAInteriorFaces(const FiniteElementSpace &fes)
{
  _integrator->AssemblePAInteriorFaces(fes);
}

void ScaleIntegrator::AssemblePABoundaryFaces(const FiniteElementSpace &fes)
{
  _integrator->AssemblePABoundaryFaces(fes);
}

void ScaleIntegrator::AddMultPA(const Vector& x, Vector& y) const
{
  _integrator->AddMultPA(x, y);
}

void ScaleIntegrator::AddMultTransposePA(const Vector &x, Vector &y) const
{
  _integrator->AddMultTransposePA(x, y);
}

void ScaleIntegrator::AssembleMF(const FiniteElementSpace &fes)
{
  _integrator->AssembleMF(fes);
}

void ScaleIntegrator::AddMultMF(const Vector& x, Vector& y) const
{
  _integrator->AddMultTransposeMF(x, y);
}

void ScaleIntegrator::AddMultTransposeMF(const Vector &x, Vector &y) const
{
  _integrator->AddMultMF(x, y);
}

void ScaleIntegrator::AssembleDiagonalMF(Vector &diag)
{
  _integrator->AssembleDiagonalMF(diag);
}

void ScaleIntegrator::AssembleEA(const FiniteElementSpace &fes, Vector &emat,
                               const bool add)
{
  _integrator->AssembleEA(fes, emat, add);
}

void ScaleIntegrator::AssembleEAInteriorFaces(const FiniteElementSpace &fes,
                                            Vector &ea_data_int,
                                            Vector &ea_data_ext,
                                            const bool add)
{
  _integrator->AssembleEAInteriorFaces(fes,ea_data_int,ea_data_ext,add);
}

void ScaleIntegrator::AssembleEABoundaryFaces(const FiniteElementSpace &fes,
                                            Vector &ea_data_bdr,
                                            const bool add)
{
  _integrator->AssembleEABoundaryFaces(fes, ea_data_bdr, add);
}

ScaleIntegrator::~ScaleIntegrator()
{
  if (_own_integrator)
    delete _integrator;
}