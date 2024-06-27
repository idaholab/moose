#pragma once
#include "auxsolver_base.h"

// Specify postprocessors that depend on one or more gridfunctions
namespace platypus
{

// Class to calculate and store the L2 error
// of a grid function with respect to a (Vector)Coefficient
class L2ErrorVectorPostprocessor : public AuxSolver
{

public:
  L2ErrorVectorPostprocessor() = default;
  L2ErrorVectorPostprocessor(const platypus::InputParameters & params);

  ~L2ErrorVectorPostprocessor() override = default;

  void Init(const platypus::GridFunctions & gridfunctions,
            platypus::Coefficients & coefficients) override;

  void Solve(double t = 0.0) override;

  std::string _var_name;      // name of the variable
  std::string _vec_coef_name; // name of the vector coefficient

  mfem::Array<double> _times;
  mfem::Array<HYPRE_BigInt> _ndofs;
  mfem::Array<double> _l2_errs;

  mfem::ParGridFunction * _gf{nullptr};
  mfem::VectorCoefficient * _vec_coeff{nullptr};
};

} // namespace platypus
