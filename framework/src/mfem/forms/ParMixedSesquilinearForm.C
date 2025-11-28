#ifdef MOOSE_MFEM_ENABLED

#include "ParMixedSesquilinearForm.h"

bool ParMixedSesquilinearForm::RealInteg()
{
   int nint = pblfr->GetFBFI()->Size() + pblfr->GetDBFI()->Size() +
              pblfr->GetBBFI()->Size() + pblfr->GetBFBFI()->Size();
   return (nint != 0);
}

bool ParMixedSesquilinearForm::ImagInteg()
{
   int nint = pblfi->GetFBFI()->Size() + pblfi->GetDBFI()->Size() +
              pblfi->GetBBFI()->Size() + pblfi->GetBFBFI()->Size();
   return (nint != 0);
}

ParMixedSesquilinearForm::ParMixedSesquilinearForm(ParFiniteElementSpace *pf,
                                         ComplexOperator::Convention
                                         convention)
   : conv(convention),
     pblfr(new ParBilinearForm(pf)),
     pblfi(new ParBilinearForm(pf))
{}

ParMixedSesquilinearForm::ParMixedSesquilinearForm(ParFiniteElementSpace *pf,
                                         ParBilinearForm *pbfr,
                                         ParBilinearForm *pbfi,
                                         ComplexOperator::Convention convention)
   : conv(convention),
     pblfr(new ParBilinearForm(pf,pbfr)),
     pblfi(new ParBilinearForm(pf,pbfi))
{}

ParMixedSesquilinearForm::~ParMixedSesquilinearForm()
{
   delete pblfr;
   delete pblfi;
}

void ParMixedSesquilinearForm::AddDomainIntegrator(BilinearFormIntegrator *bfi_real,
                                              BilinearFormIntegrator *bfi_imag)
{
   if (bfi_real) { pblfr->AddDomainIntegrator(bfi_real); }
   if (bfi_imag) { pblfi->AddDomainIntegrator(bfi_imag); }
}

void ParMixedSesquilinearForm::AddDomainIntegrator(BilinearFormIntegrator *bfi_real,
                                              BilinearFormIntegrator *bfi_imag,
                                              Array<int> & elem_marker)
{
   if (bfi_real) { pblfr->AddDomainIntegrator(bfi_real, elem_marker); }
   if (bfi_imag) { pblfi->AddDomainIntegrator(bfi_imag, elem_marker); }
}

void
ParMixedSesquilinearForm::AddBoundaryIntegrator(BilinearFormIntegrator *bfi_real,
                                           BilinearFormIntegrator *bfi_imag)
{
   if (bfi_real) { pblfr->AddBoundaryIntegrator(bfi_real); }
   if (bfi_imag) { pblfi->AddBoundaryIntegrator(bfi_imag); }
}

void
ParMixedSesquilinearForm::AddBoundaryIntegrator(BilinearFormIntegrator *bfi_real,
                                           BilinearFormIntegrator *bfi_imag,
                                           Array<int> & bdr_marker)
{
   if (bfi_real) { pblfr->AddBoundaryIntegrator(bfi_real, bdr_marker); }
   if (bfi_imag) { pblfi->AddBoundaryIntegrator(bfi_imag, bdr_marker); }
}

void
ParMixedSesquilinearForm::AddInteriorFaceIntegrator(BilinearFormIntegrator *bfi_real,
                                               BilinearFormIntegrator *bfi_imag)
{
   if (bfi_real) { pblfr->AddInteriorFaceIntegrator(bfi_real); }
   if (bfi_imag) { pblfi->AddInteriorFaceIntegrator(bfi_imag); }
}

void
ParMixedSesquilinearForm::AddBdrFaceIntegrator(BilinearFormIntegrator *bfi_real,
                                          BilinearFormIntegrator *bfi_imag)
{
   if (bfi_real) { pblfr->AddBdrFaceIntegrator(bfi_real); }
   if (bfi_imag) { pblfi->AddBdrFaceIntegrator(bfi_imag); }
}

void
ParMixedSesquilinearForm::AddBdrFaceIntegrator(BilinearFormIntegrator *bfi_real,
                                          BilinearFormIntegrator *bfi_imag,
                                          Array<int> &bdr_marker)
{
   if (bfi_real) { pblfr->AddBdrFaceIntegrator(bfi_real, bdr_marker); }
   if (bfi_imag) { pblfi->AddBdrFaceIntegrator(bfi_imag, bdr_marker); }
}

void
ParMixedSesquilinearForm::Assemble(int skip_zeros)
{
   pblfr->Assemble(skip_zeros);
   pblfi->Assemble(skip_zeros);
}

void
ParMixedSesquilinearForm::Finalize(int skip_zeros)
{
   pblfr->Finalize(skip_zeros);
   pblfi->Finalize(skip_zeros);
}

ComplexHypreParMatrix *
ParMixedSesquilinearForm::ParallelAssemble()
{
   return new ComplexHypreParMatrix(pblfr->ParallelAssemble(),
                                    pblfi->ParallelAssemble(),
                                    true, true, conv);
}

