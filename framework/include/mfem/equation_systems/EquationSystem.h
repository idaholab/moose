//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/pfem_extras.hpp"
#include "libmesh/restore_warnings.h"
#include "MFEMIntegratedBC.h"
#include "MFEMEssentialBC.h"
#include "MFEMContainers.h"
#include "MFEMKernel.h"
#include "MFEMMixedBilinearFormKernel.h"
#include "ScaleIntegrator.h"

namespace Moose::MFEM
{

/**
 * Class to store weak form components (bilinear and linear forms, and optionally
 * mixed and nonlinear forms) and build methods
 */
class EquationSystem : public mfem::Operator
{

public:
  friend class EquationSystemProblemOperator;
  friend class TimeDomainEquationSystemProblemOperator;

  EquationSystem() = default;
  ~EquationSystem() override;

  /// Add test variable to EquationSystem.
  virtual void AddTestVariableNameIfMissing(const std::string & test_var_name);
  /// Add trial variable to EquationSystem.
  virtual void AddTrialVariableNameIfMissing(const std::string & trial_var_name);

  /// Add kernels.
  virtual void AddKernel(std::shared_ptr<MFEMKernel> kernel);
  virtual void AddIntegratedBC(std::shared_ptr<MFEMIntegratedBC> kernel);
  virtual void AddEssentialBC(std::shared_ptr<MFEMEssentialBC> bc);
  virtual void ApplyEssentialBCs();

  /// Build forms
  virtual void Init(Moose::MFEM::GridFunctions & gridfunctions,
                    const Moose::MFEM::FESpaces & fespaces,
                    mfem::AssemblyLevel assembly_level);
  virtual void BuildLinearForms();
  virtual void BuildBilinearForms();
  virtual void BuildMixedBilinearForms();
  virtual void BuildEquationSystem();

  /// Form linear system, with essential boundary conditions accounted for
  virtual void FormLinearSystem(mfem::OperatorHandle & op,
                                mfem::BlockVector & trueX,
                                mfem::BlockVector & trueRHS);

  virtual void
  FormSystem(mfem::OperatorHandle & op, mfem::BlockVector & trueX, mfem::BlockVector & trueRHS);
  virtual void FormLegacySystem(mfem::OperatorHandle & op,
                                mfem::BlockVector & trueX,
                                mfem::BlockVector & trueRHS);

  /// Build linear system, with essential boundary conditions accounted for
  virtual void BuildJacobian(mfem::BlockVector & trueX, mfem::BlockVector & trueRHS);

  /// Compute residual y = Mu
  void Mult(const mfem::Vector & u, mfem::Vector & residual) const override;

  /// Compute J = M + grad_H(u)
  mfem::Operator & GetGradient(const mfem::Vector & u) const override;

  /// Update variable from solution vector after solve
  virtual void RecoverFEMSolution(mfem::BlockVector & trueX,
                                  Moose::MFEM::GridFunctions & gridfunctions);

  std::vector<mfem::Array<int>> _ess_tdof_lists;

  const std::vector<std::string> & TrialVarNames() const { return _trial_var_names; }
  const std::vector<std::string> & TestVarNames() const { return _test_var_names; }

private:
  /// Disallowed inherited method
  using mfem::Operator::RecoverFEMSolution;

protected:
  /// Deletes the HypreParMatrix associated with any pointer stored in _h_blocks,
  /// and then proceeds to delete all dynamically allocated memory for _h_blocks
  /// itself, resetting all dimensions to zero.
  void DeleteAllBlocks();

  bool VectorContainsName(const std::vector<std::string> & the_vector,
                          const std::string & name) const;

  // Test variables are associated with LinearForms,
  // whereas trial variables are associated with gridfunctions.

  /// Names of all variables corresponding to gridfunctions. This may differ
  /// from test_var_names when time derivatives are present.
  std::vector<std::string> _trial_var_names;
  /// Pointers to trial variables.
  Moose::MFEM::GridFunctions _trial_variables;
  /// Names of all test variables corresponding to linear forms in this equation system
  std::vector<std::string> _test_var_names;
  /// Pointers to finite element spaces associated with test variables.
  std::vector<mfem::ParFiniteElementSpace *> _test_pfespaces;

  // Components of weak form. // Named according to test variable
  Moose::MFEM::NamedFieldsMap<mfem::ParBilinearForm> _blfs;
  Moose::MFEM::NamedFieldsMap<mfem::ParLinearForm> _lfs;
  Moose::MFEM::NamedFieldsMap<mfem::ParNonlinearForm> _nlfs;
  Moose::MFEM::NamedFieldsMap<Moose::MFEM::NamedFieldsMap<mfem::ParMixedBilinearForm>>
      _mblfs; // named according to trial variable

  /**
   * Template method for applying BilinearFormIntegrators on domains from kernels to a BilinearForm,
   * or MixedBilinearForm
   */
  template <class FormType>
  void ApplyDomainBLFIntegrators(
      const std::string & trial_var_name,
      const std::string & test_var_name,
      std::shared_ptr<FormType> form,
      Moose::MFEM::NamedFieldsMap<
          Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>> & kernels_map);

  void ApplyDomainLFIntegrators(
      const std::string & test_var_name,
      std::shared_ptr<mfem::ParLinearForm> form,
      Moose::MFEM::NamedFieldsMap<
          Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>> & kernels_map);

  template <class FormType>
  void ApplyBoundaryBLFIntegrators(
      const std::string & trial_var_name,
      const std::string & test_var_name,
      std::shared_ptr<FormType> form,
      Moose::MFEM::NamedFieldsMap<
          Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMIntegratedBC>>>> &
          integrated_bc_map);

