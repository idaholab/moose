#pragma once
#include "../common/pfem_extras.hpp"
#include "inputs.h"
#include "kernel_base.h"
#include "named_fields_map.h"

namespace platypus
{

/*
Class to store weak form components (bilinear and linear forms, and optionally
mixed and nonlinear forms) and build methods
*/
class EquationSystem : public mfem::Operator
{
public:
  using ParBilinearFormKernel = platypus::Kernel<mfem::ParBilinearForm>;
  using ParLinearFormKernel = platypus::Kernel<mfem::ParLinearForm>;
  using ParNonlinearFormKernel = platypus::Kernel<mfem::ParNonlinearForm>;
  using ParMixedBilinearFormKernel = platypus::Kernel<mfem::ParMixedBilinearForm>;

  EquationSystem() = default;
  ~EquationSystem() override;

  // Test variables are associated with LinearForms,
  // whereas trial variables are associated with gridfunctions.

  // Names of all variables corresponding to gridfunctions. This may differ
  // from test_var_names when time derivatives are present.
  std::vector<std::string> _trial_var_names;
  // Names of all test variables corresponding to linear forms in this equation
  // system
  std::vector<std::string> _test_var_names;
  std::vector<mfem::ParFiniteElementSpace *> _test_pfespaces;

  // Components of weak form. // Named according to test variable
  platypus::NamedFieldsMap<mfem::ParBilinearForm> _blfs;
  platypus::NamedFieldsMap<mfem::ParLinearForm> _lfs;
  platypus::NamedFieldsMap<mfem::ParNonlinearForm> _nlfs;
  platypus::NamedFieldsMap<platypus::NamedFieldsMap<mfem::ParMixedBilinearForm>>
      _mblfs; // named according to trial variable

  // add test variable to EquationSystem;
  virtual void AddTestVariableNameIfMissing(const std::string & test_var_name);
  virtual void AddTrialVariableNameIfMissing(const std::string & trial_var_name);

  // Add kernels.
  void AddKernel(const std::string & test_var_name,
                 std::shared_ptr<ParBilinearFormKernel> blf_kernel);

  void AddKernel(const std::string & test_var_name, std::shared_ptr<ParLinearFormKernel> lf_kernel);

  void AddKernel(const std::string & test_var_name,
                 std::shared_ptr<ParNonlinearFormKernel> nlf_kernel);

  void AddKernel(const std::string & trial_var_name,
                 const std::string & test_var_name,
                 std::shared_ptr<ParMixedBilinearFormKernel> mblf_kernel);

  virtual void ApplyBoundaryConditions(platypus::BCMap & bc_map);

  // override to add kernels
  virtual void AddKernels() {}

  // Build forms
  virtual void Init(platypus::GridFunctions & gridfunctions,
                    const platypus::FESpaces & fespaces,
                    platypus::BCMap & bc_map,
                    platypus::Coefficients & coefficients);
  virtual void BuildLinearForms(platypus::BCMap & bc_map);
  virtual void BuildBilinearForms();
  virtual void BuildMixedBilinearForms();
  virtual void BuildEquationSystem(platypus::BCMap & bc_map);

  // Form linear system, with essential boundary conditions accounted for
  virtual void FormLinearSystem(mfem::OperatorHandle & op,
                                mfem::BlockVector & trueX,
                                mfem::BlockVector & trueRHS);

  // Build linear system, with essential boundary conditions accounted for
  virtual void BuildJacobian(mfem::BlockVector & trueX, mfem::BlockVector & trueRHS);

  /// Compute residual y = Mu
  void Mult(const mfem::Vector & u, mfem::Vector & residual) const override;

  /// Compute J = M + grad_H(u)
  mfem::Operator & GetGradient(const mfem::Vector & u) const override;

  // Update variable from solution vector after solve
  virtual void RecoverFEMSolution(mfem::BlockVector & trueX,
                                  platypus::GridFunctions & gridfunctions);

  std::vector<mfem::Array<int>> _ess_tdof_lists;

protected:
  bool VectorContainsName(const std::vector<std::string> & the_vector,
                          const std::string & name) const;

  // gridfunctions for setting Dirichlet BCs
  std::vector<std::unique_ptr<mfem::ParGridFunction>> _xs;

  mfem::Array2D<mfem::HypreParMatrix *> _h_blocks;

  // Arrays to store kernels to act on each component of weak form. Named
  // according to test variable
  platypus::NamedFieldsMap<std::vector<std::shared_ptr<ParBilinearFormKernel>>> _blf_kernels_map;

  platypus::NamedFieldsMap<std::vector<std::shared_ptr<ParLinearFormKernel>>> _lf_kernels_map;

  platypus::NamedFieldsMap<std::vector<std::shared_ptr<ParNonlinearFormKernel>>> _nlf_kernels_map;

  platypus::NamedFieldsMap<
      platypus::NamedFieldsMap<std::vector<std::shared_ptr<ParMixedBilinearFormKernel>>>>
      _mblf_kernels_map_map;

  mutable mfem::OperatorHandle _jacobian;
};

/*
Class to store weak form components for time dependent PDEs
*/
class TimeDependentEquationSystem : public EquationSystem
{
public:
  TimeDependentEquationSystem();
  ~TimeDependentEquationSystem() override = default;

  static std::string GetTimeDerivativeName(std::string name)
  {
    return std::string("d") + name + std::string("_dt");
  }

  void AddTrialVariableNameIfMissing(const std::string & trial_var_name) override;

  virtual void SetTimeStep(double dt);
  virtual void UpdateEquationSystem(platypus::BCMap & bc_map);
  mfem::ConstantCoefficient _dt_coef; // Coefficient for timestep scaling
  std::vector<std::string> _trial_var_time_derivative_names;
};

} // namespace platypus
