#pragma once
#include "auxsolver_base.h"

namespace platypus
{

// Calculate the curl of a gridfunction.
class CurlAuxSolver : public AuxSolver
{
public:
  CurlAuxSolver(std::string input_gf_name, std::string curl_gf_name);

  ~CurlAuxSolver() override = default;

  void Init(const platypus::GridFunctions & gridfunctions, Coefficients & coefficients) override;

  void Solve(double t = 0.0) override;

private:
  const std::string _input_gf_name; // name of the variable
  const std::string _curl_gf_name;  // Variable in which to store curl

  mfem::ParGridFunction * _u{nullptr};
  mfem::ParGridFunction * _curl_u{nullptr};

  std::unique_ptr<mfem::ParDiscreteLinearOperator> _curl{nullptr};
};

} // namespace platypus
