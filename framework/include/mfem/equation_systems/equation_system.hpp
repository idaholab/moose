#pragma once
#include "../common/pfem_extras.hpp"
#include "inputs.hpp"
#include "kernel_base.hpp"
#include "named_fields_map.hpp"
#include "sources.hpp"

namespace hephaestus
{

/*
Class to store weak form components (bilinear and linear forms, and optionally
mixed and nonlinear forms) and build methods
*/
class EquationSystem : public mfem::Operator
{
public:
  using ParBilinearFormKernel = hephaestus::Kernel<mfem::ParBilinearForm>;
  using ParLinearFormKernel = hephaestus::Kernel<mfem::ParLinearForm>;
  using ParNonlinearFormKernel = hephaestus::Kernel<mfem::ParNonlinearForm>;
  using ParMixedBilinearFormKernel = hephaestus::Kernel<mfem::ParMixedBilinearForm>;

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
  hephaestus::NamedFieldsMap<mfem::ParBilinearForm> _blfs;
  hephaestus::NamedFieldsMap<mfem::ParLinearForm> _lfs;
  hephaestus::NamedFieldsMap<mfem::ParNonlinearForm> _nlfs;
  hephaestus::NamedFieldsMap<hephaestus::NamedFieldsMap<mfem::ParMixedBilinearForm>>
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

  virtual void ApplyBoundaryConditions(hephaestus::BCMap & bc_map);

  // override to add kernels
  virtual void AddKernels() {}

  // Build forms
  virtual void Init(hephaestus::GridFunctions & gridfunctions,
                    const hephaestus::FESpaces & fespaces,
                    hephaestus::BCMap & bc_map,
                    hephaestus::Coefficients & coefficients);
  virtual void BuildLinearForms(hephaestus::BCMap & bc_map, hephaestus::Sources & sources);
  virtual void BuildBilinearForms();
  virtual void BuildMixedBilinearForms();
  virtual void BuildEquationSystem(hephaestus::BCMap & bc_map, hephaestus::Sources & sources);

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
                                  hephaestus::GridFunctions & gridfunctions);

  std::vector<mfem::Array<int>> _ess_tdof_lists;

protected:
  bool VectorContainsName(const std::vector<std::string> & the_vector,
                          const std::string & name) const;

  // gridfunctions for setting Dirichlet BCs
  std::vector<std::unique_ptr<mfem::ParGridFunction>> _xs;

  mfem::Array2D<mfem::HypreParMatrix *> _h_blocks;

  // Arrays to store kernels to act on each component of weak form. Named
  // according to test variable
  hephaestus::NamedFieldsMap<std::vector<std::shared_ptr<ParBilinearFormKernel>>> _blf_kernels_map;

  hephaestus::NamedFieldsMap<std::vector<std::shared_ptr<ParLinearFormKernel>>> _lf_kernels_map;

  hephaestus::NamedFieldsMap<std::vector<std::shared_ptr<ParNonlinearFormKernel>>> _nlf_kernels_map;

  hephaestus::NamedFieldsMap<
      hephaestus::NamedFieldsMap<std::vector<std::shared_ptr<ParMixedBilinearFormKernel>>>>
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
  virtual void UpdateEquationSystem(hephaestus::BCMap & bc_map, hephaestus::Sources & sources);
  mfem::ConstantCoefficient _dt_coef; // Coefficient for timestep scaling
  std::vector<std::string> _trial_var_time_derivative_names;
};

} // namespace hephaestus
