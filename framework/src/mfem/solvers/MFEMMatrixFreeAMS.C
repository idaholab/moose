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
  InputParameters params = Moose::MFEM::LinearSolverBase::validParams();
  params += MFEMBoundaryRestrictable::validParams();
  params.addClassDescription("MFEM matrix-free auxiliary-space Maxwell preconditioner for the "
                             "iterative solution of MFEM equation systems.");
  // mfem::MatrixFreeAMS is always an LOR solver
  params.setParameters("low_order_refined", true);
  params.suppressParameter<bool>("low_order_refined");
  return params;
}

MFEMMatrixFreeAMS::MFEMMatrixFreeAMS(const InputParameters & parameters)
  : Moose::MFEM::LinearSolverBase(parameters),
    MFEMBoundaryRestrictable(parameters, getMFEMProblem().mesh().getMFEMParMesh())
{
}

void
MFEMMatrixFreeAMS::SetOperator(mfem::OperatorHandle & op)
{
  mfem::Coefficient * alpha_coef{nullptr};
  mfem::Coefficient * beta_coef{nullptr};
  // Fetch material coefficients from CurlCurlIntegrator and (optional) VectorFEMassIntegrator
  // on _aform
  mfem::Array<mfem::BilinearFormIntegrator *> & domain_integs = *_aform->GetDBFI();
  for (const auto i : make_range(domain_integs.Size()))
  {
    auto * integ = domain_integs[i];
    if (dynamic_cast<mfem::CurlCurlIntegrator *>(integ))
      alpha_coef = const_cast<mfem::Coefficient *>(
          static_cast<mfem::CurlCurlIntegrator *>(integ)->GetCoefficient());
    if (dynamic_cast<mfem::VectorFEMassIntegrator *>(integ))
      beta_coef = const_cast<mfem::Coefficient *>(
          static_cast<mfem::VectorFEMassIntegrator *>(integ)->GetCoefficient());
  }
  _solver = std::make_unique<mfem::MatrixFreeAMS>(
      *_aform,
      *op,
      *_aform->ParFESpace(),
      alpha_coef,
      beta_coef,
      nullptr,
      const_cast<mfem::Array<int> &>(getBoundaryAttributes()));
}

void
MFEMMatrixFreeAMS::SetupLOR(mfem::ParBilinearForm & a, mfem::Array<int> & /*tdofs*/)
{
  // update the pointer to the bilinear form representing the curl-curl problem being preconditioned
  _aform = &a;
}

#endif
