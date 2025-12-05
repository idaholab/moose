#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMGeneralUserObject.h"

namespace Moose::MFEM
{

class ParMixedSesquilinearForm
{
private:
   mfem::ComplexOperator::Convention _conv;

   mfem::ParMixedBilinearForm *_pmblfr;
   mfem::ParMixedBilinearForm *_pmblfi;

   /* These methods check if the real/imag parts of the sesqulinear form are not
      empty */
   bool RealInteg();
   bool ImagInteg();

public:
   ParMixedSesquilinearForm(mfem::ParFiniteElementSpace *trial_fes, mfem::ParFiniteElementSpace *test_fes,
                       mfem::ComplexOperator::Convention
                       convention = mfem::ComplexOperator::HERMITIAN);

   /** @brief Create a ParMixedSesquilinearForm on the mfem::ParFiniteElementSpace @a pf,
       using the same integrators as the mfem::ParMixedBilinearForms @a pbfr and @a pbfi .

       The pointer @a pf is not owned by the newly constructed object.

       The integrators are copied as pointers and they are not owned by the
       newly constructed ParMixedSesquilinearForm. */
   ParMixedSesquilinearForm(mfem::ParFiniteElementSpace *trial_fes, mfem::ParFiniteElementSpace *test_fes,
                       mfem::ParMixedBilinearForm *pbfr,
                       mfem::ParMixedBilinearForm *pbfi,
                       mfem::ComplexOperator::Convention
                       convention = mfem::ComplexOperator::HERMITIAN);

   mfem::ComplexOperator::Convention GetConvention() const { return _conv; }
   void SetConvention(const mfem::ComplexOperator::Convention &
                      convention) { _conv = convention; }

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
      _pmblfr->SetAssemblyLevel(assembly_level);
      _pmblfi->SetAssemblyLevel(assembly_level);
   }

   mfem::ParMixedBilinearForm & real() { return *_pmblfr; }
   mfem::ParMixedBilinearForm & imag() { return *_pmblfi; }
   const mfem::ParMixedBilinearForm & real() const { return *_pmblfr; }
   const mfem::ParMixedBilinearForm & imag() const { return *_pmblfi; }

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

   void FormRectangularLinearSystem(const mfem::Array<int> &ess_trial_tdof_list, const mfem::Array<int> &ess_test_tdof_list, mfem::Vector &x, mfem::Vector &b,
                         mfem::OperatorHandle &A, mfem::Vector &X, mfem::Vector &B);

   void FormRectangularSystemMatrix(const mfem::Array<int> &ess_trial_tdof_list, const mfem::Array<int> &ess_test_tdof_list,
                         mfem::OperatorHandle &A);

   virtual ~ParMixedSesquilinearForm();
};

}

#endif
