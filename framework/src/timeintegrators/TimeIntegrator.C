//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/petsc_nonlinear_solver.h"

#include "TimeIntegrator.h"
#include "FEProblem.h"
#include "SystemBase.h"
#include "NonlinearSystem.h"

InputParameters
TimeIntegrator::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.registerBase("TimeIntegrator");
  return params;
}

TimeIntegrator::TimeIntegrator(const InputParameters & parameters)
  : MooseObject(parameters),
    Restartable(this, "TimeIntegrators"),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _nl(_fe_problem.getNonlinearSystemBase()),
    _nonlinear_implicit_system(dynamic_cast<NonlinearImplicitSystem *>(&_sys.system())),
    _Re_time(_nl.getResidualTimeVector()),
    _Re_non_time(_nl.getResidualNonTimeVector()),
    _du_dot_du(_sys.duDotDu()),
    _solution(_sys.currentSolution()),
    _solution_old(_sys.solutionState(1)),
    _t_step(_fe_problem.timeStep()),
    _dt(_fe_problem.dt()),
    _dt_old(_fe_problem.dtOld()),
    _n_nonlinear_iterations(0),
    _n_linear_iterations(0),
    _is_explicit(false),
    _is_lumped(false),
    _u_dot_factor_tag(_fe_problem.addVectorTag("u_dot_factor", Moose::VECTOR_TAG_SOLUTION)),
    _u_dotdot_factor_tag(_fe_problem.addVectorTag("u_dotdot_factor", Moose::VECTOR_TAG_SOLUTION))
{
  _fe_problem.setUDotRequested(true);
}

void
TimeIntegrator::solve()
{
  _nl.system().solve();

  _n_nonlinear_iterations = getNumNonlinearIterationsLastSolve();
  _n_linear_iterations = getNumLinearIterationsLastSolve();
}

unsigned int
TimeIntegrator::getNumNonlinearIterationsLastSolve() const
{
  return _nonlinear_implicit_system->n_nonlinear_iterations();
}

unsigned int
TimeIntegrator::getNumLinearIterationsLastSolve() const
{
  NonlinearSolver<Real> & nonlinear_solver =
      static_cast<NonlinearSolver<Real> &>(*_nonlinear_implicit_system->nonlinear_solver);

  return nonlinear_solver.get_total_linear_iterations();
}
