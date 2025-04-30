#ifdef MFEM_ENABLED

#include "TimeDomainEquationSystemProblemOperator.h"

namespace Moose::MFEM
{

void
TimeDomainEquationSystemProblemOperator::SetGridFunctions()
{
  _test_var_names = GetEquationSystem()->TestVarNames();
  _trial_var_names = GetEquationSystem()->TrialVarNames();
  _trial_variable_time_derivatives =
      _problem.gridfunctions.Get(GetEquationSystem()->TrialVarTimeDerivativeNames());

  TimeDomainProblemOperator::SetGridFunctions();
}

void
TimeDomainEquationSystemProblemOperator::Init(mfem::BlockVector & X)
{
  TimeDomainProblemOperator::Init(X);
  GetEquationSystem()->BuildEquationSystem();
}

void
TimeDomainEquationSystemProblemOperator::ImplicitSolve(const double dt,
                                                       const mfem::Vector & /*X*/,
                                                       mfem::Vector & dX_dt)
{
  dX_dt = 0.0;
  SetTestVariablesFromTrueVectors();
  for (unsigned int ind = 0; ind < _trial_variables.size(); ++ind)
  {
    _trial_variables.at(ind)->MakeTRef(
        _trial_variables.at(ind)->ParFESpace(), dX_dt, _block_true_offsets[ind]);
  }
  _problem.coefficients.setTime(GetTime());
  BuildEquationSystemOperator(dt);

  //////////////////////////////////////
  //if (_problem.mfem_preconditioner)
  //{
  //  std::cout << "Updating preconditioner..." << std::endl;
  //  _problem.mfem_preconditioner->updateSolver(*_equation_system->_blfs.Get(_equation_system->_trial_var_names.at(0)),_equation_system->_ess_tdof_lists.at(0), _problem.jacobian_preconditioner);
  //}
  
  _problem.mfem_solver->updateSolver(*_equation_system->_blfs.Get(_equation_system->_trial_var_names.at(0)),_equation_system->_ess_tdof_lists.at(0));
  //////////////////////////////////////
  

  _problem.nonlinear_solver->SetSolver(*_problem.jacobian_solver);
  _problem.nonlinear_solver->SetOperator(*GetEquationSystem());
  _problem.nonlinear_solver->Mult(_true_rhs, dX_dt);
  SetTrialVariablesFromTrueVectors();
}

void
TimeDomainEquationSystemProblemOperator::BuildEquationSystemOperator(double dt)
{
  GetEquationSystem()->SetTimeStep(dt);
  GetEquationSystem()->UpdateEquationSystem();
  GetEquationSystem()->BuildJacobian(_true_x, _true_rhs);
}

} // namespace Moose::MFEM

#endif
