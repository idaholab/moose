#ifdef MFEM_ENABLED

#include "MFEMSuperLU.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMSuperLU);

InputParameters
MFEMSuperLU::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();
  params.addClassDescription("MFEM solver for performing direct solves of sparse systems in "
                             "parallel using the SuperLU_DIST library.");
  params.addParam<bool>("low_order_refined", false, "Set usage of Low-Order Refined solver.");

  return params;
}

MFEMSuperLU::MFEMSuperLU(const InputParameters & parameters) : MFEMSolverBase(parameters)
{
  constructSolver(parameters);
}

void
MFEMSuperLU::constructSolver(const InputParameters &)
{
  _jacobian_solver = std::make_shared<Moose::MFEM::SuperLUSolver>(
      getMFEMProblem().mesh().getMFEMParMesh().GetComm());
  _solver = std::dynamic_pointer_cast<mfem::Solver>(_jacobian_solver);
}

void
MFEMSuperLU::updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs)
{

  if (getParam<bool>("low_order_refined"))
  {
    mfem::ParLORDiscretization lor_disc(a, tdofs);
    _solver.reset(new mfem::LORSolver<Moose::MFEM::SuperLUSolver>(
        lor_disc, getMFEMProblem().mesh().getMFEMParMesh().GetComm()));
  }
}

#endif
