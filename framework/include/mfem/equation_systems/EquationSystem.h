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
  EquationSystem() = default;
  ~EquationSystem() override;

  /// Add kernels.
  virtual void AddKernel(std::shared_ptr<MFEMKernel> kernel);
  virtual void AddIntegratedBC(std::shared_ptr<MFEMIntegratedBC> kernel);
  /// Add BC associated with essentially constrained DoFs on boundaries.
  virtual void AddEssentialBC(std::shared_ptr<MFEMEssentialBC> bc);

  /// Initialise
  virtual void Init(GridFunctions & gridfunctions,
                    ComplexGridFunctions & cmplx_gridfunctions,
                    mfem::AssemblyLevel assembly_level);
  /// Build linear system, with essential boundary conditions accounted for
  virtual void BuildJacobian(mfem::BlockVector & trueX, mfem::BlockVector & trueRHS);
  /// Compute residual y = Mu
  void Mult(const mfem::Vector & u, mfem::Vector & residual) const override;
  /// Compute J = M + grad_H(u)
  mfem::Operator & GetGradient(const mfem::Vector & u) const override;

  /// Update variable from solution vector after solve
  virtual void RecoverFEMSolution(mfem::BlockVector & trueX,
                                  GridFunctions & gridfunctions,
                                  ComplexGridFunctions & cmplx_gridfunctions);

  // Test variables are associated with linear forms,
  // whereas trial variables are associated with gridfunctions.
  const std::vector<std::string> & GetTrialVarNames() const { return _trial_var_names; }
  const std::vector<std::string> & GetTestVarNames() const { return _test_var_names; }

