#pragma once
#include "../common/pfem_extras.hpp"
#include "formulation.hpp"
#include "inputs.hpp"
#include "sources.hpp"

namespace hephaestus
{

class StaticsFormulation : public SteadyStateEMFormulation
{
public:
  StaticsFormulation(std::string alpha_coef_name, std::string h_curl_var_name);

  ~StaticsFormulation() override = default;

  void ConstructJacobianPreconditioner() override;

  void ConstructJacobianSolver() override;

  void ConstructOperator() override;

  void RegisterGridFunctions() override;

  void RegisterCoefficients() override;

protected:
  const std::string _alpha_coef_name;
  const std::string _h_curl_var_name;
};

class StaticsOperator : public ProblemOperator
{
public:
  StaticsOperator(hephaestus::Problem & problem,
                  std::string h_curl_var_name,
                  std::string stiffness_coef_name);

  ~StaticsOperator() override = default;

  void SetGridFunctions() override;
  void Init(mfem::Vector & X) override;
  void Solve(mfem::Vector & X) override;

private:
  std::string _h_curl_var_name, _stiffness_coef_name;

  mfem::Coefficient * _stiff_coef{nullptr}; // Stiffness Material Coefficient
};

} // namespace hephaestus
