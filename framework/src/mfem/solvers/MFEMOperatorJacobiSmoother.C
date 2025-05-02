#ifdef MFEM_ENABLED

#include "MFEMOperatorJacobiSmoother.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMOperatorJacobiSmoother);

InputParameters
MFEMOperatorJacobiSmoother::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();
  params.addClassDescription("MFEM solver for performing Jacobi smoothing of the equation system.");
  return params;
}

MFEMOperatorJacobiSmoother::MFEMOperatorJacobiSmoother(const InputParameters & parameters)
  : MFEMSolverBase(parameters)
{
  constructSolver(parameters);
}

void
MFEMOperatorJacobiSmoother::constructSolver(const InputParameters &)
{
  _jacobian_preconditioner = std::make_shared<mfem::OperatorJacobiSmoother>();
  _preconditioner = std::dynamic_pointer_cast<mfem::Solver>(_jacobian_preconditioner);
}

void
MFEMOperatorJacobiSmoother::updateSolver(mfem::ParBilinearForm &a, mfem::Array<int> &tdofs)
{

  if (getParam<bool>("low_order_refined"))
    _preconditioner.reset(new mfem::LORSolver<mfem::OperatorJacobiSmoother>(a, tdofs));
  
}

#endif
