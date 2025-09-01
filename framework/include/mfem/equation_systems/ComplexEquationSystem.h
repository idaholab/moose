#ifdef MOOSE_MFEM_ENABLED

#pragma once
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/pfem_extras.hpp"
#include "libmesh/restore_warnings.h"
#include "MFEMContainers.h"
#include "MFEMComplexKernel.h"
#include "MFEMComplexIntegratedBC.h"
#include "MFEMComplexEssentialBC.h"

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
                    ComplexGridFunctions & cpx_gridfunctions,
                    const FESpaces & fespaces,
                    mfem::AssemblyLevel assembly_level) override;

  virtual void BuildEquationSystem() override;
  virtual void BuildLinearForms() override;
  virtual void BuildBilinearForms() override;
  virtual void ApplyEssentialBCs() override;

  virtual void AddKernel(std::shared_ptr<MFEMKernel> kernel) override;
  void AddIntegratedBC(std::shared_ptr<MFEMIntegratedBC> bc) override;
  void AddComplexEssentialBCs(std::shared_ptr<MFEMComplexEssentialBC> bc);

  virtual void FormSystem(mfem::OperatorHandle & op,
                          mfem::BlockVector & trueX,
                          mfem::BlockVector & trueRHS) override;
  virtual void FormLegacySystem(mfem::OperatorHandle & op,
                                mfem::BlockVector & trueX,
                                mfem::BlockVector & trueRHS) override;

  template <class FormType>
  void ApplyDomainBLFIntegrators(
      const std::string & trial_var_name,
      const std::string & test_var_name,
      std::shared_ptr<FormType> form,
      NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexKernel>>>> &
          kernels_map);

  inline void ApplyDomainLFIntegrators(
      const std::string & test_var_name,
      std::shared_ptr<mfem::ParComplexLinearForm> form,
      NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexKernel>>>> &
          kernels_map);

  template <class FormType>
  void ApplyBoundaryBLFIntegrators(
      const std::string & trial_var_name,
      const std::string & test_var_name,
      std::shared_ptr<FormType> form,
      NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexIntegratedBC>>>> &
          integrated_bc_map);

  inline void ApplyBoundaryLFIntegrators(
      const std::string & test_var_name,
      std::shared_ptr<mfem::ParComplexLinearForm> form,
      NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexIntegratedBC>>>> &
          integrated_bc_map);

  // Complex Linear and Bilinear Forms
  NamedFieldsMap<mfem::ParSesquilinearForm> _slfs;
  NamedFieldsMap<mfem::ParComplexLinearForm> _clfs;

  // Complex gridfunctions for setting Dirichlet BCs
  std::vector<std::unique_ptr<mfem::ParComplexGridFunction>> _cxs;
  std::vector<std::unique_ptr<mfem::ParComplexGridFunction>> _cdxdts;

  // Complex kernels and integrated BCs
  NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexKernel>>>> _cpx_kernels_map;
  NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexIntegratedBC>>>>
      _cpx_integrated_bc_map;

  // Complex essential BCs
  NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexEssentialBC>>> _cpx_essential_bc_map;

  // Complex trial variables
  ComplexGridFunctions _cpx_trial_variables;
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

      if (integ_real != nullptr || integ_imag != nullptr)
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

      if (integ_real != nullptr && integ_imag != nullptr)
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

      if (integ_real != nullptr || integ_imag != nullptr)
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

      if (integ_real != nullptr && integ_imag != nullptr)
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