void
ParMixedSesquilinearForm::FormLinearSystem(const Array<int> &ess_tdof_list,
                                      Vector &x, Vector &b,
                                      OperatorHandle &A,
                                      Vector &X, Vector &B,
                                      int ci)
{
   ParFiniteElementSpace *pfes = pblfr->ParFESpace();
   const int vsize = pfes->GetVSize();

   // Allocate temporary vector
   Vector b_0;
   b_0.UseDevice(true);
   b_0.SetSize(vsize);
   b_0 = 0.0;

   // Extract the real and imaginary parts of the input vectors
   MFEM_ASSERT(x.Size() == 2 * vsize, "Input GridFunction of incorrect size!");
   x.Read();
   Vector x_r; x_r.MakeRef(x, 0, vsize);
   Vector x_i; x_i.MakeRef(x, vsize, vsize);

   MFEM_ASSERT(b.Size() == 2 * vsize, "Input LinearForm of incorrect size!");
   b.Read();
   Vector b_r; b_r.MakeRef(b, 0, vsize);
   Vector b_i; b_i.MakeRef(b, vsize, vsize);

   if (conv == ComplexOperator::BLOCK_SYMMETRIC) { b_i *= -1.0; }

   const int tvsize = pfes->GetTrueVSize();
   OperatorHandle A_r, A_i;

   X.UseDevice(true);
   X.SetSize(2 * tvsize);
   X = 0.0;

   B.UseDevice(true);
   B.SetSize(2 * tvsize);
   B = 0.0;

   Vector X_r; X_r.MakeRef(X, 0, tvsize);
   Vector X_i; X_i.MakeRef(X, tvsize, tvsize);
   Vector B_r; B_r.MakeRef(B, 0, tvsize);
   Vector B_i; B_i.MakeRef(B, tvsize, tvsize);

   Vector X_0, B_0;

   if (RealInteg())
   {
      b_0 = b_r;
      pblfr->FormLinearSystem(ess_tdof_list, x_r, b_0, A_r, X_0, B_0, ci);
      X_r = X_0; B_r = B_0;

      b_0 = b_i;
      pblfr->FormLinearSystem(ess_tdof_list, x_i, b_0, A_r, X_0, B_0, ci);
      X_i = X_0; B_i = B_0;

      if (ImagInteg())
      {
         b_0 = 0.0;
         pblfi->FormLinearSystem(ess_tdof_list, x_i, b_0, A_i, X_0, B_0, false);
         B_r -= B_0;

         b_0 = 0.0;
         pblfi->FormLinearSystem(ess_tdof_list, x_r, b_0, A_i, X_0, B_0, false);
         B_i += B_0;
      }
   }
   else if (ImagInteg())
   {
      b_0 = b_i;
      pblfi->FormLinearSystem(ess_tdof_list, x_r, b_0, A_i, X_0, B_0, ci);
      X_r = X_0; B_i = B_0;

      b_0 = b_r; b_0 *= -1.0;
      pblfi->FormLinearSystem(ess_tdof_list, x_i, b_0, A_i, X_0, B_0, ci);
      X_i = X_0; B_r = B_0; B_r *= -1.0;
   }
   else
   {
      MFEM_ABORT("Real and Imaginary part of the Sesquilinear form are empty");
   }

   if (RealInteg() && ImagInteg())
   {
      // Modify RHS to conform with standard essential BC treatment
      const int n = ess_tdof_list.Size();
      auto d_B_r = B_r.Write();
      auto d_B_i = B_i.Write();
      auto d_X_r = X_r.Read();
      auto d_X_i = X_i.Read();
      auto d_idx = ess_tdof_list.Read();
      mfem::forall(n, [=] MFEM_HOST_DEVICE (int i)
      {
         const int j = d_idx[i];
         d_B_r[j] = d_X_r[j];
         d_B_i[j] = d_X_i[j];
      });
      // Modify off-diagonal blocks (imaginary parts of the matrix) to conform
      // with standard essential BC treatment
      if (A_i.Type() == Operator::Hypre_ParCSR)
      {
         HypreParMatrix * Ah;
         A_i.Get(Ah);
         hypre_ParCSRMatrix *Aih = *Ah;
         Ah->HypreReadWrite();
         const int *d_ess_tdof_list =
            ess_tdof_list.GetMemory().Read(GetHypreForallMemoryClass(), n);
         HYPRE_Int *d_diag_i = Aih->diag->i;
         real_t *d_diag_data = Aih->diag->data;
         mfem::hypre_forall(n, [=] MFEM_HOST_DEVICE (int k)
         {
            const int j = d_ess_tdof_list[k];
            d_diag_data[d_diag_i[j]] = 0.0;
         });
      }
      else
      {
         A_i.As<ConstrainedOperator>()->SetDiagonalPolicy
         (mfem::Operator::DiagonalPolicy::DIAG_ZERO);
      }
   }

   if (conv == ComplexOperator::BLOCK_SYMMETRIC)
   {
      B_i *= -1.0;
      b_i *= -1.0;
   }

   x_r.SyncAliasMemory(x);
   x_i.SyncAliasMemory(x);
   b_r.SyncAliasMemory(b);
   b_i.SyncAliasMemory(b);

   X_r.SyncAliasMemory(X);
   X_i.SyncAliasMemory(X);
   B_r.SyncAliasMemory(B);
   B_i.SyncAliasMemory(B);

   // A = A_r + i A_i
   A.Clear();
   if ( A_r.Type() == Operator::Hypre_ParCSR ||
        A_i.Type() == Operator::Hypre_ParCSR )
   {
      ComplexHypreParMatrix * A_hyp =
         new ComplexHypreParMatrix(A_r.As<HypreParMatrix>(),
                                   A_i.As<HypreParMatrix>(),
                                   A_r.OwnsOperator(),
                                   A_i.OwnsOperator(),
                                   conv);
      A.Reset<ComplexHypreParMatrix>(A_hyp, true);
   }
   else
   {
      ComplexOperator * A_op =
         new ComplexOperator(A_r.As<Operator>(),
                             A_i.As<Operator>(),
                             A_r.OwnsOperator(),
                             A_i.OwnsOperator(),
                             conv);
      A.Reset<ComplexOperator>(A_op, true);
   }
   A_r.SetOperatorOwner(false);
   A_i.SetOperatorOwner(false);
}

