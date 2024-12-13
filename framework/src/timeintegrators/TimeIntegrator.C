//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeIntegrator.h"
#include "FEProblem.h"
#include "NonlinearSystemBase.h"

#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/nonlinear_solver.h"
#include "libmesh/dof_map.h"

using namespace libMesh;

InputParameters
TimeIntegrator::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addParam<std::vector<VariableName>>(
      "variables", {}, "A subset of the variables that this time integrator should be applied to");
  params.registerBase("TimeIntegrator");
  return params;
}

TimeIntegrator::TimeIntegrator(const InputParameters & parameters)
  : MooseObject(parameters),
    Restartable(this, "TimeIntegrators"),
    NonlinearTimeIntegratorInterface(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"),
                                     *getCheckedPointerParam<SystemBase *>("_sys")),
    LinearTimeIntegratorInterface(*getCheckedPointerParam<SystemBase *>("_sys")),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _du_dot_du(_sys.duDotDus()),
    _solution(_sys.currentSolution()),
    _solution_old(_sys.solutionState(1)),
    _solution_sub(declareRestartableDataWithContext<std::unique_ptr<NumericVector<Number>>>(
        "solution_sub", &const_cast<libMesh::Parallel::Communicator &>(this->comm()))),
    _solution_old_sub(declareRestartableDataWithContext<std::unique_ptr<NumericVector<Number>>>(
        "solution_old_sub", &const_cast<libMesh::Parallel::Communicator &>(this->comm()))),
    _t_step(_fe_problem.timeStep()),
    _dt(_fe_problem.dt()),
    _dt_old(_fe_problem.dtOld()),
    _n_nonlinear_iterations(0),
    _n_linear_iterations(0),
    _is_lumped(false),
    _var_restriction(declareRestartableData<bool>(
        "var_restriction", !getParam<std::vector<VariableName>>("variables").empty())),
    _local_indices(declareRestartableData<std::vector<dof_id_type>>("local_indices")),
    _vars(declareRestartableData<std::unordered_set<unsigned int>>("vars")),
    _from_subvector(NumericVector<Number>::build(this->comm()))
{
  _fe_problem.setUDotRequested(true);
}

void
TimeIntegrator::init()
{
  if (!_var_restriction)
    return;

  const auto & var_names = getParam<std::vector<VariableName>>("variables");
  std::vector<unsigned int> var_num_vec;
  auto & lm_sys = _sys.system();
  lm_sys.get_all_variable_numbers(var_num_vec);
  std::unordered_set<unsigned int> var_nums(var_num_vec.begin(), var_num_vec.end());
  for (const auto & var_name : var_names)
    if (lm_sys.has_variable(var_name))
    {
      const auto var_num = lm_sys.variable_number(var_name);
      _vars.insert(var_num);
      var_nums.erase(var_num);
    }

  // If var_nums is empty then that means the user has specified all the variables in this system
  if (var_nums.empty())
  {
    _var_restriction = false;
    return;
  }

  std::vector<dof_id_type> var_dof_indices, work_vec;
  for (const auto var_num : _vars)
  {
    work_vec = _local_indices;
    _local_indices.clear();
    lm_sys.get_dof_map().local_variable_indices(var_dof_indices, lm_sys.get_mesh(), var_num);
    std::merge(work_vec.begin(),
               work_vec.end(),
               var_dof_indices.begin(),
               var_dof_indices.end(),
               std::back_inserter(_local_indices));
  }

  _solution_sub = NumericVector<Number>::build(_solution->comm());
  _solution_old_sub = NumericVector<Number>::build(_solution_old.comm());
}

void
TimeIntegrator::solve()
{
  mooseError("Calling TimeIntegrator::solve() is no longer supported");
}

void
TimeIntegrator::setNumIterationsLastSolve()
{
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
  auto & nonlinear_solver = _nonlinear_implicit_system->nonlinear_solver;
  libmesh_assert(nonlinear_solver);

  return nonlinear_solver->get_total_linear_iterations();
}

void
TimeIntegrator::copyVector(const NumericVector<Number> & from, NumericVector<Number> & to)
{
  if (!_var_restriction)
    to = from;
  else
  {
    auto to_sub = to.get_subvector(_local_indices);
    from.create_subvector(*_from_subvector, _local_indices, false);
    *to_sub = *_from_subvector;
    to.restore_subvector(std::move(to_sub), _local_indices);
  }
}

bool
TimeIntegrator::integratesVar(const unsigned int var_num) const
{
  if (!_var_restriction)
    return true;

  return _vars.count(var_num);
}

void
TimeIntegrator::computeDuDotDu()
{
  const auto coeff = duDotDuCoeff();
  for (const auto i : index_range(_du_dot_du))
    if (integratesVar(i))
      _du_dot_du[i] = coeff / _dt;
}