protected:
  /// Add coupled variable to EquationSystem.
  virtual void AddCoupledVariableNameIfMissing(const std::string & coupled_var_name);
  /// Add eliminated variable to EquationSystem.
  virtual void AddEliminatedVariableNameIfMissing(const std::string & eliminated_var_name);
  /// Add test variable to EquationSystem.
  virtual void AddTestVariableNameIfMissing(const std::string & test_var_name);
  /// Set trial variable names from subset of coupled variables that have an associated test variable.
  virtual void SetTrialVariableNames();

  /// Deletes the HypreParMatrix associated with any pointer stored in _h_blocks,
  /// and then proceeds to delete all dynamically allocated memory for _h_blocks
  /// itself, resetting all dimensions to zero.
  void DeleteAllBlocks();

  bool VectorContainsName(const std::vector<std::string> & the_vector,
                          const std::string & name) const;

  /// Apply essential BC(s) associated with var_name to set true DoFs of trial_gf and update
  /// markers of all essential boundaries
  virtual void ApplyEssentialBC(const std::string & var_name,
                                mfem::ParGridFunction & trial_gf,
                                mfem::Array<int> & global_ess_markers);
  /// Update all essentially constrained true DoF markers and values on boundaries
  virtual void ApplyEssentialBCs();

  /// Perform trivial eliminations of coupled variables lacking corresponding test variables
  virtual void EliminateCoupledVariables();
  /// Build linear forms and eliminate constrained DoFs
  virtual void BuildLinearForms();
  /// Build bilinear forms (diagonal Jacobian contributions)
  virtual void BuildBilinearForms();
  /// Build mixed bilinear forms (off-diagonal Jacobian contributions)
  virtual void BuildMixedBilinearForms();
  /// Build all forms comprising this EquationSystem
  virtual void BuildEquationSystem();

  /// Form linear system and jacobian operator based on on- and off-diagonal bilinear form
  /// contributions, populate solution and RHS vectors of true DoFs, and apply constraints.
  virtual void FormLinearSystem(mfem::OperatorHandle & op,
                                mfem::BlockVector & trueX,
                                mfem::BlockVector & trueRHS);
  /// Form matrix-free representation of system operator.
  /// Used when EquationSystem assembly level is set to 'FULL', 'ELEMENT', 'PARTIAL', or 'NONE'.
  virtual void FormSystemOperator(mfem::OperatorHandle & op,
                                  mfem::BlockVector & trueX,
                                  mfem::BlockVector & trueRHS);
  /// Form matrix representation of system operator as a HypreParMatrix.
  /// Used when EquationSystem assembly level is set to 'LEGACY'.
  virtual void FormSystemMatrix(mfem::OperatorHandle & op,
                                mfem::BlockVector & trueX,
                                mfem::BlockVector & trueRHS);

  /**
   * Template method for applying BilinearFormIntegrators on domains from kernels to a BilinearForm,
   * or MixedBilinearForm
   */
  template <class FormType>
  void ApplyDomainBLFIntegrators(
      const std::string & trial_var_name,
      const std::string & test_var_name,
      std::shared_ptr<FormType> form,
      NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>> & kernels_map,
      std::optional<mfem::real_t> scale_factor = std::nullopt);

  void ApplyDomainLFIntegrators(
      const std::string & test_var_name,
      std::shared_ptr<mfem::ParLinearForm> form,
      NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>> & kernels_map);

  template <class FormType>
  void ApplyBoundaryBLFIntegrators(
      const std::string & trial_var_name,
      const std::string & test_var_name,
      std::shared_ptr<FormType> form,
      NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMIntegratedBC>>>> &
          integrated_bc_map,
      std::optional<mfem::real_t> scale_factor = std::nullopt);

  void ApplyBoundaryLFIntegrators(
      const std::string & test_var_name,
      std::shared_ptr<mfem::ParLinearForm> form,
      NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMIntegratedBC>>>> &
          integrated_bc_map);

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

  // Components of weak form, named according to test variable
  NamedFieldsMap<mfem::ParBilinearForm> _blfs;
  NamedFieldsMap<mfem::ParLinearForm> _lfs;
  NamedFieldsMap<mfem::ParNonlinearForm> _nlfs;
  NamedFieldsMap<NamedFieldsMap<mfem::ParMixedBilinearForm>>
      _mblfs; // named according to trial variable

  /// Gridfunctions holding essential constraints from Dirichlet BCs
  std::vector<std::unique_ptr<mfem::ParGridFunction>> _var_ess_constraints;
  std::vector<mfem::Array<int>> _ess_tdof_lists;

  mfem::Array2D<const mfem::HypreParMatrix *> _h_blocks;
  /// Arrays to store kernels to act on each component of weak form.
  /// Named according to test and trial variables.
  NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>> _kernels_map;
  /// Arrays to store integrated BCs to act on each component of weak form.
  /// Named according to test and trial variables.
  NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMIntegratedBC>>>> _integrated_bc_map;
  /// Arrays to store essential BCs to act on each component of weak form.
  /// Named according to test variable.
  NamedFieldsMap<std::vector<std::shared_ptr<MFEMEssentialBC>>> _essential_bc_map;

  mutable mfem::OperatorHandle _jacobian;

  mfem::AssemblyLevel _assembly_level;

private:
  friend class EquationSystemProblemOperator;
  /// Disallowed inherited method
  using mfem::Operator::RecoverFEMSolution;
};

template <class FormType>
void
EquationSystem::ApplyDomainBLFIntegrators(
    const std::string & trial_var_name,
    const std::string & test_var_name,
    std::shared_ptr<FormType> form,
    NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>> & kernels_map,
    std::optional<mfem::real_t> scale_factor)
{
  if (kernels_map.Has(test_var_name) && kernels_map.Get(test_var_name)->Has(trial_var_name))
  {
    auto kernels = kernels_map.GetRef(test_var_name).GetRef(trial_var_name);
    for (auto & kernel : kernels)
    {
      mfem::BilinearFormIntegrator * integ = kernel->createBFIntegrator();

      if (integ)
      {
        if (scale_factor.has_value())
          integ = new ScaleIntegrator(integ, scale_factor.value(), true);
        kernel->isSubdomainRestricted()
            ? form->AddDomainIntegrator(std::move(integ), kernel->getSubdomainMarkers())
            : form->AddDomainIntegrator(std::move(integ));
      }

      mfem::BilinearFormIntegrator * dg_integ = kernel->createFaceBFIntegrator();
      if (dg_integ)
      {
        // AddInteriorFaceIntegrator doesn't have the overload for passing in the
        // boundary markers as well
        form->AddInteriorFaceIntegrator(std::move(dg_integ));
      }
    }
  }
}

