#pragma once
#include "../common/pfem_extras.hpp"
#include "formulation.hpp"
#include "inputs.hpp"

namespace hephaestus
{

class DualFormulation : public TimeDomainEMFormulation
{
public:
  DualFormulation(std::string alpha_coef_name,
                  std::string beta_coef_name,
                  std::string h_curl_var_name,
                  std::string h_div_var_name);

  ~DualFormulation() override = default;

  void ConstructJacobianPreconditioner() override;

  void ConstructJacobianSolver() override;

  void ConstructOperator() override;

  void RegisterGridFunctions() override;

  void RegisterCoefficients() override;

protected:
  const std::string _alpha_coef_name;
  const std::string _beta_coef_name;
  const std::string _h_curl_var_name;
  const std::string _h_div_var_name;
};

class WeakCurlEquationSystem : public TimeDependentEquationSystem
{
public:
  WeakCurlEquationSystem(const hephaestus::InputParameters & params);
  ~WeakCurlEquationSystem() override = default;

  void Init(hephaestus::GridFunctions & gridfunctions,
            const hephaestus::FESpaces & fespaces,
            hephaestus::BCMap & bc_map,
            hephaestus::Coefficients & coefficients) override;
  void AddKernels() override;

  std::string _h_curl_var_name, _h_div_var_name, _alpha_coef_name, _beta_coef_name,
      _dtalpha_coef_name;
};

class DualOperator : public TimeDomainEquationSystemProblemOperator
{
public:
  DualOperator(hephaestus::Problem & problem,
               std::unique_ptr<TimeDependentEquationSystem> equation_system)
    : TimeDomainEquationSystemProblemOperator(problem, std::move(equation_system))
  {
  }

  void Init(mfem::Vector & X) override;

  void ImplicitSolve(const double dt, const mfem::Vector & X, mfem::Vector & dX_dt) override;
  void SetGridFunctions() override;

  mfem::ParFiniteElementSpace * _h_curl_fe_space{nullptr};
  mfem::ParFiniteElementSpace * _h_div_fe_space{nullptr};

  std::string _h_curl_var_name, _h_div_var_name;

  mfem::ParGridFunction * _u{nullptr};  // HCurl vector field
  mfem::ParGridFunction * _dv{nullptr}; // HDiv vector field

protected:
  std::unique_ptr<mfem::ParDiscreteLinearOperator> _curl;
};
} // namespace hephaestus
