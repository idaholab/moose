#pragma once
#include "../common/pfem_extras.hpp"
#include "boundary_conditions.h"
#include "coefficients.h"
#include "MFEMContainers.h"
#include "MFEMKernel.h"

namespace platypus
{

/*
Class to store weak form components (bilinear and linear forms, and optionally
mixed and nonlinear forms) and build methods
*/
class EquationSystem : public mfem::Operator
{
public:
  using MFEMBilinearFormKernel = MFEMKernel<mfem::BilinearFormIntegrator>;
  using MFEMLinearFormKernel = MFEMKernel<mfem::LinearFormIntegrator>;
  using MFEMNonlinearFormKernel = MFEMKernel<mfem::NonlinearFormIntegrator>;
  using MFEMMixedBilinearFormKernel = MFEMKernel<mfem::BilinearFormIntegrator>;

  EquationSystem() = default;
  ~EquationSystem() override;

  // Test variables are associated with LinearForms,
  // whereas trial variables are associated with gridfunctions.

  // Names of all variables corresponding to gridfunctions. This may differ
  // from test_var_names when time derivatives are present.
  std::vector<std::string> _trial_var_names;
  // Pointers to trial variables.
  platypus::GridFunctions _trial_variables;
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
  virtual void AddKernel(const std::string & test_var_name,
                         std::shared_ptr<MFEMBilinearFormKernel> blf_kernel);

  void AddKernel(const std::string & test_var_name,
                 std::shared_ptr<MFEMLinearFormKernel> lf_kernel);

  void AddKernel(const std::string & test_var_name,
                 std::shared_ptr<MFEMNonlinearFormKernel> nlf_kernel);

  void AddKernel(const std::string & trial_var_name,
                 const std::string & test_var_name,
                 std::shared_ptr<MFEMMixedBilinearFormKernel> mblf_kernel);

  virtual void ApplyBoundaryConditions(platypus::BCMap & bc_map);

  // Build forms
  virtual void Init(platypus::GridFunctions & gridfunctions,
                    const platypus::FESpaces & fespaces,
                    platypus::BCMap & bc_map,
                    mfem::AssemblyLevel assembly_level);
  virtual void BuildLinearForms(platypus::BCMap & bc_map);
  virtual void BuildBilinearForms();
  virtual void BuildMixedBilinearForms();
  virtual void BuildEquationSystem(platypus::BCMap & bc_map);

  // Form linear system, with essential boundary conditions accounted for
  virtual void FormLinearSystem(mfem::OperatorHandle & op,
                                mfem::BlockVector & trueX,
                                mfem::BlockVector & trueRHS);

  virtual void FormSystem(mfem::OperatorHandle & op,
                                mfem::BlockVector & trueX,
                                mfem::BlockVector & trueRHS);
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
  virtual void RecoverFEMSolution(mfem::BlockVector & trueX,
                                  platypus::GridFunctions & gridfunctions);

  std::vector<mfem::Array<int>> _ess_tdof_lists;

  /**
   * Template method for storing kernels.
   */
  template <class T>
  void addKernelToMap(std::shared_ptr<T> kernel,
                      platypus::NamedFieldsMap<std::vector<std::shared_ptr<T>>> & kernels_map)
  {
    auto test_var_name = kernel->getTestVariableName();
    if (!kernels_map.Has(test_var_name))
    {
      // 1. Create kernels vector.
      auto kernels = std::make_shared<std::vector<std::shared_ptr<T>>>();
      // 2. Register with map to prevent leaks.
      kernels_map.Register(test_var_name, std::move(kernels));
    }
    kernels_map.GetRef(test_var_name).push_back(std::move(kernel));
  }

protected:
  bool VectorContainsName(const std::vector<std::string> & the_vector,
                          const std::string & name) const;

  // gridfunctions for setting Dirichlet BCs
  std::vector<std::unique_ptr<mfem::ParGridFunction>> _xs;
  std::vector<std::unique_ptr<mfem::ParGridFunction>> _dxdts;

  mfem::Array2D<mfem::HypreParMatrix *> _h_blocks;

  // Arrays to store kernels to act on each component of weak form. Named
  // according to test variable
  platypus::NamedFieldsMap<std::vector<std::shared_ptr<MFEMBilinearFormKernel>>> _blf_kernels_map;

  platypus::NamedFieldsMap<std::vector<std::shared_ptr<MFEMLinearFormKernel>>> _lf_kernels_map;

  platypus::NamedFieldsMap<std::vector<std::shared_ptr<MFEMNonlinearFormKernel>>> _nlf_kernels_map;

  platypus::NamedFieldsMap<
      platypus::NamedFieldsMap<std::vector<std::shared_ptr<MFEMMixedBilinearFormKernel>>>>
      _mblf_kernels_map_map;

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
  ~TimeDependentEquationSystem() override = default;

  void AddTrialVariableNameIfMissing(const std::string & trial_var_name) override;

  virtual void SetTimeStep(double dt);
  virtual void UpdateEquationSystem(platypus::BCMap & bc_map);
  mfem::ConstantCoefficient _dt_coef; // Coefficient for timestep scaling
  std::vector<std::string> _trial_var_time_derivative_names;

  platypus::NamedFieldsMap<std::vector<std::shared_ptr<MFEMBilinearFormKernel>>>
      _td_blf_kernels_map;
  // Container to store contributions to weak form of the form (F du/dt, v)
  platypus::NamedFieldsMap<mfem::ParBilinearForm> _td_blfs;

  virtual void AddKernel(const std::string & test_var_name,
                         std::shared_ptr<MFEMBilinearFormKernel> blf_kernel) override;
  virtual void BuildBilinearForms() override;
  virtual void FormLegacySystem(mfem::OperatorHandle & op,
                                mfem::BlockVector & truedXdt,
                                mfem::BlockVector & trueRHS) override;
};

} // namespace platypus
