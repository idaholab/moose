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

  EquationSystem() = default;
  ~EquationSystem() override;

  /// Add test variable to EquationSystem.
  virtual void AddTestVariableNameIfMissing(const std::string & test_var_name);
  /// Add coupled variable to EquationSystem.
  virtual void AddCoupledVariableNameIfMissing(const std::string & coupled_var_name);

  /// Add kernels.
  virtual void AddKernel(std::shared_ptr<MFEMKernel> kernel);
  virtual void AddIntegratedBC(std::shared_ptr<MFEMIntegratedBC> kernel);

  /// Add BC associated with essentially constrained DoFs on boundaries.
  virtual void AddEssentialBC(std::shared_ptr<MFEMEssentialBC> bc);

  /// Initialise
  virtual void Init(Moose::MFEM::GridFunctions & gridfunctions, mfem::AssemblyLevel assembly_level);

  /// Build linear forms and eliminate constrained DoFs
  virtual void BuildLinearForms();

  /// Apply essential BC(s) associated with test_var_name to set true DoFs of trial_gf and update
  /// markers of all essential boundaries
  virtual void ApplyEssentialBC(const std::string & test_var_name,
                                mfem::ParGridFunction & trial_gf,
                                mfem::Array<int> & global_ess_markers);
  /// Update all essentially constrained true DoF markers and values on boundaries
  virtual void ApplyEssentialBCs();

  virtual void BuildNonLinearActions();

  /// Perform trivial eliminations of coupled variables lacking corresponding test variables
  virtual void EliminateCoupledVariables();

  /// Build bilinear forms (diagonal Jacobian contributions)
  virtual void BuildBilinearForms();
  /// Build mixed bilinear forms (off-diagonal Jacobian contributions)
  virtual void BuildMixedBilinearForms();
  /// Build all forms comprising this EquationSystem
  virtual void BuildEquationSystem(Moose::MFEM::GridFunctions & gridfunctions,
                                   mfem::Array<int> & btoffsets);

  /// Form Jacobian operator based on on- and off-diagonal bilinear form contributions, populate
  /// solution and RHS vectors of true DoFs, and apply constraints
  void AssembleJacobian(
      Moose::MFEM::NamedFieldsMap<mfem::ParBilinearForm> & jac_blfs,
      Moose::MFEM::NamedFieldsMap<Moose::MFEM::NamedFieldsMap<mfem::ParMixedBilinearForm>> &
          jac_mblfs,
      Moose::MFEM::NamedFieldsMap<mfem::ParLinearForm> & rhs_lfs,
      std::vector<mfem::Array<int>> & ess_tdof_lists,
      std::vector<std::unique_ptr<mfem::ParGridFunction>> & var_ess_constraints,
      mfem::OperatorHandle & op,
      mfem::BlockVector & trueX,
      mfem::BlockVector & trueRHS);

  /// Form linear system, with essential boundary conditions accounted for
  virtual void FormLinearSystem(mfem::OperatorHandle & op,
                                mfem::BlockVector & trueX,
                                mfem::BlockVector & trueRHS);

  virtual void
  FormSystem(mfem::OperatorHandle & op, mfem::BlockVector & trueX, mfem::BlockVector & trueRHS);
  virtual void FormLegacySystem(mfem::OperatorHandle & op,
                                mfem::BlockVector & trueX,
                                mfem::BlockVector & trueRHS);

  void UpdateJacobian() const;

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

  /// Set trial variable names from subset of coupled variables that have an associated test variable.
  virtual void SetTrialVariableNames();

