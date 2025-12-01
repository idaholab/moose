#ifdef MOOSE_MFEM_ENABLED

#include "ParMixedSesquilinearForm.h"

bool ParMixedSesquilinearForm::RealInteg()
{
   int nint = _pmblfr->GetFBFI()->Size() + _pmblfr->GetDBFI()->Size() +
              _pmblfr->GetBBFI()->Size() + _pmblfr->GetBFBFI()->Size();
   return (nint != 0);
}

bool ParMixedSesquilinearForm::ImagInteg()
{
   int nint = _pmblfi->GetFBFI()->Size() + _pmblfi->GetDBFI()->Size() +
              _pmblfi->GetBBFI()->Size() + _pmblfi->GetBFBFI()->Size();
   return (nint != 0);
}

ParMixedSesquilinearForm::ParMixedSesquilinearForm(mfem::ParFiniteElementSpace *trial_fes, mfem::ParFiniteElementSpace *test_fes,
                                         mfem::ComplexOperator::Convention
                                         convention)
   : _conv(convention),
     _pmblfr(new mfem::ParMixedBilinearForm(trial_fes, test_fes)),
     _pmblfi(new mfem::ParMixedBilinearForm(trial_fes, test_fes))
{}

ParMixedSesquilinearForm::ParMixedSesquilinearForm(mfem::ParFiniteElementSpace *trial_fes, mfem::ParFiniteElementSpace *test_fes,
                                         mfem::ParMixedBilinearForm *pbfr,
                                         mfem::ParMixedBilinearForm *pbfi,
                                         mfem::ComplexOperator::Convention convention)
   : _conv(convention),
     _pmblfr(new mfem::ParMixedBilinearForm(trial_fes, test_fes, pbfr)),
     _pmblfi(new mfem::ParMixedBilinearForm(trial_fes, test_fes, pbfi))
{}

ParMixedSesquilinearForm::~ParMixedSesquilinearForm()
{
   delete _pmblfr;
   delete _pmblfi;
}

void ParMixedSesquilinearForm::AddDomainIntegrator(mfem::BilinearFormIntegrator *bfi_real,
                                              mfem::BilinearFormIntegrator *bfi_imag)
{
   if (bfi_real) { _pmblfr->AddDomainIntegrator(bfi_real); }
   if (bfi_imag) { _pmblfi->AddDomainIntegrator(bfi_imag); }
}

void ParMixedSesquilinearForm::AddDomainIntegrator(mfem::BilinearFormIntegrator *bfi_real,
                                              mfem::BilinearFormIntegrator *bfi_imag,
                                              mfem::Array<int> & elem_marker)
{
   if (bfi_real) { _pmblfr->AddDomainIntegrator(bfi_real, elem_marker); }
   if (bfi_imag) { _pmblfi->AddDomainIntegrator(bfi_imag, elem_marker); }
}

void
ParMixedSesquilinearForm::AddBoundaryIntegrator(mfem::BilinearFormIntegrator *bfi_real,
                                           mfem::BilinearFormIntegrator *bfi_imag)
{
   if (bfi_real) { _pmblfr->AddBoundaryIntegrator(bfi_real); }
   if (bfi_imag) { _pmblfi->AddBoundaryIntegrator(bfi_imag); }
}

void
ParMixedSesquilinearForm::AddBoundaryIntegrator(mfem::BilinearFormIntegrator *bfi_real,
                                           mfem::BilinearFormIntegrator *bfi_imag,
                                           mfem::Array<int> & bdr_marker)
{
   if (bfi_real) { _pmblfr->AddBoundaryIntegrator(bfi_real, bdr_marker); }
   if (bfi_imag) { _pmblfi->AddBoundaryIntegrator(bfi_imag, bdr_marker); }
}

void
ParMixedSesquilinearForm::AddInteriorFaceIntegrator(mfem::BilinearFormIntegrator *bfi_real,
                                               mfem::BilinearFormIntegrator *bfi_imag)
{
   if (bfi_real) { _pmblfr->AddInteriorFaceIntegrator(bfi_real); }
   if (bfi_imag) { _pmblfi->AddInteriorFaceIntegrator(bfi_imag); }
}

