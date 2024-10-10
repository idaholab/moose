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
  CheckIntegrator();
  _integrator->AssemblePA(fes);
}

void ScaleIntegrator::AssembleDiagonalPA(mfem::Vector &diag)
{
  _integrator->AssembleDiagonalPA(diag);
  diag *= _scale;
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
  // y += Mx*scale
  mfem::Vector Mx(y.Size());
  Mx = 0.0;
  _integrator->AddMultPA(x, Mx);
  Mx *= _scale;
  y += Mx;
}

void ScaleIntegrator::AddMultTransposePA(const mfem::Vector &x, mfem::Vector &y) const
{
  // y += M^T x*scale
  mfem::Vector MTx(y.Size());
  MTx = 0.0;
  _integrator->AddMultTransposePA(x, MTx);
  MTx *= _scale;
  y += MTx;
}

void ScaleIntegrator::AssembleMF(const mfem::FiniteElementSpace &fes)
{
  _integrator->AssembleMF(fes);
  std::cout << "AssembleMF" << std::endl;

}

void ScaleIntegrator::AddMultMF(const mfem::Vector& x, mfem::Vector& y) const
{
  _integrator->AddMultTransposeMF(x, y);
  std::cout << "AddMultTransposeMF" << std::endl;

}

void ScaleIntegrator::AddMultTransposeMF(const mfem::Vector &x, mfem::Vector &y) const
{
  _integrator->AddMultMF(x, y);
  std::cout << "AddMultTransposeMF" << std::endl;

}

void ScaleIntegrator::AssembleDiagonalMF(mfem::Vector &diag)
{
  _integrator->AssembleDiagonalMF(diag);
  std::cout << "AssembleDiagonalMF" << std::endl;
}

void ScaleIntegrator::AssembleEA(const mfem::FiniteElementSpace &fes, mfem::Vector &emat,
                               const bool add)
{
  _integrator->AssembleEA(fes, emat, add);
  std::cout << "AssembleEA" << std::endl;

}

void ScaleIntegrator::AssembleEAInteriorFaces(const mfem::FiniteElementSpace &fes,
                                            mfem::Vector &ea_data_int,
                                            mfem::Vector &ea_data_ext,
                                            const bool add)
{
  _integrator->AssembleEAInteriorFaces(fes,ea_data_int,ea_data_ext,add);
  std::cout << "AssembleEAInteriorFaces" << std::endl;

}

void ScaleIntegrator::AssembleEABoundaryFaces(const mfem::FiniteElementSpace &fes,
                                            mfem::Vector &ea_data_bdr,
                                            const bool add)
{
  _integrator->AssembleEABoundaryFaces(fes, ea_data_bdr, add);
  std::cout << "AssembleEABoundaryFaces" << std::endl;

}

ScaleIntegrator::~ScaleIntegrator()
{
  if (_own_integrator)
    delete _integrator;
}