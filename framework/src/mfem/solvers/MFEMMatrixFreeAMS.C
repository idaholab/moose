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
  // Deferred: construction of the mfem::MatrixFreeAMS solver is postponed until the operator is set
}

void
MFEMMatrixFreeAMS::SetOperatorImpl(const mfem::Operator & op)
{
  _solver = std::make_unique<mfem::MatrixFreeAMS>(*_a,
                                                  const_cast<mfem::Operator &>(op),
                                                  *_a->ParFESpace(),
                                                  &_alpha_coef,
                                                  &_beta_coef,
                                                  nullptr,
                                                  _ess_bdr_markers,
                                                  _inner_pi_its,
                                                  _inner_g_its);
}

template <>
void
Moose::MFEM::LORLinearSolverBase<mfem::MatrixFreeAMS>::UpdateEquationSystemContext()
{
  LinearSolverBase::UpdateEquationSystemContext();
  SetupLOR();
}

#endif
