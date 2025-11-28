#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMGeneralUserObject.h"


class ParMixedSesquilinearForm
{
private:
   mfem::ComplexOperator::Convention conv;

   mfem::ParBilinearForm *pblfr;
   mfem::ParBilinearForm *pblfi;

   /* These methods check if the real/imag parts of the sesqulinear form are not
      empty */
   bool RealInteg();
   bool ImagInteg();

public:
   ParMixedSesquilinearForm(mfem::ParFiniteElementSpace *pf,
                       mfem::ComplexOperator::Convention
                       convention = mfem::ComplexOperator::HERMITIAN);

   /** @brief Create a ParMixedSesquilinearForm on the mfem::ParFiniteElementSpace @a pf,
       using the same integrators as the mfem::ParBilinearForms @a pbfr and @a pbfi .

       The pointer @a pf is not owned by the newly constructed object.

       The integrators are copied as pointers and they are not owned by the
       newly constructed ParMixedSesquilinearForm. */
   ParMixedSesquilinearForm(mfem::ParFiniteElementSpace *pf, mfem::ParBilinearForm *pbfr,
                       mfem::ParBilinearForm *pbfi,
                       mfem::ComplexOperator::Convention
                       convention = mfem::ComplexOperator::HERMITIAN);

   mfem::ComplexOperator::Convention GetConvention() const { return conv; }
   void SetConvention(const mfem::ComplexOperator::Convention &
                      convention) { conv = convention; }

   /// Set the desired assembly level.
   /** Valid choices are:

       - AssemblyLevel::LEGACY (default)
       - AssemblyLevel::FULL
       - AssemblyLevel::PARTIAL
       - AssemblyLevel::ELEMENT
       - AssemblyLevel::NONE

       This method must be called before assembly. */
   void SetAssemblyLevel(mfem::AssemblyLevel assembly_level)
   {
      pblfr->SetAssemblyLevel(assembly_level);
      pblfi->SetAssemblyLevel(assembly_level);
   }

   mfem::ParBilinearForm & real() { return *pblfr; }
   mfem::ParBilinearForm & imag() { return *pblfi; }
   const mfem::ParBilinearForm & real() const { return *pblfr; }
   const mfem::ParBilinearForm & imag() const { return *pblfi; }

   /// Adds new Domain Integrator.
   void AddDomainIntegrator(mfem::BilinearFormIntegrator *bfi_real,
                            mfem::BilinearFormIntegrator *bfi_imag);

   /// Adds new Domain Integrator, restricted to specific attributes.
   void AddDomainIntegrator(mfem::BilinearFormIntegrator *bfi_real,
                            mfem::BilinearFormIntegrator *bfi_imag,
                            mfem::Array<int> &elem_marker);

   /// Adds new Boundary Integrator.
   void AddBoundaryIntegrator(mfem::BilinearFormIntegrator *bfi_real,
                              mfem::BilinearFormIntegrator *bfi_imag);

   /** @brief Adds new boundary Integrator, restricted to specific boundary
       attributes.

       Assumes ownership of @a bfi.

       The mfem::array @a bdr_marker is stored internally as a pointer to the given
       mfem::Array<int> object. */
   void AddBoundaryIntegrator(mfem::BilinearFormIntegrator *bfi_real,
                              mfem::BilinearFormIntegrator *bfi_imag,
                              mfem::Array<int> &bdr_marker);

   /// Adds new interior Face Integrator. Assumes ownership of @a bfi.
   void AddInteriorFaceIntegrator(mfem::BilinearFormIntegrator *bfi_real,
                                  mfem::BilinearFormIntegrator *bfi_imag);

   /// Adds new boundary Face Integrator. Assumes ownership of @a bfi.
   void AddBdrFaceIntegrator(mfem::BilinearFormIntegrator *bfi_real,
                             mfem::BilinearFormIntegrator *bfi_imag);

   /** @brief Adds new boundary Face Integrator, restricted to specific boundary
       attributes.

       Assumes ownership of @a bfi.

       The mfem::array @a bdr_marker is stored internally as a pointer to the given
       mfem::Array<int> object. */
   void AddBdrFaceIntegrator(mfem::BilinearFormIntegrator *bfi_real,
                             mfem::BilinearFormIntegrator *bfi_imag,
                             mfem::Array<int> &bdr_marker);

   /// Assemble the local matrix
   void Assemble(int skip_zeros = 1);

   /// Finalizes the matrix initialization.
   void Finalize(int skip_zeros = 1);

   /// Returns the matrix assembled on the true dofs, i.e. P^t A P.
   /** The returned matrix has to be deleted by the caller. */
   mfem::ComplexHypreParMatrix *ParallelAssemble();

   /// Return the parallel FE space associated with the mfem::ParBilinearForm.
   mfem::ParFiniteElementSpace *ParFESpace() const { return pblfr->ParFESpace(); }

   void FormLinearSystem(const mfem::Array<int> &ess_tdof_list, mfem::Vector &x, mfem::Vector &b,
                         mfem::OperatorHandle &A, mfem::Vector &X, mfem::Vector &B,
                         int copy_interior = 0);

   void FormSystemMatrix(const mfem::Array<int> &ess_tdof_list,
                         mfem::OperatorHandle &A);

   /** Call this method after solving a linear system constructed using the
       FormLinearSystem method to recover the solution as a ParGridFunction-size
       mfem::vector in x. Use the same arguments as in the FormLinearSystem call. */
   virtual void RecoverFEMSolution(const mfem::Vector &X, const mfem::Vector &b, mfem::Vector &x);

   virtual void Update(mfem::FiniteElementSpace *nfes = NULL);

   virtual ~ParMixedSesquilinearForm();
};

#endif