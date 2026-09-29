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
    add(_z, y, y);
  }

  void MultTranspose(const mfem::Vector & x, mfem::Vector & y) const override
  {
    _z.SetSize(_A->Width());
    _A->MultTranspose(x, _z);
    _B->MultTranspose(x, y);
    add(_z, y, y);
  }

  // This mostly copies the method taken by a BilinearForm/PABilinearFormExtension.
  // The only reason we need to do everything ourselves is because we need to call
  // _nlf->GetDNFI() instead of what usually happens (calling GetDBFI() on the
  // underlying bilinearform instead)
  void AssembleDiagOnNonlinearForm(mfem::Vector & diag) const
  {
    const mfem::Operator * elemR =
        _nlf->FESpace()->GetElementRestriction(mfem::ElementDofOrdering::LEXICOGRAPHIC);
    mfem::Vector ye(elemR->Height());
    ye = 0.0;
    // assemble grad diag on each of the domain integrators
    for (auto & dnfi : *_nlf->GetDNFI())
      dnfi->AssembleGradDiagonalPA(ye);
    // ditto for the boundary integrators
    for (auto & bnfi : *_nlf->GetBNFI())
      bnfi->AssembleGradDiagonalPA(ye);

    // The element restriction transposes onto local dofs, instead of true dofs. So if we are
    // running on multiple ranks we're likely to get that elemR->Width() is the size of the
    // locals dofs, and in general larger than diag.Size(), which is the size of the true dofs.
    // So, when we run with multiple ranks, we may have to prolongate before we do the final
    // element restriction.
    mfem::Vector local_diag(elemR->Width());

    // ElementRestriction applies orientation sign flips, which H(curl) and H(div) spaces carry
    // on shared DoFs. A diagonal needs those signs squared, so the unsigned transpose is the
    // correct one here; restrictions without signs need no such correction.
    const mfem::ElementRestriction * const signed_restriction =
        dynamic_cast<const mfem::ElementRestriction *>(elemR);

    if (signed_restriction)
      signed_restriction->AbsMultTranspose(ye, local_diag);
    else
      elemR->MultTranspose(ye, local_diag);

    const mfem::Operator * P = _nlf->FESpace()->GetProlongationMatrix();
    if (!P || mfem::IsIdentityProlongation(P))
      diag = local_diag;

    else if (_nlf->FESpace()->Conforming())
      P->MultTranspose(local_diag, diag);

    else
      mooseError("Nonconforming mesh unsupported!");
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
    nlf_diag.SetSubVector(_nlf->GetEssentialTrueDofs(), 0.);

    mfem::Vector b_diag(diag.Size());
    _B->AssembleDiagonal(b_diag);

    add(nlf_diag, b_diag, diag);
  }

private:
  const mfem::Operator *_A, *_B;
  mutable mfem::Vector _z;
  mfem::ParNonlinearForm * _nlf; // not owned
};

} // namespace Moose::MFEM
