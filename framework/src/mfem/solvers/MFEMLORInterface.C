//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMLORInterface.h"

namespace Moose::MFEM
{
InputParameters
LORInterface::validParams()
{
  InputParameters params = SolverBase::validParams();
  params.addParam<bool>("low_order_refined", false, "Set usage of Low-Order Refined solver.");
  return params;
}

LORInterface::LORInterface(LinearSolverBase & solver_base)
  : _solver_base(solver_base),
    _lor{solver_base.parameters().get<bool>("low_order_refined")}
{
}

void
LORInterface::CheckSpectralEquivalence(mfem::ParBilinearForm & blf) const
{
  if (auto fec = dynamic_cast<const mfem::H1_FECollection *>(blf.FESpace()->FEColl()))
  {
    if (fec->GetBasisType() != mfem::BasisType::GaussLobatto)
      mooseError("Low-Order-Refined solver requires the FESpace basis to be GaussLobatto "
                 "for H1 elements.");
  }
  else if (auto fec = dynamic_cast<const mfem::ND_FECollection *>(blf.FESpace()->FEColl()))
  {
    if (fec->GetClosedBasisType() != mfem::BasisType::GaussLobatto ||
        fec->GetOpenBasisType() != mfem::BasisType::IntegratedGLL)
      mooseError("Low-Order-Refined solver requires the FESpace closed-basis to be GaussLobatto "
                 "and the open-basis to be IntegratedGLL for ND elements.");
  }
  else if (auto fec = dynamic_cast<const mfem::RT_FECollection *>(blf.FESpace()->FEColl()))
  {
    if (fec->GetClosedBasisType() != mfem::BasisType::GaussLobatto ||
        fec->GetOpenBasisType() != mfem::BasisType::IntegratedGLL)
      mooseError("Low-Order-Refined solver requires the FESpace closed-basis to be GaussLobatto "
                 "and the open-basis to be IntegratedGLL for RT elements.");
  }
}
} // namespace Moose::MFEM

#endif