  void ApplyBoundaryLFIntegrators(
      const std::string & test_var_name,
      std::shared_ptr<mfem::ParLinearForm> form,
      Moose::MFEM::NamedFieldsMap<
          Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMIntegratedBC>>>> &
          integrated_bc_map);

  /// Gridfunctions for setting Dirichlet BCs
  std::vector<std::unique_ptr<mfem::ParGridFunction>> _xs;
  std::vector<std::unique_ptr<mfem::ParGridFunction>> _dxdts;

  mfem::Array2D<const mfem::HypreParMatrix *> _h_blocks;

  /// Arrays to store kernels to act on each component of weak form.
  /// Named according to test and trial variables.
  Moose::MFEM::NamedFieldsMap<Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>>
      _kernels_map;
  /// Arrays to store integrated BCs to act on each component of weak form.
  /// Named according to test and trial variables.
  Moose::MFEM::NamedFieldsMap<
      Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMIntegratedBC>>>>
      _integrated_bc_map;
  /// Arrays to store essential BCs to act on each component of weak form.
  /// Named according to test variable.
  Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMEssentialBC>>> _essential_bc_map;

  mutable mfem::OperatorHandle _jacobian;

  mfem::AssemblyLevel _assembly_level;
};

template <class FormType>
void
EquationSystem::ApplyDomainBLFIntegrators(
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
      mfem::BilinearFormIntegrator * integ = kernel->createBFIntegrator();
      if (integ != nullptr)
      {
        kernel->isSubdomainRestricted()
            ? form->AddDomainIntegrator(std::move(integ), kernel->getSubdomainMarkers())
            : form->AddDomainIntegrator(std::move(integ));
      }
    }
  }
}

inline void
EquationSystem::ApplyDomainLFIntegrators(
    const std::string & test_var_name,
    std::shared_ptr<mfem::ParLinearForm> form,
    Moose::MFEM::NamedFieldsMap<
        Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>> & kernels_map)
{
  if (kernels_map.Has(test_var_name) && kernels_map.Get(test_var_name)->Has(test_var_name))
  {
    auto kernels = kernels_map.GetRef(test_var_name).GetRef(test_var_name);
    for (auto & kernel : kernels)
    {
      mfem::LinearFormIntegrator * integ = kernel->createLFIntegrator();
      if (integ != nullptr)
      {
        kernel->isSubdomainRestricted()
            ? form->AddDomainIntegrator(std::move(integ), kernel->getSubdomainMarkers())
            : form->AddDomainIntegrator(std::move(integ));
      }
    }
  }
}

template <class FormType>
void
EquationSystem::ApplyBoundaryBLFIntegrators(
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
      mfem::BilinearFormIntegrator * integ = bc->createBFIntegrator();
      if (integ != nullptr)
      {
        bc->isBoundaryRestricted()
            ? form->AddBoundaryIntegrator(std::move(integ), bc->getBoundaryMarkers())
            : form->AddBoundaryIntegrator(std::move(integ));
      }
    }
  }
}

inline void
EquationSystem::ApplyBoundaryLFIntegrators(
    const std::string & test_var_name,
    std::shared_ptr<mfem::ParLinearForm> form,
    Moose::MFEM::NamedFieldsMap<
        Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMIntegratedBC>>>> &
        integrated_bc_map)
{
  if (integrated_bc_map.Has(test_var_name) &&
      integrated_bc_map.Get(test_var_name)->Has(test_var_name))
  {
    auto bcs = integrated_bc_map.GetRef(test_var_name).GetRef(test_var_name);
    for (auto & bc : bcs)
    {
      mfem::LinearFormIntegrator * integ = bc->createLFIntegrator();
      if (integ != nullptr)
      {
        bc->isBoundaryRestricted()
            ? form->AddBoundaryIntegrator(std::move(integ), bc->getBoundaryMarkers())
            : form->AddBoundaryIntegrator(std::move(integ));
      }
    }
  }
}

/**
 * Class to store weak form components for time dependent PDEs
 */
class TimeDependentEquationSystem : public EquationSystem
{
public:
  TimeDependentEquationSystem();

  void AddTrialVariableNameIfMissing(const std::string & trial_var_name) override;

  virtual void SetTimeStep(double dt);
  virtual void UpdateEquationSystem();

  virtual void AddKernel(std::shared_ptr<MFEMKernel> kernel) override;
  virtual void BuildBilinearForms() override;
  virtual void FormLegacySystem(mfem::OperatorHandle & op,
                                mfem::BlockVector & truedXdt,
                                mfem::BlockVector & trueRHS) override;
  virtual void FormSystem(mfem::OperatorHandle & op,
                          mfem::BlockVector & truedXdt,
                          mfem::BlockVector & trueRHS) override;

  const std::vector<std::string> & TrialVarTimeDerivativeNames() const;

protected:
  /// Coefficient for timestep scaling
  mfem::ConstantCoefficient _dt_coef;
  std::vector<std::string> _trial_var_time_derivative_names;

  Moose::MFEM::NamedFieldsMap<Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>>
      _td_kernels_map;
  /// Container to store contributions to weak form of the form (F du/dt, v)
  Moose::MFEM::NamedFieldsMap<mfem::ParBilinearForm> _td_blfs;
};

inline const std::vector<std::string> &
TimeDependentEquationSystem::TrialVarTimeDerivativeNames() const
{
  return _trial_var_time_derivative_names;
}

} // namespace Moose::MFEM

#endif
