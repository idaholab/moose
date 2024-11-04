#include "ScaleIntegrator.h"

namespace platypus
{

void
ScaleIntegrator::SetIntRule(const mfem::IntegrationRule * ir)
{
  IntRule = ir;
  _integrator->SetIntRule(ir);
}

void
ScaleIntegrator::AssembleElementMatrix(const mfem::FiniteElement & el,
                                       mfem::ElementTransformation & Trans,
                                       mfem::DenseMatrix & elmat)
{
  CheckIntegrator();
  _integrator->AssembleElementMatrix(el, Trans, elmat);
  elmat *= _scale;
}

void
ScaleIntegrator::AssembleElementMatrix2(const mfem::FiniteElement & el1,
                                        const mfem::FiniteElement & el2,
                                        mfem::ElementTransformation & Trans,
                                        mfem::DenseMatrix & elmat)
{
  CheckIntegrator();
  _integrator->AssembleElementMatrix2(el1, el2, Trans, elmat);
  elmat *= _scale;
}

void
ScaleIntegrator::AssembleFaceMatrix(const mfem::FiniteElement & el1,
                                    const mfem::FiniteElement & el2,
                                    mfem::FaceElementTransformations & Trans,
                                    mfem::DenseMatrix & elmat)
{
  CheckIntegrator();
  _integrator->AssembleFaceMatrix(el1, el2, Trans, elmat);
  elmat *= _scale;
}

void
ScaleIntegrator::AssemblePA(const mfem::FiniteElementSpace & fes)
{
  CheckIntegrator();
  _integrator->AssemblePA(fes);
}

void
ScaleIntegrator::AssembleDiagonalPA(mfem::Vector & diag)
{
  _integrator->AssembleDiagonalPA(diag);
  diag *= _scale;
}

void
ScaleIntegrator::AssemblePAInteriorFaces(const mfem::FiniteElementSpace & fes)
{
  _integrator->AssemblePAInteriorFaces(fes);
}

void
ScaleIntegrator::AssemblePABoundaryFaces(const mfem::FiniteElementSpace & fes)
{
  _integrator->AssemblePABoundaryFaces(fes);
}

void
ScaleIntegrator::AddMultPA(const mfem::Vector & x, mfem::Vector & y) const
{
  // y += Mx*scale
  mfem::Vector Mx(y.Size());
  Mx = 0.0;
  _integrator->AddMultPA(x, Mx);
  Mx *= _scale;
  y += Mx;
}

void
ScaleIntegrator::AddMultTransposePA(const mfem::Vector & x, mfem::Vector & y) const
{
  // y += M^T x*scale
  mfem::Vector MTx(y.Size());
  MTx = 0.0;
  _integrator->AddMultTransposePA(x, MTx);
  MTx *= _scale;
  y += MTx;
}

void
ScaleIntegrator::AssembleEA(const mfem::FiniteElementSpace & fes,
                            mfem::Vector & emat,
                            const bool add)
{
  CheckIntegrator();
  if (add)
  {
    mfem::Vector emat_scale(emat.Size());
    _integrator->AssembleEA(fes, emat_scale, false);
    emat_scale *= _scale;
    emat += emat_scale;
  }
  else
  {
    _integrator->AssembleEA(fes, emat, add);
    emat *= _scale;
  }
}

ScaleIntegrator::~ScaleIntegrator()
{
  if (_own_integrator)
    delete _integrator;
}

} // namespace platypus