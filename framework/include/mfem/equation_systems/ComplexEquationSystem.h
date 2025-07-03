#ifdef MFEM_ENABLED

#pragma once
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/pfem_extras.hpp"
#include "libmesh/restore_warnings.h"
#include "MFEMContainers.h"
#include "MFEMKernel.h"

namespace Moose::MFEM
{
/*
Class to store weak form components (bilinear and linear forms, and optionally
mixed and nonlinear forms) and build methods
*/
class ComplexEquationSystem : public Moose::MFEM::EquationSystem
{

public:

  ComplexEquationSystem() = default;
  ~ComplexEquationSystem() = default;

  // Build forms
  virtual void Init(Moose::MFEM::ComplexGridFunctions & cpx_gridfunctions,
                    const Moose::MFEM::FESpaces & fespaces,
                    mfem::AssemblyLevel assembly_level);

  virtual void BuildEquationSystem() override;
  virtual void BuildLinearForms() override;
  virtual void BuildBilinearForms() override;
  virtual void ApplyEssentialBCs() override;

template <class FormType>
void
ApplyDomainBLFIntegrators(
    const std::string & trial_var_name,
    const std::string & test_var_name,
    std::shared_ptr<FormType> form,
    Moose::MFEM::NamedFieldsMap<
        Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>> & kernels_map);

inline void
ApplyDomainLFIntegrators(
    const std::string & test_var_name,
    std::shared_ptr<mfem::ParComplexLinearForm> form,
    Moose::MFEM::NamedFieldsMap<
        Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>> & kernels_map);

template <class FormType>
void
ApplyBoundaryBLFIntegrators(
    const std::string & trial_var_name,
    const std::string & test_var_name,
    std::shared_ptr<FormType> form,
    Moose::MFEM::NamedFieldsMap<
        Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMIntegratedBC>>>> &
        integrated_bc_map);

inline void
ApplyBoundaryLFIntegrators(
    const std::string & test_var_name,
    std::shared_ptr<mfem::ParComplexLinearForm> form,
    Moose::MFEM::NamedFieldsMap<
        Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMIntegratedBC>>>> &
        integrated_bc_map);

  // Complex Linear and Bilinear Forms
  Moose::MFEM::NamedFieldsMap<mfem::ParSesquilinearForm> _slfs;
  Moose::MFEM::NamedFieldsMap<mfem::ParComplexLinearForm> _clfs;

  // Complex gridfunctions for setting Dirichlet BCs
  std::vector<std::unique_ptr<mfem::ParComplexGridFunction>> _cxs;
  std::vector<std::unique_ptr<mfem::ParComplexGridFunction>> _cdxdts;

  // Complex gridfunctions for the problem
  Moose::MFEM::ComplexGridFunctions _cpx_trial_variables;

};

template <class FormType>
void
ComplexEquationSystem::ApplyDomainBLFIntegrators(
    const std::string & trial_var_name,
    const std::string & test_var_name,
    std::shared_ptr<FormType> form,
    Moose::MFEM::NamedFieldsMap<
        Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>> & kernels_map)
{
  if (kernels_map.Has(test_var_name) && kernels_map.Get(test_var_name)->Has(trial_var_name))
  {
    auto kernels = kernels_map.GetRef(test_var_name).GetRef(trial_var_name);
    for (auto & kernel : kernels)
    {
      std::pair<mfem::BilinearFormIntegrator *, mfem::BilinearFormIntegrator *> integ = kernel->createBFIntegrator();
      if (integ.first != nullptr && integ.second != nullptr)
      {
        kernel->isSubdomainRestricted()
            ? form->AddDomainIntegrator(std::move(integ.first), std::move(integ.second), kernel->getSubdomains())
            : form->AddDomainIntegrator(std::move(integ.first), std::move(integ.second));
      }
    }
  }
}

inline void
ComplexEquationSystem::ApplyDomainLFIntegrators(
    const std::string & test_var_name,
    std::shared_ptr<mfem::ParComplexLinearForm> form,
    Moose::MFEM::NamedFieldsMap<
        Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>> & kernels_map)
{
  if (kernels_map.Has(test_var_name))
  {
    auto kernels = kernels_map.GetRef(test_var_name).GetRef(test_var_name);
    for (auto & kernel : kernels)
    {
      std::pair<mfem::LinearFormIntegrator *, mfem::LinearFormIntegrator *> integ = kernel->createLFIntegrator();
      if (integ.first != nullptr && integ.second != nullptr)
      {
        kernel->isSubdomainRestricted()
            ? form->AddDomainIntegrator(std::move(integ.first), std::move(integ.second), kernel->getSubdomains())
            : form->AddDomainIntegrator(std::move(integ.first), std::move(integ.second));
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
    Moose::MFEM::NamedFieldsMap<
        Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMIntegratedBC>>>> &
        integrated_bc_map)
{
  if (integrated_bc_map.Has(test_var_name) &&
      integrated_bc_map.Get(test_var_name)->Has(trial_var_name))
  {
    auto bcs = integrated_bc_map.GetRef(test_var_name).GetRef(trial_var_name);
    for (auto & bc : bcs)
    {
      std::pair<mfem::BilinearFormIntegrator *, mfem::BilinearFormIntegrator *> integ = bc->createBFIntegrator();
      if (integ.first != nullptr && integ.second != nullptr)
      {
        bc->isBoundaryRestricted()
            ? form->AddBoundaryIntegrator(std::move(integ.first), std::move(integ.second), bc->getBoundaries())
            : form->AddBoundaryIntegrator(std::move(integ.first), std::move(integ.second));
      }
    }
  }
}

inline void
ComplexEquationSystem::ApplyBoundaryLFIntegrators(
    const std::string & test_var_name,
    std::shared_ptr<mfem::ParComplexLinearForm> form,
    Moose::MFEM::NamedFieldsMap<
        Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMIntegratedBC>>>> &
        integrated_bc_map)
{
  if (integrated_bc_map.Has(test_var_name))
  {
    auto bcs = integrated_bc_map.GetRef(test_var_name).GetRef(test_var_name);
    for (auto & bc : bcs)
    {
      std::pair<mfem::LinearFormIntegrator *, mfem::LinearFormIntegrator *> integ = bc->createLFIntegrator();
      if (integ.first != nullptr && integ.second != nullptr)
      {
        bc->isBoundaryRestricted()
            ? form->AddBoundaryIntegrator(std::move(integ.first), std::move(integ.second), bc->getBoundaries())
            : form->AddBoundaryIntegrator(std::move(integ.first), std::move(integ.second));
      }
    }
  }
}

}

#endif
