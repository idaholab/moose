#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/pfem_extras.hpp"
#include "libmesh/restore_warnings.h"
#include "EquationSystem.h"
#include "MFEMComplexKernel.h"
#include "MFEMComplexIntegratedBC.h"
#include "MFEMComplexEssentialBC.h"
#include "ParMixedSesquilinearForm.h"

namespace Moose::MFEM
{
/*
Class to store weak form components (bilinear and linear forms, and optionally
mixed and nonlinear forms) and build methods
*/
class ComplexEquationSystem : public EquationSystem
{

public:
  ComplexEquationSystem() = default;
  ~ComplexEquationSystem() = default;

  // Build forms
  virtual void Init(GridFunctions & gridfunctions,
                    ComplexGridFunctions & cmplx_gridfunctions,
                    mfem::AssemblyLevel assembly_level) override;

  /// Build all forms comprising this EquationSystem
  virtual void BuildEquationSystem() override;

  /// Build linear forms and eliminate constrained DoFs
  virtual void BuildLinearForms() override;

  /// Build bilinear forms (diagonal Jacobian contributions)
  virtual void BuildBilinearForms() override;

  /// Apply essential BC(s) associated with var_name to set true DoFs of trial_gf and update
  /// markers of all essential boundaries
  virtual void ApplyComplexEssentialBC(const std::string & var_name,
                                       mfem::ParComplexGridFunction & trial_gf,
                                       mfem::Array<int> & global_ess_markers);
  /// Update all essentially constrained true DoF markers and values on boundaries
  virtual void ApplyEssentialBCs() override;

  /// Add complex kernels
  void AddComplexKernel(std::shared_ptr<MFEMComplexKernel> kernel);

  /// Add complex integrated BCs
  void AddComplexIntegratedBC(std::shared_ptr<MFEMComplexIntegratedBC> bc);

  /// Add complex essential BCs
  void AddComplexEssentialBCs(std::shared_ptr<MFEMComplexEssentialBC> bc);

  /// Form matrix-free representation of system operator.
  /// Used when EquationSystem assembly level is set to 'FULL', 'ELEMENT', 'PARTIAL', or 'NONE'.
  virtual void FormSystemOperator(mfem::OperatorHandle & op,
                                  mfem::BlockVector & trueX,
                                  mfem::BlockVector & trueRHS) override;

  /// Form matrix representation of system operator as a HypreParMatrix.
  /// Used when EquationSystem assembly level is set to 'LEGACY'.
  virtual void FormSystemMatrix(mfem::OperatorHandle & op,
                                mfem::BlockVector & trueX,
                                mfem::BlockVector & trueRHS) override;

  /// Update variable from solution vector after solve
  void RecoverComplexFEMSolution(mfem::BlockVector & trueX,
                                 Moose::MFEM::GridFunctions & gridfunctions,
                                 Moose::MFEM::ComplexGridFunctions & cmplx_gridfunctions);

  /// Template method for applying BilinearFormIntegrators on domains from kernels to a SesquilinearForm
  template <class FormType>
  void ApplyDomainBLFIntegrators(
      const std::string & trial_var_name,
      const std::string & test_var_name,
      std::shared_ptr<FormType> form,
      NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexKernel>>>> &
          kernels_map);

  /// Method for applying LinearFormIntegrators on domains from kernels to a ParComplexLinearForm
  inline void ApplyDomainLFIntegrators(
      const std::string & test_var_name,
      std::shared_ptr<mfem::ParComplexLinearForm> form,
      NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexKernel>>>> &
          kernels_map);

  /// Template method for applying BilinearFormIntegrators on boudaries from kernels to a SesquilinearForm
  template <class FormType>
  void ApplyBoundaryBLFIntegrators(
      const std::string & trial_var_name,
      const std::string & test_var_name,
      std::shared_ptr<FormType> form,
      NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexIntegratedBC>>>> &
          integrated_bc_map);

  /// Method for applying LinearFormIntegrators on boundaries from kernels to a ParComplexLinearForm
  inline void ApplyBoundaryLFIntegrators(
      const std::string & test_var_name,
      std::shared_ptr<mfem::ParComplexLinearForm> form,
      NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexIntegratedBC>>>> &
          integrated_bc_map);

  // Complex Linear and Bilinear Forms
  NamedFieldsMap<mfem::ParSesquilinearForm> _slfs;
  NamedFieldsMap<ParMixedSesquilinearForm> _mslfs;
  NamedFieldsMap<mfem::ParComplexLinearForm> _clfs;

  // Complex kernels and integrated BCs
  NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexKernel>>>>
      _cmplx_kernels_map;
  NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexIntegratedBC>>>>
      _cmplx_integrated_bc_map;