protected:
  /// Deletes the HypreParMatrix associated with any pointer stored in _h_blocks,
  /// and then proceeds to delete all dynamically allocated memory for _h_blocks
  /// itself, resetting all dimensions to zero.
  void DeleteAllBlocks();

  bool VectorContainsName(const std::vector<std::string> & the_vector,
                          const std::string & name) const;

  // Test variables are associated with LinearForms,
  // whereas trial variables are associated with gridfunctions.

  /// Names of all trial variables of kernels and boundary conditions
  /// added to this EquationSystem.
  std::vector<std::string> _coupled_var_names;
  /// Subset of _coupled_var_names of all variables corresponding to gridfunctions with degrees of
  /// freedom that comprise the state vector of this EquationSystem. This will differ from
  /// _coupled_var_names when time derivatives or other eliminated variables are present.
  std::vector<std::string> _trial_var_names;
  /// Names of all coupled variables without a corresponding test variable.
  std::vector<std::string> _eliminated_var_names;
  /// Pointers to coupled variables not part of the reduced EquationSystem.
  Moose::MFEM::GridFunctions _eliminated_variables;
  /// Names of all test variables corresponding to linear forms in this equation system
  std::vector<std::string> _test_var_names;
  /// Pointers to finite element spaces associated with test variables.
  std::vector<mfem::ParFiniteElementSpace *> _test_pfespaces;
  /// Pointers to finite element spaces associated with coupled variables.
  std::vector<mfem::ParFiniteElementSpace *> _coupled_pfespaces;

  // Components of weak form. // Named according to test variable
  Moose::MFEM::NamedFieldsMap<mfem::ParBilinearForm> _blfs;
  Moose::MFEM::NamedFieldsMap<mfem::ParLinearForm> _lfs;
  Moose::MFEM::NamedFieldsMap<mfem::ParLinearForm> _nlfs;
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

  void ApplyDomainNLActionIntegrators(
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

  /// Gridfunctions holding essential constraints from Dirichlet BCs
  std::vector<std::unique_ptr<mfem::ParGridFunction>> _var_ess_constraints;

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
  mutable mfem::Vector _trueRHS;
  mutable mfem::BlockVector _trueBlockRHS, _trueBlockSol, _BlockResidual;

  Moose::MFEM::GridFunctions * _gfuncs;
  mfem::Array<int> * _block_true_offsets;
  mfem::Array<int> empty_tdof;

  mfem::AssemblyLevel _assembly_level;

  bool _non_linear = false;
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

inline void
EquationSystem::ApplyDomainNLActionIntegrators(
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
      mfem::LinearFormIntegrator * integ = kernel->createNLActionIntegrator();
      if (integ != nullptr)
      {
        _non_linear = true;
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
  friend class TimeDomainEquationSystemProblemOperator;
  TimeDependentEquationSystem(const Moose::MFEM::TimeDerivativeMap & time_derivative_map);

  /// Initialise
  virtual void Init(Moose::MFEM::GridFunctions & gridfunctions,
                    mfem::AssemblyLevel assembly_level) override;

  virtual void SetTimeStep(mfem::real_t dt);
  virtual void UpdateEquationSystem(Moose::MFEM::GridFunctions & gridfunctions,
                                    mfem::Array<int> & btoffsets);

  virtual void AddKernel(std::shared_ptr<MFEMKernel> kernel) override;
  virtual void BuildBilinearForms() override;
  virtual void BuildMixedBilinearForms() override;
  virtual void ApplyEssentialBCs() override;
  virtual void EliminateCoupledVariables() override;
  virtual void FormLegacySystem(mfem::OperatorHandle & op,
                                mfem::BlockVector & truedXdt,
                                mfem::BlockVector & trueRHS) override;
  virtual void FormSystem(mfem::OperatorHandle & op,
                          mfem::BlockVector & truedXdt,
                          mfem::BlockVector & trueRHS) override;

  /// Fetch all integrators on a source bilinear form, scale them by a real factor, and add to a second target bilienar form.
  /// Useful for scaling bilinear form integrators by timesteps.
  template <class FormType>
  void ScaleAndAddBLFIntegrators(std::shared_ptr<FormType> source_blf,
                                 std::shared_ptr<FormType> target_blf,
                                 mfem::real_t scale_factor)
  {
    // Add and scale all domain integrators existing on source_blf to target_blf
    auto domain_integrators = source_blf->GetDBFI();
    auto domain_markers = source_blf->GetDBFI_Marker();
    for (int i = 0; i < domain_integrators->Size(); ++i)
      target_blf->AddDomainIntegrator(
          new ScaleIntegrator(*domain_integrators[i], scale_factor, false), *(*domain_markers[i]));
    // Add and scale all boundary integrators existing on source_blf to target_blf
    auto boundary_integrators = source_blf->GetBBFI();
    auto boundary_markers = source_blf->GetBBFI_Marker();
    for (int i = 0; i < boundary_integrators->Size(); ++i)
      target_blf->AddBoundaryIntegrator(
          new ScaleIntegrator(*boundary_integrators[i], scale_factor, false),
          *(*boundary_markers[i]));
  }

protected:
  /// Coefficient for timestep scaling
  mfem::ConstantCoefficient _dt_coef;

  Moose::MFEM::NamedFieldsMap<Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>>
      _td_kernels_map;
  /// Containers to store contributions to weak form of the form (F du/dt, v)
  Moose::MFEM::NamedFieldsMap<mfem::ParBilinearForm> _td_blfs;
  Moose::MFEM::NamedFieldsMap<Moose::MFEM::NamedFieldsMap<mfem::ParMixedBilinearForm>>
      _td_mblfs; // named according to trial variable

  /// Gridfunctions holding essential constraints from Dirichlet BCs
  std::vector<std::unique_ptr<mfem::ParGridFunction>> _td_var_ess_constraints;

  /// Map between variable names and their time derivatives
  const Moose::MFEM::TimeDerivativeMap & _time_derivative_map;

private:
  /// Set trial variable names from subset of coupled variables that have an associated test variable.
  virtual void SetTrialVariableNames() override;
};

} // namespace Moose::MFEM

#endif
