//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMatrixFreeAMS.h"
#include "MFEMProblem.h"

namespace
{
/// Forwards to a smoother owned elsewhere; mfem::MatrixFreeAMS deletes its smoother on destruction
class SmootherReference : public mfem::Solver
{
public:
  explicit SmootherReference(mfem::Solver & smoother)
    : mfem::Solver(smoother.Height(), smoother.Width()), _smoother(smoother)
  {
  }
  void SetOperator(const mfem::Operator & op) override { _smoother.SetOperator(op); }
  void Mult(const mfem::Vector & x, mfem::Vector & y) const override { _smoother.Mult(x, y); }

private:
  mfem::Solver & _smoother;
};
}

registerMooseObject("MooseApp", MFEMMatrixFreeAMS);

namespace Moose::MFEM
{
MatrixFreeAMS::MatrixFreeAMS(mfem::Coefficient & alpha_coef,
                             mfem::Coefficient & beta_coef,
                             mfem::Solver & smoother,
                             int inner_pi_its,
                             int inner_g_its)
  : _alpha_coef(alpha_coef),
    _beta_coef(beta_coef),
    _inner_pi_its(inner_pi_its),
    _inner_g_its(inner_g_its),
    _smoother(smoother)
{
}

void
MatrixFreeAMS::SetOperator(const mfem::Operator & op)
{
  height = op.Height();
  width = op.Width();

  _smoother.SetOperator(op);
  auto * smoother_ref = new SmootherReference(_smoother);

  // The constructor of mfem::MatrixFreeAMS requires the target operator to be known, so this
  // constructs the solver
  auto matrix_free_ams = std::make_unique<mfem::MatrixFreeAMS>(*_aform,
                                                               const_cast<mfem::Operator &>(op),
                                                               *_aform->ParFESpace(),
                                                               &_alpha_coef,
                                                               &_beta_coef,
                                                               nullptr,
                                                               _ess_bdr_markers,
                                                               _inner_pi_its,
                                                               _inner_g_its,
                                                               smoother_ref);
  _matrix_free_ams = std::move(matrix_free_ams);
}
} // namespace Moose::MFEM

InputParameters
MFEMMatrixFreeAMS::validParams()
{
  InputParameters params = Moose::MFEM::LORLinearSolverBase<mfem::MatrixFreeAMS>::validParams();
  params.addClassDescription("MFEM matrix-free auxiliary-space Maxwell preconditioner for the "
                             "iterative solution of MFEM equation systems.");
  params.addParam<MFEMScalarCoefficientName>(
      "alpha_coefficient",
      "1.",
      "Name of scalar coefficient used in curl-curl component of target equation system.");
  params.addParam<MFEMScalarCoefficientName>(
      "beta_coefficient",
      "1.",
      "Name of scalar coefficient used in mass component of target equation system.");
  params.addParam<unsigned int>(
      "inner_pi_iterations", 2, "Number of CG iterations on auxiliary Pi space.");
  params.addParam<unsigned int>(
      "inner_g_iterations", 2, "Number of CG iterations on auxiliary G space.");
  params.addParam<MFEMSolverName>(
      "smoother",
      "Smoother for the mfem::MatrixFreeAMS solve. Must accept a matrix-free operator, e.g. "
      "MFEMOperatorJacobiSmoother or MFEMOperatorChebyshevSmoother. Defaults to damped Jacobi on "
      "the operator being preconditioned.");
  // mfem::MatrixFreeAMS is always an LOR solver
  params.setParameters("low_order_refined", true);
  params.suppressParameter<bool>("low_order_refined");
  return params;
}

MFEMMatrixFreeAMS::MFEMMatrixFreeAMS(const InputParameters & parameters)
  : Moose::MFEM::LORLinearSolverBase<mfem::MatrixFreeAMS>(parameters),
    _alpha_coef(getScalarCoefficient("alpha_coefficient")),
    _beta_coef(getScalarCoefficient("beta_coefficient")),
    _inner_pi_its(getParam<unsigned int>("inner_pi_iterations")),
    _inner_g_its(getParam<unsigned int>("inner_g_iterations"))
{
  ConstructSolver();
}

void
MFEMMatrixFreeAMS::ConstructSolver()
{
  mfem::Solver * smoother = nullptr;
  if (isParamSetByUser("smoother"))
  {
    auto & smoother_object = getMFEMProblem().getMFEMObject<LinearSolverBase>(
        "Moose::MFEM::SolverBase", getParam<MFEMSolverName>("smoother"));

    // The smoother is driven through its mfem::Solver alone, so the LOR and preconditioner setup
    // that its own UpdateEquationSystemContext() performs never runs
    const auto & smoother_params = smoother_object.parameters();
    if (smoother_params.have_parameter<bool>("low_order_refined") &&
        smoother_params.get<bool>("low_order_refined"))
      paramError("smoother", "Low-order-refined smoothers are not supported.");
    if (smoother_params.have_parameter<MFEMSolverName>("preconditioner") &&
        smoother_params.isParamSetByUser("preconditioner"))
      paramError("smoother", "Smoothers with a preconditioner are not supported.");

    smoother = &smoother_object.GetSolver();
  }
  else
  {
    // Jacobi on the operator being preconditioned rather than mfem::MatrixFreeAMS's own smoother
    // built from _aform: for a nonlinear problem the gradient term belongs to the equation
    // system's nonlinear form, so _aform is not that operator. The damping matches the value MFEM
    // uses for its own smoother.
    _default_smoother = std::make_unique<mfem::OperatorJacobiSmoother>(0.25);
    smoother = _default_smoother.get();
  }
  auto solver = std::make_unique<Moose::MFEM::MatrixFreeAMS>(
      _alpha_coef, _beta_coef, *smoother, _inner_pi_its, _inner_g_its);
  _solver = std::move(solver);
}

template <>
void
Moose::MFEM::LORLinearSolverBase<mfem::MatrixFreeAMS>::UpdateEquationSystemContext()
{
  LinearSolverBase::UpdateEquationSystemContext();
  SetupLOR(_equation_system);
  // update the pointer to the bilinear form representing the curl-curl problem being
  // preconditioned
  auto & matrix_free_ams = cast_ref<Moose::MFEM::MatrixFreeAMS &>(*_solver);
  matrix_free_ams.SetBilinearForm(*_a);
  matrix_free_ams.SetBoundaryMarkers(_ess_bdr_markers);
}

#endif