void
ParMixedSesquilinearForm::AddBdrFaceIntegrator(mfem::BilinearFormIntegrator *bfi_real,
                                          mfem::BilinearFormIntegrator *bfi_imag)
{
   if (bfi_real) { _pmblfr->AddBdrFaceIntegrator(bfi_real); }
   if (bfi_imag) { _pmblfi->AddBdrFaceIntegrator(bfi_imag); }
}

void
ParMixedSesquilinearForm::AddBdrFaceIntegrator(mfem::BilinearFormIntegrator *bfi_real,
                                          mfem::BilinearFormIntegrator *bfi_imag,
                                          mfem::Array<int> &bdr_marker)
{
   if (bfi_real) { _pmblfr->AddBdrFaceIntegrator(bfi_real, bdr_marker); }
   if (bfi_imag) { _pmblfi->AddBdrFaceIntegrator(bfi_imag, bdr_marker); }
}

void
ParMixedSesquilinearForm::Assemble(int skip_zeros)
{
   _pmblfr->Assemble(skip_zeros);
   _pmblfi->Assemble(skip_zeros);
}

void
ParMixedSesquilinearForm::Finalize(int skip_zeros)
{
   _pmblfr->Finalize(skip_zeros);
   _pmblfi->Finalize(skip_zeros);
}

mfem::ComplexHypreParMatrix *
ParMixedSesquilinearForm::ParallelAssemble()
{
   return new mfem::ComplexHypreParMatrix(_pmblfr->ParallelAssemble(),
                                    _pmblfi->ParallelAssemble(),
                                    true, true, _conv);
}

