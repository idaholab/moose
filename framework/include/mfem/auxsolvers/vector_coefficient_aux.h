#pragma once
#include "coefficient_aux.h"

namespace platypus
{

// Project a stored vector Coefficient onto a (vector) GridFunction
class VectorCoefficientAux : public AuxSolver
{
public:
  VectorCoefficientAux(std::string gf_name,
                       std::string vec_coef_name,
                       platypus::InputParameters solver_options = platypus::InputParameters());

  ~VectorCoefficientAux() override = default;

  void Init(const platypus::GridFunctions & gridfunctions,
            platypus::Coefficients & coefficients) override;

  virtual void BuildBilinearForm();
  virtual void BuildLinearForm();
  void Solve(double t = 0.0) override;

protected:
  const std::string _gf_name;       // name of the variable
  const std::string _vec_coef_name; // name of the vector coefficient

  mfem::ParGridFunction * _gf{nullptr};
  mfem::VectorCoefficient * _vec_coef{nullptr};

  // Pointer to store test FE space. Assumed to be same as trial FE space.
  mfem::ParFiniteElementSpace * _test_fes{nullptr};

  // Bilinear and linear forms
  std::unique_ptr<mfem::ParBilinearForm> _a{nullptr};
  std::unique_ptr<mfem::ParLinearForm> _b{nullptr};

private:
  const platypus::InputParameters _solver_options;

  // Operator matrices
  std::unique_ptr<mfem::HypreParMatrix> _a_mat{nullptr};

  // Solver
  std::unique_ptr<platypus::DefaultJacobiPCGSolver> _solver{nullptr};
};

} // namespace platypus
