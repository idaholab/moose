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
MFEMSuperLU::updateSolver(mfem::ParBilinearForm &, mfem::Array<int> &)
{

  if (getParam<bool>("low_order_refined"))
    mooseError("SuperLU solver does not support LOR solve");
  
}

#endif