void
ParMixedSesquilinearForm::FormRectangularLinearSystem(const mfem::Array<int> &ess_trial_tdof_list, const mfem::Array<int> &ess_test_tdof_list,
                                      mfem::Vector &x, mfem::Vector &b,
                                      mfem::OperatorHandle &A,
                                      mfem::Vector &X, mfem::Vector &B,
                                      int ci)
{
//   mfem::ParFiniteElementSpace *pfes = _pmblfr->ParFESpace();
//   const int vsize = pfes->GetVSize();
//
//   // Allocate temporary mfem::vector
//   mfem::Vector b_0;
//   b_0.UseDevice(true);
//   b_0.SetSize(vsize);
//   b_0 = 0.0;
//
//   // Extract the real and imaginary parts of the input mfem::vectors
//   MFEM_ASSERT(x.Size() == 2 * vsize, "Input GridFunction of incorrect size!");
//   x.Read();
//   mfem::Vector x_r; x_r.MakeRef(x, 0, vsize);
//   mfem::Vector x_i; x_i.MakeRef(x, vsize, vsize);
//
//   MFEM_ASSERT(b.Size() == 2 * vsize, "Input LinearForm of incorrect size!");
//   b.Read();
//   mfem::Vector b_r; b_r.MakeRef(b, 0, vsize);
//   mfem::Vector b_i; b_i.MakeRef(b, vsize, vsize);
//
//   if (_conv == mfem::ComplexOperator::BLOCK_SYMMETRIC) { b_i *= -1.0; }
//
//   const int tvsize = pfes->GetTrueVSize();
//   mfem::OperatorHandle A_r, A_i;
//
//   X.UseDevice(true);
//   X.SetSize(2 * tvsize);
//   X = 0.0;
//
//   B.UseDevice(true);
//   B.SetSize(2 * tvsize);
//   B = 0.0;
//
//   mfem::Vector X_r; X_r.MakeRef(X, 0, tvsize);
//   mfem::Vector X_i; X_i.MakeRef(X, tvsize, tvsize);
//   mfem::Vector B_r; B_r.MakeRef(B, 0, tvsize);
//   mfem::Vector B_i; B_i.MakeRef(B, tvsize, tvsize);
//
//   mfem::Vector X_0, B_0;
//
//   if (RealInteg())
//   {
//      b_0 = b_r;
//      _pmblfr->FormLinearSystem(ess_tdof_list, x_r, b_0, A_r, X_0, B_0, ci);
//      X_r = X_0; B_r = B_0;
//
//      b_0 = b_i;
//      _pmblfr->FormLinearSystem(ess_tdof_list, x_i, b_0, A_r, X_0, B_0, ci);
//      X_i = X_0; B_i = B_0;
//
//      if (ImagInteg())
//      {
//         b_0 = 0.0;
//         _pmblfi->FormLinearSystem(ess_tdof_list, x_i, b_0, A_i, X_0, B_0, false);
//         B_r -= B_0;
//
//         b_0 = 0.0;
//         _pmblfi->FormLinearSystem(ess_tdof_list, x_r, b_0, A_i, X_0, B_0, false);
//         B_i += B_0;
//      }
//   }
//   else if (ImagInteg())
//   {
//      b_0 = b_i;
//      _pmblfi->FormLinearSystem(ess_tdof_list, x_r, b_0, A_i, X_0, B_0, ci);
//      X_r = X_0; B_i = B_0;
//
//      b_0 = b_r; b_0 *= -1.0;
//      _pmblfi->FormLinearSystem(ess_tdof_list, x_i, b_0, A_i, X_0, B_0, ci);
//      X_i = X_0; B_r = B_0; B_r *= -1.0;
//   }
//   else
//   {
//      MFEM_ABORT("Real and Imaginary part of the Sesquilinear form are empty");
//   }
//
//   if (RealInteg() && ImagInteg())
//   {
//      // Modify RHS to conform with standard essential BC treatment
//      const int n = ess_tdof_list.Size();
//      auto d_B_r = B_r.Write();
//      auto d_B_i = B_i.Write();
//      auto d_X_r = X_r.Read();
//      auto d_X_i = X_i.Read();
//      auto d_idx = ess_tdof_list.Read();
//      mfem::forall(n, [=] MFEM_HOST_DEVICE (int i)
//      {
//         const int j = d_idx[i];
//         d_B_r[j] = d_X_r[j];
//         d_B_i[j] = d_X_i[j];
//      });
//      // Modify off-diagonal blocks (imaginary parts of the matrix) to conform
//      // with standard essential BC treatment
//      if (A_i.Type() == mfem::Operator::Hypre_ParCSR)
//      {
//         mfem::HypreParMatrix * Ah;
//         A_i.Get(Ah);
//         hypre_ParCSRMatrix *Aih = *Ah;
//         Ah->HypreReadWrite();
//         const int *d_ess_tdof_list =
//            ess_tdof_list.GetMemory().Read(mfem::GetHypreForallMemoryClass(), n);
//         HYPRE_Int *d_diag_i = Aih->diag->i;
//         mfem::real_t *d_diag_data = Aih->diag->data;
//         mfem::hypre_forall(n, [=] MFEM_HOST_DEVICE (int k)
//         {
//            const int j = d_ess_tdof_list[k];
//            d_diag_data[d_diag_i[j]] = 0.0;
//         });
//      }
//      else
//      {
//         A_i.As<mfem::ConstrainedOperator>()->SetDiagonalPolicy
//         (mfem::Operator::DiagonalPolicy::DIAG_ZERO);
//      }
//   }
//
//   if (_conv == mfem::ComplexOperator::BLOCK_SYMMETRIC)
//   {
//      B_i *= -1.0;
//      b_i *= -1.0;
//   }
//
//   x_r.SyncAliasMemory(x);
//   x_i.SyncAliasMemory(x);
//   b_r.SyncAliasMemory(b);
//   b_i.SyncAliasMemory(b);
//
//   X_r.SyncAliasMemory(X);
//   X_i.SyncAliasMemory(X);
//   B_r.SyncAliasMemory(B);
//   B_i.SyncAliasMemory(B);
//
//   // A = A_r + i A_i
//   A.Clear();
//   if ( A_r.Type() == mfem::Operator::Hypre_ParCSR ||
//        A_i.Type() == mfem::Operator::Hypre_ParCSR )
//   {
//      mfem::ComplexHypreParMatrix * A_hyp =
//         new mfem::ComplexHypreParMatrix(A_r.As<mfem::HypreParMatrix>(),
//                                   A_i.As<mfem::HypreParMatrix>(),
//                                   A_r.OwnsOperator(),
//                                   A_i.OwnsOperator(),
//                                   _conv);
//      A.Reset<mfem::ComplexHypreParMatrix>(A_hyp, true);
//   }
//   else
//   {
//      mfem::ComplexOperator * A_op =
//         new mfem::ComplexOperator(A_r.As<mfem::Operator>(),
//                             A_i.As<mfem::Operator>(),
//                             A_r.OwnsOperator(),
//                             A_i.OwnsOperator(),
//                             _conv);
//      A.Reset<mfem::ComplexOperator>(A_op, true);
//   }
//   A_r.SetOperatorOwner(false);
//   A_i.SetOperatorOwner(false);
}