void
ParMixedSesquilinearForm::FormSystemMatrix(const Array<int> &ess_tdof_list,
                                      OperatorHandle &A)
{
   OperatorHandle A_r, A_i;
   if (RealInteg())
   {
      pblfr->FormSystemMatrix(ess_tdof_list, A_r);
   }
   if (ImagInteg())
   {
      pblfi->FormSystemMatrix(ess_tdof_list, A_i);
   }
   if (!RealInteg() && !ImagInteg())
   {
      MFEM_ABORT("Both Real and Imaginary part of the Sesquilinear form are empty");
   }

   if (RealInteg() && ImagInteg())
   {
      // Modify off-diagonal blocks (imaginary parts of the matrix) to conform
      // with standard essential BC treatment
      if ( A_i.Type() == Operator::Hypre_ParCSR )
      {
         int n = ess_tdof_list.Size();
         HypreParMatrix * Ah;
         A_i.Get(Ah);
         hypre_ParCSRMatrix * Aih = *Ah;
         for (int k = 0; k < n; k++)
         {
            int j = ess_tdof_list[k];
            Aih->diag->data[Aih->diag->i[j]] = 0.0;
         }
      }
      else
      {
         A_i.As<ConstrainedOperator>()->SetDiagonalPolicy
         (mfem::Operator::DiagonalPolicy::DIAG_ZERO);
      }
   }

   // A = A_r + i A_i
   A.Clear();
   if ( A_r.Type() == Operator::Hypre_ParCSR ||
        A_i.Type() == Operator::Hypre_ParCSR )
   {
      ComplexHypreParMatrix * A_hyp =
         new ComplexHypreParMatrix(A_r.As<HypreParMatrix>(),
                                   A_i.As<HypreParMatrix>(),
                                   A_r.OwnsOperator(),
                                   A_i.OwnsOperator(),
                                   conv);
      A.Reset<ComplexHypreParMatrix>(A_hyp, true);
   }
   else
   {
      ComplexOperator * A_op =
         new ComplexOperator(A_r.As<Operator>(),
                             A_i.As<Operator>(),
                             A_r.OwnsOperator(),
                             A_i.OwnsOperator(),
                             conv);
      A.Reset<ComplexOperator>(A_op, true);
   }
   A_r.SetOperatorOwner(false);
   A_i.SetOperatorOwner(false);
}

void
ParMixedSesquilinearForm::RecoverFEMSolution(const Vector &X, const Vector &b,
                                        Vector &x)
{
   ParFiniteElementSpace *pfes = pblfr->ParFESpace();

   const Operator &P = *pfes->GetProlongationMatrix();

   const int vsize  = pfes->GetVSize();
   const int tvsize = X.Size() / 2;

   X.Read();
   Vector X_r; X_r.MakeRef(const_cast<Vector&>(X), 0, tvsize);
   Vector X_i; X_i.MakeRef(const_cast<Vector&>(X), tvsize, tvsize);

   x.Write();
   Vector x_r; x_r.MakeRef(x, 0, vsize);
   Vector x_i; x_i.MakeRef(x, vsize, vsize);

   // Apply conforming prolongation
   P.Mult(X_r, x_r);
   P.Mult(X_i, x_i);

   x_r.SyncAliasMemory(x);
   x_i.SyncAliasMemory(x);
}

void
ParMixedSesquilinearForm::Update(FiniteElementSpace *nfes)
{
   if ( pblfr ) { pblfr->Update(nfes); }
   if ( pblfi ) { pblfi->Update(nfes); }
}

#endif