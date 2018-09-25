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
#include "FEProblemBase.h"

template <>
InputParameters
validParams<TimeIntegrator>()
{
  InputParameters params = validParams<MooseObject>();
  params.registerBase("TimeIntegrator");
  return params;
}

TimeIntegrator::TimeIntegrator(const InputParameters & parameters)
  : MooseObject(parameters),
    Restartable(this, "TimeIntegrators"),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _nl(_fe_problem.getNonlinearSystemBase()),
    _nonlinear_implicit_system(dynamic_cast<const NonlinearImplicitSystem *>(&_sys.system())),
    _u_dot(_sys.solutionUDot()),
    _du_dot_du(_sys.duDotDu()),
    _solution(_sys.currentSolution()),
    _solution_old(_sys.solutionOld()),
    _solution_older(_sys.solutionOlder()),
    _t_step(_fe_problem.timeStep()),
    _dt(_fe_problem.dt()),
    _dt_old(_fe_problem.dtOld()),
    _Re_time(_nl.getResidualTimeVector()),
    _Re_non_time(_nl.getResidualNonTimeVector()),
    _n_nonlinear_iterations(0),
    _n_linear_iterations(0)
{
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

void
TimeIntegrator::computeDotProperties(Moose::MaterialDataType type, THREAD_ID tid) const
{
  auto material_data = _fe_problem.getMaterialData(type, tid);
  if (!material_data->hasDotProperties())
    return;

  const auto & props = material_data->props();
  const auto & old_props = material_data->propsOld();
  auto & dot_props = material_data->propsDot();

  auto n = dot_props.size();
  for (decltype(n) i = 0; i < n; ++i)
  {
    // skip properties that have not been required for time derivatives
    if (!dot_props[i])
      continue;

    auto & dot_prop = dynamic_cast<MaterialProperty<Real> &>(*dot_props[i]);
    auto & prop = dynamic_cast<const MaterialProperty<Real> &>(*props[i]);
    auto & old_prop = dynamic_cast<const MaterialProperty<Real> &>(*old_props[i]);
    for (unsigned int qp = 0; qp < prop.size(); ++qp)
      dot_prop[qp] = (prop[qp] - old_prop[qp]) / _dt;
  }
}
