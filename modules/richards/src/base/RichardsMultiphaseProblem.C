//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RichardsMultiphaseProblem.h"

// MOOSE includes
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "NonlinearSystem.h"

registerMooseObject("RichardsApp", RichardsMultiphaseProblem);

InputParameters
RichardsMultiphaseProblem::validParams()
{
  InputParameters params = FEProblemBase::validParams();
  params.addRequiredParam<NonlinearVariableName>(
      "bounded_var", "Variable whose value will be constrained to be greater than lower_var");
  params.addRequiredParam<NonlinearVariableName>(
      "lower_var",
      "Variable that acts as a lower bound to bounded_var.  It will not be "
      "constrained during the solution procedure");
  return params;
}

RichardsMultiphaseProblem::RichardsMultiphaseProblem(const InputParameters & params)
  : FEProblem(params),
    // in the following have to get the names of the variables, and then find their numbers in
    // initialSetup,
    // as their numbers won't be defined at the moment of instantiation of this class
    _bounded_var_name(params.get<NonlinearVariableName>("bounded_var")),
    _lower_var_name(params.get<NonlinearVariableName>("lower_var")),
    _bounded_var_num(0),
    _lower_var_num(0)
{
}

RichardsMultiphaseProblem::~RichardsMultiphaseProblem() {}

void
RichardsMultiphaseProblem::initialSetup()
{
  // the first argument to getVariable is threadID - i hope the following always works
  unsigned int tid = 0;

  // We are going to do more specific checks below, hence allowing
  // more specific error messages to be printed in case something goes
  // wrong. Therefore we just pass VAR_ANY here.
  MooseVariableFEBase & bounded = getVariable(
      tid, _bounded_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);
  MooseVariableFEBase & lower = getVariable(
      tid, _lower_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);

  // some checks
  if (!bounded.isNodal() || !lower.isNodal())
    mooseError("Both the bounded and lower variables must be nodal variables in "
               "RichardsMultiphaseProblem");
  if (bounded.feType().family != lower.feType().family)
    mooseError("Both the bounded and lower variables must belong to the same element family, eg "
               "LAGRANGE, in RichardsMultiphaseProblem");
  if (bounded.feType().order != lower.feType().order)
    mooseError("Both the bounded and lower variables must have the same order, eg FIRST, in "
               "RichardsMultiphaseProblem");

  // extract the required info
  _bounded_var_num = bounded.number();
  _lower_var_num = lower.number();

  FEProblemBase::initialSetup();
}

bool
RichardsMultiphaseProblem::shouldUpdateSolution()
{
  return true;
}

bool
RichardsMultiphaseProblem::updateSolution(NumericVector<Number> & vec_solution,
                                          NumericVector<Number> & ghosted_solution)
{
  bool updatedSolution =
      false; // this gets set to true if we needed to enforce the bound at any node

  unsigned int sys_num = getNonlinearSystemBase().number();

  // For parallel procs i believe that i have to use local_nodes_begin, rather than just nodes_begin
  // _mesh comes from SystemBase (_mesh = getNonlinearSystemBase().subproblem().mesh(), and
  // subproblem is this object)
  for (const auto & node : _mesh.getMesh().local_node_ptr_range())
  {
    // dofs[0] is the dof number of the bounded variable at this node
    // dofs[1] is the dof number of the lower variable at this node
    std::vector<dof_id_type> dofs(2);
    dofs[0] = node->dof_number(sys_num, _bounded_var_num, 0);
    dofs[1] = node->dof_number(sys_num, _lower_var_num, 0);

    // soln[0] is the value of the bounded variable at this node
    // soln[1] is the value of the lower variable at this node
    std::vector<Number> soln(2);
    vec_solution.get(dofs, soln);

    // do the bounding
    if (soln[0] < soln[1])
    {
      vec_solution.set(dofs[0], soln[1]); // set the bounded variable equal to the lower value
      updatedSolution = true;
    }
  }

  // The above vec_solution.set calls potentially added "set" commands to a queue
  // The following actions the queue (doing MPI commands if necessary), so
  // vec_solution will actually be modified by this following command
  vec_solution.close();

  // if any proc updated the solution, all procs will know about it
  _communicator.max(updatedSolution);

  if (updatedSolution)
  {
    ghosted_solution = vec_solution;
    ghosted_solution.close();
  }

  return updatedSolution;
}
