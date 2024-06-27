#include "l2_error_vector_aux.hpp"

namespace hephaestus
{

L2ErrorVectorPostprocessor::L2ErrorVectorPostprocessor(const hephaestus::InputParameters & params)
  : _var_name(params.GetParam<std::string>("VariableName")),
    _vec_coef_name(params.GetParam<std::string>("VectorCoefficientName"))
{
}

void
L2ErrorVectorPostprocessor::Init(const hephaestus::GridFunctions & gridfunctions,
                                 hephaestus::Coefficients & coefficients)
{
  _gf = gridfunctions.Get(_var_name);
  _vec_coeff = coefficients._vectors.Get(_vec_coef_name);
}

void
L2ErrorVectorPostprocessor::Solve(double t)
{
  double l2_err = _gf->ComputeL2Error(*_vec_coeff);
  HYPRE_BigInt ndof = _gf->ParFESpace()->GlobalTrueVSize();

  _times.Append(t);
  _l2_errs.Append(l2_err);
  _ndofs.Append(ndof);
}

} // namespace hephaestus
