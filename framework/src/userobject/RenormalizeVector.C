//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RenormalizeVector.h"
#include "MooseError.h"
#include "NonlinearSystemBase.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/int_range.h"

registerMooseObject("MooseApp", RenormalizeVector);

class RenormalizeVectorThread : public ParallelObject
{
public:
  RenormalizeVectorThread(FEProblemBase & problem,
                          const std::vector<VariableName> & vars,
                          const Real norm);
  // Splitting Constructor
  RenormalizeVectorThread(RenormalizeVectorThread & x, Threads::split split);
  // destructor to close the solution once more
  ~RenormalizeVectorThread();

  void operator()(const ConstElemRange & range);
  void join(const RenormalizeVectorThread &) {}

protected:
  FEProblemBase & _problem;
  System * _sys;
  std::vector<unsigned int> _var_numbers;
  const Real _target_norm;

  THREAD_ID _tid;
};

InputParameters
RenormalizeVector::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Renormalize the solution of a set of variables comprising a vector");
  params.addCoupledVar("v", "Variables comprising the vector");
  params.addParam<Real>("norm", 1.0, "Desired norm for the coupled variable vector");
  return params;
}

RenormalizeVector::RenormalizeVector(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _mesh(_fe_problem.mesh()),
    _vars(getParam<std::vector<VariableName>>("v")),
    _target_norm(getParam<Real>("norm"))
{
}

void
RenormalizeVector::initialize()
{
}

void
RenormalizeVector::execute()
{
  RenormalizeVectorThread renorm(_fe_problem, _vars, _target_norm);
  ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
  Threads::parallel_reduce(elem_range, renorm);
}

void
RenormalizeVector::finalize()
{
}

// Thread worker implementation follows

RenormalizeVectorThread::RenormalizeVectorThread(FEProblemBase & problem,
                                                 const std::vector<VariableName> & varnames,
                                                 const Real norm)
  : ParallelObject(problem.comm()), _problem(problem), _sys(nullptr), _target_norm(norm)
{
  for (const auto & varname : varnames)
  {
    auto & var = _problem.getVariable(0, varname);
    if (_sys)
    {
      if (_sys != &var.sys().system())
        mooseError("Variables passed in RenormalizeVectorThread must be all in the same system.");
    }
    else
      _sys = &var.sys().system();

    if (var.isArray())
    {
      const auto & array_var = _problem.getArrayVariable(0, varname);
      for (unsigned int p = 0; p < var.count(); ++p)
        _var_numbers.push_back(_sys->variable_number(array_var.componentName(p)));
    }
    else
      _var_numbers.push_back(_sys->variable_number(varname));
  }

  // get the non linear system
  const auto & nl_sys = _problem.getNonlinearSystemBase();

  if (&(nl_sys.solutionState(0)) != _sys->solution.get())
    mooseError("Specify variables from the non-linear system");

  // do one solution.close to get updated
  _sys->solution->close();
}

RenormalizeVectorThread::~RenormalizeVectorThread()
{
  auto & nl_sys = _problem.getNonlinearSystemBase();
  for (const auto s : make_range(3))
    if (nl_sys.hasSolutionState(s))
      nl_sys.solutionState(s).close();

  _sys->update();
}

// Splitting Constructor
RenormalizeVectorThread::RenormalizeVectorThread(RenormalizeVectorThread & x,
                                                 Threads::split /*split*/)
  : ParallelObject(x._problem.comm()),
    _problem(x._problem),
    _sys(x._sys),
    _var_numbers(x._var_numbers),
    _target_norm(x._target_norm)
{
}

void
RenormalizeVectorThread::operator()(const ConstElemRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  // get the non linear system
  auto & nl_sys = _problem.getNonlinearSystemBase();

  mooseAssert(_sys, "We should have a system, did you forget to specify any variable in vars?");
  auto & dof_map = _sys->get_dof_map();
  const auto local_dof_begin = dof_map.first_dof();
  const auto local_dof_end = dof_map.end_dof();

  std::vector<std::vector<dof_id_type>> dof_indices(_var_numbers.size());
  std::vector<Real> cache(_var_numbers.size());

  for (const auto & elem : range)
  {
    // prepare variable dofs
    for (const auto i : index_range(_var_numbers))
    {
      dof_map.dof_indices(elem, dof_indices[i], _var_numbers[i]);

      // check that all vars have the same number of dofs
      mooseAssert(dof_indices[i].size() == dof_indices[0].size(),
                  "All specified variables should have the same number of DOFs");
    }

    // iterate over current, old, and older solutions
    for (const auto s : make_range(3))
      if (nl_sys.hasSolutionState(s))
      {
        auto & solution = nl_sys.solutionState(s);

        // loop over all DOFs
        for (const auto j : index_range(dof_indices[0]))
        {
          // check if the first variable's DOFs are local (if they are all other variables should
          // have local DOFS as well)
          if (dof_indices[0][j] > local_dof_end || dof_indices[0][j] < local_dof_begin)
            continue;

          // compute current norm
          Real norm = 0.0;
          for (const auto i : index_range(_var_numbers))
            norm += Utility::pow<2>(solution(dof_indices[i][j]));

          if (norm == 0.0)
            continue;
          norm = std::sqrt(norm);

          // renormalize
          for (const auto i : index_range(_var_numbers))
            solution.set(dof_indices[i][j], solution(dof_indices[i][j]) / norm * _target_norm);
        }
      }
  }
}
