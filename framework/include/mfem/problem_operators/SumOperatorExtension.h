//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"
#include "MooseError.h"

namespace Moose::MFEM
{

// This class is necessary since a lot of the member variables
// of mfem::SumOperator are private.
// It is essential that the first operator is the gradient of the
// nonlinear form here, as we call B->AssembleDiagonal later on,
// and this will fail for the gradient of the nlf.
class SumOperatorExtension : public mfem::Operator
{
public:
  SumOperatorExtension(const mfem::Operator * A,
                       const mfem::Operator * B,
                       mfem::ParNonlinearForm * nlf)
    : Operator(A->Height(), A->Width()), _A(A), _B(B), _z(A->Height()), _nlf(nlf)
  {
    mooseAssert(A->Width() == B->Width(), "Operator Widths must match");
    mooseAssert(A->Height() == B->Height(), "Operator Heights must match");

    // skipping check for if A or B casts into a mfem::Solver. They should
    // not be in iterative mode.
  }

  ~SumOperatorExtension() override = default;

  void Mult(const mfem::Vector & x, mfem::Vector & y) const override
  {
    _z.SetSize(_A->Height());
    _A->Mult(x, _z);
    _B->Mult(x, y);
    add(_alpha, _z, _beta, y, y);
  }

  void MultTranspose(const mfem::Vector & x, mfem::Vector & y) const override
  {
    _z.SetSize(_A->Width());
    _A->MultTranspose(x, _z);
    _B->MultTranspose(x, y);
    add(_alpha, _z, _beta, y, y);
  }

  // This mostly copies the method taken by a BilinearForm/PABilinearFormExtension.
  // The only reason we need to do everything ourselves is because we need to call
  // _nlf->GetDNFI() instead of what usually happens (calling GetDBFI() on the
  // underlying bilinearform instead)
  void AssembleDiagOnNonlinearForm(mfem::Vector & diag) const
  {
    // firstly, the dnfi
    mfem::Array<mfem::NonlinearFormIntegrator *> & dnfi = *_nlf->GetDNFI();
    mfem::Array<mfem::NonlinearFormIntegrator *> & bnfi = *_nlf->GetBNFI();

    mfem::FiniteElementSpace * fes = _nlf->FESpace();

    const mfem::Operator * elemR =
        fes->GetElementRestriction(mfem::ElementDofOrdering::LEXICOGRAPHIC);

    const int ye_size = elemR->Height();

    mfem::Vector ye(ye_size);
    ye = 0.0;

    // assemble grad diag on each of the domain integrators
    for (int i = 0; i < dnfi.Size(); i++)
    {
      dnfi[i]->AssembleGradDiagonalPA(ye);
    }

    // ditto for the boundary integrators
    for (int i = 0; i < bnfi.Size(); i++)
    {
      bnfi[i]->AssembleGradDiagonalPA(ye);
    }

    // ElementRestriction applies orientation sign flips, which H(curl) and H(div) spaces carry
    // on shared DoFs. A diagonal needs those signs squared, so the unsigned transpose is the
    // correct one here; restrictions without signs need no such correction.
    const mfem::ElementRestriction * const signed_restriction =
        dynamic_cast<const mfem::ElementRestriction *>(elemR);

    if (signed_restriction)
      signed_restriction->AbsMultTranspose(ye, diag);
    else
      elemR->MultTranspose(ye, diag);
  }

  void AssembleDiagonal(mfem::Vector & diag) const override
  {
    mfem::Vector nlf_diag(diag.Size());
    AssembleDiagOnNonlinearForm(nlf_diag);

    // Ultimately, we want the diag that we assemble here to have 1s
    // on all essential rows. Since we combine diagonal values from
    // the nonlinear form and the bilinear form here, we choose to
    // make sure the nonlinear form has a diag_policy of 0 (see the
    // for loop) and the bilinear form has a diag_policy
    // of 1 (default). This means when we combine at the end, we have 1s
    // on essential rows.
    const mfem::Array<int> & ess_tdofs = _nlf->GetEssentialTrueDofs();
    const int csz = ess_tdofs.Size();
    auto idx = ess_tdofs.HostRead();
    auto d_diag = nlf_diag.HostReadWrite();
    for (int i = 0; i < csz; i++)
      d_diag[idx[i]] = 0.0;

    // ditto for B
    mfem::Vector b_diag(diag.Size());
    _B->AssembleDiagonal(b_diag);

    add(_alpha, nlf_diag, _beta, b_diag, diag);
  }

private:
  const mfem::Operator *_A, *_B;
  const mfem::real_t _alpha = 1.0, _beta = 1.0;
  mutable mfem::Vector _z;
  mfem::ParNonlinearForm * _nlf; // not owned
};

} // namespace Moose::MFEM
