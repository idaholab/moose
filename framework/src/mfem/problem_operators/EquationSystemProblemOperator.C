#ifdef MFEM_ENABLED

#include "EquationSystemProblemOperator.h"

namespace Moose::MFEM
{
void
EquationSystemProblemOperator::SetGridFunctions()
{
  _trial_var_names = GetEquationSystem()->TrialVarNames();
  ProblemOperator::SetGridFunctions();
}

void
EquationSystemProblemOperator::Init(mfem::BlockVector & X)
{
  ProblemOperator::Init(X);

  GetEquationSystem()->BuildEquationSystem();
}

void
EquationSystemProblemOperator::AddEstimator(std::shared_ptr<MFEMEstimator> estimator)
{
  _estimator = estimator;
}

void
EquationSystemProblemOperator::SetUpAMR()
{
  _use_amr = true;

  _refiner = std::make_unique<mfem::ThresholdRefiner>(*_estimator->createEstimator());
  _refiner->SetTotalErrorFraction(0.7);
}

/*
The refiner will return true if we have met the stopping condition for refinement.
If we don't use AMR, we should just return false.

TODO: should probably call an error if _use_amr is set to false

*/
bool
EquationSystemProblemOperator::HRefine()
{
  bool output = false;
  if (_use_amr)
  {
    _refiner->Apply(*_problem.pmesh);

    output = _refiner->Stop();

    // update after refinement as well; previously this was done in the executioner
    UpdateAfterRefinement();
  }
  return output;
}

bool
EquationSystemProblemOperator::PRefine()
{
  bool output = false;
  if (_use_amr)
  {
    mfem::Array<mfem::pRefinement> prefinements;
    mfem::Array<mfem::Refinement> refinements;

    _refiner->MarkWithoutRefining(*_problem.pmesh, refinements);

    output = (_problem.pmesh->ReduceInt(refinements.Size()) == 0LL);

    prefinements.SetSize(refinements.Size());
    for (int i = 0; i < refinements.Size(); i++)
    {
      prefinements[i].index = refinements[i].index;
      prefinements[i].delta = 1; // Increase the element order by 1
    }

    _estimator->getFESpace()->PRefineAndUpdate(prefinements);
    UpdateAfterRefinement();
  }
  return output;
}

void
EquationSystemProblemOperator::Solve(mfem::Vector &)
{
  GetEquationSystem()->BuildJacobian(_true_x, _true_rhs);

  if ((_problem.jacobian_solver->isLOR() || _problem.jacobian_preconditioner->isLOR()) &&
      _equation_system->_test_var_names.size() > 1)
    mooseError("LOR solve is only supported for single-variable systems");

  _problem.jacobian_solver->updateSolver(
      *_equation_system->_blfs.Get(_equation_system->_test_var_names.at(0)),
      _equation_system->_ess_tdof_lists.at(0));

  _problem.nonlinear_solver->SetSolver(*_problem.jacobian_solver->getSolver());
  _problem.nonlinear_solver->SetOperator(*GetEquationSystem());
  _problem.nonlinear_solver->Mult(_true_rhs, _true_x);

  GetEquationSystem()->RecoverFEMSolution(_true_x, _problem.gridfunctions);
}

} // namespace Moose::MFEM

#endif
