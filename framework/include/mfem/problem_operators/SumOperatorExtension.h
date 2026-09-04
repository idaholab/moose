/*
The other classes in this directory are inherting from mfem::Operator,
so I figure this is a good place to put it.
*/
#pragma once

#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"

namespace Moose::MFEM
{

// We have to do something hacky to get the constrained list out of the ConstrainedOperator.
// We need this for the stuff that comes in ConstrainedOperator::AssembleDiagonal, after
// calling AssembleDiagonal on the non/bi-linearform underneath.
//
// Revisiting this comment - the whole point of this is to avoid double-adding 1s to the
// diagonal of the finished operator, which ends up happening whenever there is a bilinear
// and a nonlinear form added to the same variable
class ConstrainedOperatorExtension : public mfem::ConstrainedOperator
{
public:
  void FinishAssembleDiagonal(mfem::Vector & diag) const
  {
    if (diag_policy == DIAG_KEEP)
    {
      return;
    }

    const int csz = constraint_list.Size();
    auto d_diag = diag.ReadWrite();
    auto idx = constraint_list.Read();

    // just do simple for loop
    mooseAssert(diag_policy == DIAG_ONE or diag_policy == DIAG_ZERO, "");

    for (int i = 0; i < csz; i++)
    {
      const int id = idx[i];
      d_diag[id] = (diag_policy == DIAG_ONE) ? 1.0 : 0.0;
    }
  }
};

// Since the member variables of SumOperator are private, we have
// to define our own class anyway...
class SumOperatorExtension : public mfem::Operator
{
  const mfem::Operator *A, *B;
  const mfem::real_t _alpha, _beta;
  mutable mfem::Vector z;
  mfem::ParNonlinearForm * _nlf; // not owned
public:
  SumOperatorExtension(const mfem::Operator * A,
                       const mfem::real_t alpha,
                       const mfem::Operator * B,
                       const mfem::real_t beta,
                       mfem::ParNonlinearForm * nlf)
    : Operator(A->Height(), A->Width()),
      A(A),
      B(B),
      _alpha(alpha),
      _beta(beta),
      z(A->Height()),
      _nlf(nlf)
  {
    mooseAssert(A->Width() == B->Width(), "Operator Widths must match");
    mooseAssert(A->Height() == B->Height(), "Operator Heights must match");

    // skipping check for if A or B casts into a mfem::Solver. They should
    // not be in iterative mode.
  }

  virtual ~SumOperatorExtension() {}

  void Mult(const mfem::Vector & x, mfem::Vector & y) const override
  {
    z.SetSize(A->Height());
    A->Mult(x, z);
    B->Mult(x, y);
    add(_alpha, z, _beta, y, y);
  }

  void MultTranspose(const mfem::Vector & x, mfem::Vector & y) const override
  {
    z.SetSize(A->Width());
    A->Mult(x, z);
    B->Mult(x, y);
    add(_alpha, z, _beta, y, y);
  }

  // This mostly copies the method taken by a BilinearForm/PABilinearFormExtension.
  // The only reason we need to do everything ourselves is because we need to call
  // _nlf->GetDNFI() instead of what usually happens (calling GetDBFI() on the
  // underlying bilinearform instead)
  void AssembleDiagOnNonlinearForm(mfem::Vector & diag) const
  {
    // the nlf will hold most things
    // firstly, the dnfi
    mfem::Array<mfem::NonlinearFormIntegrator *> & dnfi = *_nlf->GetDNFI();

    // next, the elemR. this should also unlock the size we need for ye
    mfem::FiniteElementSpace * fes = _nlf->FESpace();

    // this LEXICOGRAPHIC is correct. i check in gdb
    const mfem::Operator * elemR =
        fes->GetElementRestriction(mfem::ElementDofOrdering::LEXICOGRAPHIC);

    const int ye_size = elemR->Height(); // check this

    mfem::Vector ye(ye_size);
    ye = 0.0;

    for (int i = 0; i < dnfi.Size(); i++)
    {
      dnfi[i]->AssembleGradDiagonalPA(ye);
    }

    // finally, let's do it the way that the blf path does it
    const mfem::ElementRestriction * H1elem_restrict =
        dynamic_cast<const mfem::ElementRestriction *>(elemR);

    if (H1elem_restrict)
      H1elem_restrict->AbsMultTranspose(ye, diag);
    else
      elemR->MultTranspose(ye, diag);
  }

  void AssembleDiagonalOnConstrainedOperatorExtn(mfem::Vector & diag) const
  {
    // first, call AssembleDiagOnNonlinearForm
    AssembleDiagOnNonlinearForm(diag);

    // does A cast into ConstrainedOperator?
    const mfem::ConstrainedOperator * cA = dynamic_cast<const mfem::ConstrainedOperator *>(A);
    mooseAssert(cA, "");

    // Now, do all the rest of the stuff that happens in ConstrainedOperator::AssembleDiagonal
    // cast A into constrained operator
    const ConstrainedOperatorExtension * cA_extn =
        reinterpret_cast<const ConstrainedOperatorExtension *>(cA);
    mooseAssert(cA_extn, "");

    cA_extn->FinishAssembleDiagonal(diag);
  }

  void AssembleDiagonal(mfem::Vector & diag) const override
  {
    // slow and steady. Yes we could reuse the z vector here...
    mfem::Vector tempA(diag.Size());
    mfem::Vector tempB(diag.Size());

    // send temp A down
    // A->AssembleDiagonal(tempA);
    AssembleDiagonalOnConstrainedOperatorExtn(tempA);

    // ditto for B
    B->AssembleDiagonal(tempB);

    // mix them together
    add(_alpha, tempA, _beta, tempB, diag);
  }
};

} // namespace Moose::MFEM
