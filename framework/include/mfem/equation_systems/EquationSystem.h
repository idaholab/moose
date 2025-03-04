#ifdef MFEM_ENABLED

#pragma once
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/pfem_extras.hpp"
#include "libmesh/restore_warnings.h"
#include "MFEMBoundaryConditionUtils.h"
#include "MFEMContainers.h"
#include "MFEMKernel.h"
#include "MFEMMixedBilinearFormKernel.h"
#include "ScaleIntegrator.h"

namespace Moose::MFEM
{

/*
Class to store weak form components (bilinear and linear forms, and optionally
mixed and nonlinear forms) and build methods
*/
class EquationSystem : public mfem::Operator
{
public:
  EquationSystem() = default;
  ~EquationSystem() override;

  // add test variable to EquationSystem;
  virtual void AddTestVariableNameIfMissing(const std::string & test_var_name);
  virtual void AddTrialVariableNameIfMissing(const std::string & trial_var_name);

  // Add kernels.
  virtual void AddKernel(std::shared_ptr<MFEMKernel> kernel);

  virtual void ApplyBoundaryConditions(Moose::MFEM::BCMap & bc_map);

  // Build forms
  virtual void Init(Moose::MFEM::GridFunctions & gridfunctions,
                    const Moose::MFEM::FESpaces & fespaces,
                    Moose::MFEM::BCMap & bc_map,
                    mfem::AssemblyLevel assembly_level);
  virtual void BuildLinearForms(Moose::MFEM::BCMap & bc_map);
  virtual void BuildBilinearForms(Moose::MFEM::BCMap & bc_map);
  virtual void BuildMixedBilinearForms();
  virtual void BuildEquationSystem(Moose::MFEM::BCMap & bc_map);

  // Form linear system, with essential boundary conditions accounted for
  virtual void FormLinearSystem(mfem::OperatorHandle & op,
                                mfem::BlockVector & trueX,
                                mfem::BlockVector & trueRHS);

  virtual void
  FormSystem(mfem::OperatorHandle & op, mfem::BlockVector & trueX, mfem::BlockVector & trueRHS);
  virtual void FormLegacySystem(mfem::OperatorHandle & op,
                                mfem::BlockVector & trueX,
                                mfem::BlockVector & trueRHS);

  // Build linear system, with essential boundary conditions accounted for
  virtual void BuildJacobian(mfem::BlockVector & trueX, mfem::BlockVector & trueRHS);

  /// Compute residual y = Mu
  void Mult(const mfem::Vector & u, mfem::Vector & residual) const override;

  /// Compute J = M + grad_H(u)
  mfem::Operator & GetGradient(const mfem::Vector & u) const override;

  // Update variable from solution vector after solve
  using mfem::Operator::RecoverFEMSolution;
  virtual void RecoverFEMSolution(mfem::BlockVector & trueX,
                                  Moose::MFEM::GridFunctions & gridfunctions);

  std::vector<mfem::Array<int>> _ess_tdof_lists;

  const std::vector<std::string> & TrialVarNames() const { return _trial_var_names; }
  const std::vector<std::string> & TestVarNames() const { return _test_var_names; }

protected:
  bool VectorContainsName(const std::vector<std::string> & the_vector,
                          const std::string & name) const;

  // Test variables are associated with LinearForms,
  // whereas trial variables are associated with gridfunctions.

  // Names of all variables corresponding to gridfunctions. This may differ
  // from test_var_names when time derivatives are present.
  std::vector<std::string> _trial_var_names;
  // Pointers to trial variables.
  Moose::MFEM::GridFunctions _trial_variables;
  // Names of all test variables corresponding to linear forms in this equation
  // system
  std::vector<std::string> _test_var_names;
  std::vector<mfem::ParFiniteElementSpace *> _test_pfespaces;

  // Components of weak form. // Named according to test variable
  Moose::MFEM::NamedFieldsMap<mfem::ParBilinearForm> _blfs;
  Moose::MFEM::NamedFieldsMap<mfem::ParLinearForm> _lfs;
  Moose::MFEM::NamedFieldsMap<mfem::ParNonlinearForm> _nlfs;
  Moose::MFEM::NamedFieldsMap<Moose::MFEM::NamedFieldsMap<mfem::ParMixedBilinearForm>>
      _mblfs; // named according to trial variable

  // gridfunctions for setting Dirichlet BCs
  std::vector<std::unique_ptr<mfem::ParGridFunction>> _xs;
  std::vector<std::unique_ptr<mfem::ParGridFunction>> _dxdts;

  mfem::Array2D<const mfem::HypreParMatrix *> _h_blocks;

  // Arrays to store kernels to act on each component of weak form. Named
  // according to test variable
  Moose::MFEM::NamedFieldsMap<Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>>
      _kernels_map;

  mutable mfem::OperatorHandle _jacobian;

  mfem::AssemblyLevel _assembly_level;
};

/*
Class to store weak form components for time dependent PDEs
*/
class TimeDependentEquationSystem : public EquationSystem
{
public:
  TimeDependentEquationSystem();

  void AddTrialVariableNameIfMissing(const std::string & trial_var_name) override;

  virtual void SetTimeStep(double dt);
  virtual void UpdateEquationSystem(Moose::MFEM::BCMap & bc_map);

  virtual void AddKernel(std::shared_ptr<MFEMKernel> kernel) override;
  virtual void BuildBilinearForms(Moose::MFEM::BCMap & bc_map) override;
  virtual void FormLegacySystem(mfem::OperatorHandle & op,
                                mfem::BlockVector & truedXdt,
                                mfem::BlockVector & trueRHS) override;
  virtual void FormSystem(mfem::OperatorHandle & op,
                          mfem::BlockVector & truedXdt,
                          mfem::BlockVector & trueRHS) override;

  const std::vector<std::string> & TrialVarTimeDerivativeNames() const;

protected:
  mfem::ConstantCoefficient _dt_coef; // Coefficient for timestep scaling
  std::vector<std::string> _trial_var_time_derivative_names;

  Moose::MFEM::NamedFieldsMap<Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>>
      _td_kernels_map;
  // Container to store contributions to weak form of the form (F du/dt, v)
  Moose::MFEM::NamedFieldsMap<mfem::ParBilinearForm> _td_blfs;
};

inline const std::vector<std::string> &
TimeDependentEquationSystem::TrialVarTimeDerivativeNames() const
{
  return _trial_var_time_derivative_names;
}

} // namespace Moose::MFEM

#endif