void
ParMixedSesquilinearForm::FormRectangularSystemMatrix(const mfem::Array<int> &ess_trial_tdof_list, const mfem::Array<int> &ess_test_tdof_list,
                                      mfem::OperatorHandle &A)
{
   mfem::OperatorHandle A_r, A_i;
   if (RealInteg())
   {
      _pmblfr->FormRectangularSystemMatrix(ess_trial_tdof_list, ess_test_tdof_list, A_r);
   }
   if (ImagInteg())
   {
      _pmblfi->FormRectangularSystemMatrix(ess_trial_tdof_list, ess_test_tdof_list, A_i);
   }
   if (!RealInteg() && !ImagInteg())
   {
      MFEM_ABORT("Both Real and Imaginary part of the Mixed Sesquilinear form are empty");
   }
   
   //if (RealInteg() && ImagInteg())
   //{
   //   // Modify off-diagonal blocks (imaginary parts of the matrix) to conform
   //   // with standard essential BC treatment
   //   if ( A_i.Type() == mfem::Operator::Hypre_ParCSR )
   //   {
   //      int n_trial = ess_tdof_list.Size();
   //      mfem::HypreParMatrix * Ah;
   //      A_i.Get(Ah);
   //      hypre_ParCSRMatrix * Aih = *Ah;
   //      for (int k = 0; k < n; k++)
   //      {
   //         int j = ess_tdof_list[k];
   //         Aih->diag->data[Aih->diag->i[j]] = 0.0;
   //      }
   //   }
   //   else
   //   {
   //      A_i.As<mfem::ConstrainedOperator>()->SetDiagonalPolicy
   //      (mfem::Operator::DiagonalPolicy::DIAG_ZERO);
   //   }
   //}

   // A = A_r + i A_i
   A.Clear();
   if ( A_r.Type() == mfem::Operator::Hypre_ParCSR ||
        A_i.Type() == mfem::Operator::Hypre_ParCSR )
   {
      mfem::ComplexHypreParMatrix * A_hyp =
         new mfem::ComplexHypreParMatrix(A_r.As<mfem::HypreParMatrix>(),
                                   A_i.As<mfem::HypreParMatrix>(),
                                   A_r.OwnsOperator(),
                                   A_i.OwnsOperator(),
                                   _conv);
      A.Reset<mfem::ComplexHypreParMatrix>(A_hyp, true);
   }
   else
   {
      mfem::ComplexOperator * A_op =
         new mfem::ComplexOperator(A_r.As<mfem::Operator>(),
                             A_i.As<mfem::Operator>(),
                             A_r.OwnsOperator(),
                             A_i.OwnsOperator(),
                             _conv);
      A.Reset<mfem::ComplexOperator>(A_op, true);
   }
   A_r.SetOperatorOwner(false);
   A_i.SetOperatorOwner(false);
}



#endif