inline void
EquationSystem::ApplyDomainLFIntegrators(
    const std::string & test_var_name,
    std::shared_ptr<mfem::ParLinearForm> form,
    NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>> & kernels_map)
{
  if (kernels_map.Has(test_var_name) && kernels_map.Get(test_var_name)->Has(test_var_name))
  {
    auto kernels = kernels_map.GetRef(test_var_name).GetRef(test_var_name);
    for (auto & kernel : kernels)
    {
      mfem::LinearFormIntegrator * integ = kernel->createLFIntegrator();

      if (integ)
      {
        kernel->isSubdomainRestricted()
            ? form->AddDomainIntegrator(std::move(integ), kernel->getSubdomainMarkers())
            : form->AddDomainIntegrator(std::move(integ));
      }

      // Do the same with the DG stuff
      mfem::LinearFormIntegrator * dg_integ = kernel->createFaceLFIntegrator();
      if (dg_integ)
      {
        // AddInteriorFaceIntegrator doesn't have the overload for passing in the
        // boundary markers as well
        form->AddInteriorFaceIntegrator(std::move(dg_integ));
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
    NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMIntegratedBC>>>> &
        integrated_bc_map,
    std::optional<mfem::real_t> scale_factor)
{
  if (integrated_bc_map.Has(test_var_name) &&
      integrated_bc_map.Get(test_var_name)->Has(trial_var_name))
  {
    auto bcs = integrated_bc_map.GetRef(test_var_name).GetRef(trial_var_name);
    for (auto & bc : bcs)
    {
      mfem::BilinearFormIntegrator * integ = bc->createBFIntegrator();

      if (integ)
      {
        if (scale_factor.has_value())
          integ = new ScaleIntegrator(integ, scale_factor.value(), true);
        bc->isBoundaryRestricted()
            ? form->AddBoundaryIntegrator(std::move(integ), bc->getBoundaryMarkers())
            : form->AddBoundaryIntegrator(std::move(integ));
      }

      // Do the same with the DG stuff
      mfem::BilinearFormIntegrator * dg_integ = bc->createFaceBFIntegrator();
      if (dg_integ)
      {
        bc->isBoundaryRestricted()
            ? form->AddBdrFaceIntegrator(std::move(dg_integ), bc->getBoundaryMarkers())
            : form->AddBdrFaceIntegrator(std::move(dg_integ));
      }
    }
  }
}

inline void
EquationSystem::ApplyBoundaryLFIntegrators(
    const std::string & test_var_name,
    std::shared_ptr<mfem::ParLinearForm> form,
    NamedFieldsMap<NamedFieldsMap<std::vector<std::shared_ptr<MFEMIntegratedBC>>>> &
        integrated_bc_map)
{
  if (integrated_bc_map.Has(test_var_name) &&
      integrated_bc_map.Get(test_var_name)->Has(test_var_name))
  {
    auto bcs = integrated_bc_map.GetRef(test_var_name).GetRef(test_var_name);
    for (auto & bc : bcs)
    {
      mfem::LinearFormIntegrator * integ = bc->createLFIntegrator();

      if (integ)
      {
        bc->isBoundaryRestricted()
            ? form->AddBoundaryIntegrator(std::move(integ), bc->getBoundaryMarkers())
            : form->AddBoundaryIntegrator(std::move(integ));
      }

      // Do the same with the DG stuff
      mfem::LinearFormIntegrator * dg_integ = bc->createFaceLFIntegrator();
      if (dg_integ)
      {
        bc->isBoundaryRestricted()
            ? form->AddBdrFaceIntegrator(std::move(dg_integ), bc->getBoundaryMarkers())
            : form->AddBdrFaceIntegrator(std::move(dg_integ));
      }
    }
  }
}

} // namespace Moose::MFEM

#endif