  // Complex essential BCs
  NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexEssentialBC>>> _cmplx_essential_bc_map;

  /// Pointers to coupled variables not part of the reduced EquationSystem.
  ComplexGridFunctions _cmplx_eliminated_variables;

  /// Complex Gridfunctions holding essential constraints from Dirichlet BCs
  std::vector<std::unique_ptr<mfem::ParComplexGridFunction>> _cmplx_var_ess_constraints;
};

template <class FormType>
void
ComplexEquationSystem::ApplyDomainBLFIntegrators(
    const std::string & trial_var_name,
    const std::string & test_var_name,
    std::shared_ptr<FormType> form,
    NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexKernel>>>> & kernels_map)
{
  if (kernels_map.Has(test_var_name) && kernels_map.Get(test_var_name)->Has(trial_var_name))
  {
    auto kernels = kernels_map.GetRef(test_var_name).GetRef(trial_var_name);
    for (auto & kernel : kernels)
    {
      mfem::BilinearFormIntegrator * integ_real = kernel->getRealBFIntegrator();
      mfem::BilinearFormIntegrator * integ_imag = kernel->getImagBFIntegrator();

      if (integ_real || integ_imag)
      {
        kernel->isSubdomainRestricted()
            ? form->AddDomainIntegrator(
                  std::move(integ_real), std::move(integ_imag), kernel->getSubdomainMarkers())
            : form->AddDomainIntegrator(std::move(integ_real), std::move(integ_imag));
      }
    }
  }
}

inline void
ComplexEquationSystem::ApplyDomainLFIntegrators(
    const std::string & test_var_name,
    std::shared_ptr<mfem::ParComplexLinearForm> form,
    NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexKernel>>>> & kernels_map)
{
  if (kernels_map.Has(test_var_name))
  {
    auto kernels = kernels_map.GetRef(test_var_name).GetRef(test_var_name);
    for (auto & kernel : kernels)
    {
      mfem::LinearFormIntegrator * integ_real = kernel->getRealLFIntegrator();
      mfem::LinearFormIntegrator * integ_imag = kernel->getImagLFIntegrator();

      if (integ_real || integ_imag)
      {
        kernel->isSubdomainRestricted()
            ? form->AddDomainIntegrator(
                  std::move(integ_real), std::move(integ_imag), kernel->getSubdomainMarkers())
            : form->AddDomainIntegrator(std::move(integ_real), std::move(integ_imag));
      }
    }
  }
}

template <class FormType>
void
ComplexEquationSystem::ApplyBoundaryBLFIntegrators(
    const std::string & trial_var_name,
    const std::string & test_var_name,
    std::shared_ptr<FormType> form,
    NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexIntegratedBC>>>> &
        integrated_bc_map)
{
  if (integrated_bc_map.Has(test_var_name) &&
      integrated_bc_map.Get(test_var_name)->Has(trial_var_name))
  {
    auto bcs = integrated_bc_map.GetRef(test_var_name).GetRef(trial_var_name);
    for (auto & bc : bcs)
    {
      mfem::BilinearFormIntegrator * integ_real = bc->getRealBFIntegrator();
      mfem::BilinearFormIntegrator * integ_imag = bc->getImagBFIntegrator();

      if (integ_real || integ_imag)
      {
        bc->isBoundaryRestricted()
            ? form->AddBoundaryIntegrator(
                  std::move(integ_real), std::move(integ_imag), bc->getBoundaryMarkers())
            : form->AddBoundaryIntegrator(std::move(integ_real), std::move(integ_imag));
      }
    }
  }
}

inline void
ComplexEquationSystem::ApplyBoundaryLFIntegrators(
    const std::string & test_var_name,
    std::shared_ptr<mfem::ParComplexLinearForm> form,
    NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexIntegratedBC>>>> &
        integrated_bc_map)
{
  if (integrated_bc_map.Has(test_var_name))
  {
    auto bcs = integrated_bc_map.GetRef(test_var_name).GetRef(test_var_name);
    for (auto & bc : bcs)
    {
      mfem::LinearFormIntegrator * integ_real = bc->getRealLFIntegrator();
      mfem::LinearFormIntegrator * integ_imag = bc->getImagLFIntegrator();

      if (integ_real || integ_imag)
      {
        bc->isBoundaryRestricted()
            ? form->AddBoundaryIntegrator(
                  std::move(integ_real), std::move(integ_imag), bc->getBoundaryMarkers())
            : form->AddBoundaryIntegrator(std::move(integ_real), std::move(integ_imag));
      }
    }
  }
}

}

#endif
