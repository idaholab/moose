#pragma once
#include "auxsolver_base.hpp"

namespace hephaestus
{

// Scale a gridfunction in H(Curl) or H(Div) by a scalar Coefficient, and store
// the result. Suitable for solving for H(Div) or H(Curl) conforming fields for
// expressions like v = a*σ*u
class ScaledVectorGridFunctionAux : public AuxSolver
{
public:
  ScaledVectorGridFunctionAux(
      std::string input_gf_name,
      std::string scaled_gf_name,
      std::string coef_name,
      const double & aConst = 1.0,
      const double & bConst = 1.0,
      std::string shift_gf_name = "",
      hephaestus::InputParameters solver_options = hephaestus::InputParameters());

  ~ScaledVectorGridFunctionAux() override = default;

  void Init(const hephaestus::GridFunctions & gridfunctions,
            hephaestus::Coefficients & coefficients) override;
  virtual void BuildBilinearForm();
  virtual void BuildMixedBilinearForm();
  void Solve(double t = 0.0) override;

protected:
  // Pointers to store trial and test FE spaces
  mfem::ParFiniteElementSpace * _trial_fes{nullptr};
  mfem::ParFiniteElementSpace * _test_fes{nullptr};

  // Bilinear forms
  std::unique_ptr<mfem::ParBilinearForm> _a{nullptr};
  std::unique_ptr<mfem::ParMixedBilinearForm> _a_mixed{nullptr};

  // Coefficient to scale input gridfunction by
  mfem::Coefficient * _coef{nullptr};
  // Optional constant to scale input gridfunction by

private:
  const std::string _input_gf_name;
  const std::string _scaled_gf_name;
  const std::string _shift_gf_name;
  const std::string _coef_name;
  const double _a_const;
  const double _b_const;
  const hephaestus::InputParameters _solver_options;

  // Input gridfunction to be scaled by a scalar coefficient
  mfem::ParGridFunction * _input_gf{nullptr};

  // Gridfunction in which to store result
  mfem::ParGridFunction * _scaled_gf{nullptr};

  // Optional gridfunction in which to shift result by
  mfem::ParGridFunction * _shift_gf{nullptr};

  // Operator matrices
  std::unique_ptr<mfem::HypreParMatrix> _a_mat{nullptr};
  std::unique_ptr<mfem::HypreParMatrix> _mixed_mat{nullptr};

  // Solver
  std::unique_ptr<hephaestus::DefaultJacobiPCGSolver> _solver{nullptr};
};
} // namespace hephaestus
