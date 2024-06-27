#pragma once
#include "../common/pfem_extras.hpp"
#include "formulation.h"
#include "inputs.h"
#include "sources.h"

namespace platypus
{

class HCurlFormulation : public TimeDomainEMFormulation
{
public:
  HCurlFormulation(std::string alpha_coef_name,
                   std::string beta_coef_name,
                   std::string h_curl_var_name);

  ~HCurlFormulation() override = default;

  void ConstructOperator() override;

  void ConstructJacobianPreconditioner() override;

  void ConstructJacobianSolver() override;

  void RegisterGridFunctions() override;

  void RegisterCoefficients() override;

protected:
  const std::string _alpha_coef_name;
  const std::string _beta_coef_name;
  const std::string _h_curl_var_name;
};

class CurlCurlEquationSystem : public TimeDependentEquationSystem
{
public:
  CurlCurlEquationSystem(const platypus::InputParameters & params);

  void Init(platypus::GridFunctions & gridfunctions,
            const platypus::FESpaces & fespaces,
            platypus::BCMap & bc_map,
            Coefficients & coefficients) override;
  void AddKernels() override;

  std::string _h_curl_var_name, _alpha_coef_name, _beta_coef_name, _dtalpha_coef_name;
};

} // namespace platypus
