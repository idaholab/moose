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

registerMooseObject("MooseApp", MFEMMatrixFreeAMS);

namespace Moose::MFEM
{
MatrixFreeAMS::MatrixFreeAMS(mfem::Coefficient & alpha_coef,
                             mfem::Coefficient & beta_coef,
                             int inner_pi_its,
                             int inner_g_its)
  : _alpha_coef(alpha_coef),
    _beta_coef(beta_coef),
    _inner_pi_its(inner_pi_its),
    _inner_g_its(inner_g_its)
{
}

void
MatrixFreeAMS::SetOperator(const mfem::Operator & op)
{
  height = op.Height();
  width = op.Width();

  // Build the Jacobi smoother from op rather than letting mfem::MatrixFreeAMS build its own from
  // _aform. For a nonlinear problem the gradient term belongs to the equation system's nonlinear
  // form, so _aform, the bilinear form, is not the operator being preconditioned. The damping
  // matches the value MFEM uses for its own smoother.
  mfem::Solver * smoother = nullptr;
  if (_nonlinear)
  {
    auto * jacobi = new mfem::OperatorJacobiSmoother(0.25);
    jacobi->SetOperator(op); // the damping-only constructor defers the diagonal to here
    smoother = jacobi;
  }

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
                                                               smoother);
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
  auto solver = std::make_unique<Moose::MFEM::MatrixFreeAMS>(
      _alpha_coef, _beta_coef, _inner_pi_its, _inner_g_its);
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
  matrix_free_ams.SetNonlinear(_equation_system->IsNonlinear());
}

#endif
