//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdjointSolve.h"
#include "OptimizationAppTypes.h"

#include "FEProblem.h"
#include "NonlinearSystemBase.h"
#include "NonlinearSystem.h"
#include "NodalBCBase.h"

#include "libmesh/petsc_matrix.h"
#include "libmesh/petsc_vector.h"
#include "petscmat.h"

InputParameters
AdjointSolve::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addRequiredParam<NonlinearSystemName>(
      "forward_system", "Name of the system representing the forward problem.");
  params.addRequiredParam<NonlinearSystemName>(
      "adjoint_system", "Name of the system representing the adjoint problem.");
  return params;
}

AdjointSolve::AdjointSolve(Executioner & ex)
  : SolveObject(ex),
    _forward_sys_num(_problem.nlSysNum(getParam<NonlinearSystemName>("forward_system"))),
    _adjoint_sys_num(_problem.nlSysNum(getParam<NonlinearSystemName>("adjoint_system"))),
    _nl_forward(_problem.getNonlinearSystemBase(_forward_sys_num)),
    _nl_adjoint(_problem.getNonlinearSystemBase(_adjoint_sys_num))
{
  // These should never be hit, but just in case
  if (!dynamic_cast<NonlinearSystem *>(&_nl_forward))
    paramError("forward_system", "Forward system does not appear to be a 'NonlinearSystem'.");
  if (!dynamic_cast<NonlinearSystem *>(&_nl_adjoint))
    paramError("adjoint_system", "Adjoint system does not appear to be a 'NonlinearSystem'.");
}

bool
AdjointSolve::solve()
{
  TIME_SECTION("execute", 1, "Executing adjoint problem", false);

  if (_inner_solve && !_inner_solve->solve())
    return false;

  _problem.execute(OptimizationAppTypes::EXEC_ADJOINT_TIMESTEP_BEGIN);
  if (!_problem.execMultiApps(OptimizationAppTypes::EXEC_ADJOINT_TIMESTEP_BEGIN))
  {
    _console << "MultiApps failed to converge on ADJOINT_TIMESTEP_BEGIN!" << std::endl;
    return false;
  }

  checkIntegrity();

  // Convenient references
  // Adjoint matrix, solution, and right-hand-side
  auto & matrix = static_cast<ImplicitSystem &>(_nl_forward.system()).get_system_matrix();
  auto & solution = _nl_adjoint.solution();
  auto & rhs = _nl_adjoint.getResidualNonTimeVector();
  // Linear solver parameters
  auto & es = _problem.es();
  const auto tol = es.parameters.get<Real>("linear solver tolerance");
  const auto maxits = es.parameters.get<unsigned int>("linear solver maximum iterations");
  // Linear solver for adjoint system
  auto & solver = *static_cast<ImplicitSystem &>(_nl_adjoint.system()).get_linear_solver();

  // Assemble adjoint system by evaluating the forward Jacobian, computing the adjoint
  // residual/source, and homogenizing nodal BCs
  assembleAdjointSystem(matrix, solution, rhs);
  applyNodalBCs(matrix, solution, rhs);

  // Solve the adjoint system
  solver.adjoint_solve(matrix, solution, rhs, tol, maxits);
  _nl_adjoint.update();
  if (solver.get_converged_reason() < 0)
  {
    _console << "Adjoint solve failed to converge with reason: "
             << Utility::enum_to_string(solver.get_converged_reason()) << std::endl;
    return false;
  }

  _problem.execute(OptimizationAppTypes::EXEC_ADJOINT_TIMESTEP_END);
  if (!_problem.execMultiApps(OptimizationAppTypes::EXEC_ADJOINT_TIMESTEP_END))
  {
    _console << "MultiApps failed to converge on ADJOINT_TIMESTEP_END!" << std::endl;
    return false;
  }

  return true;
}

void
AdjointSolve::assembleAdjointSystem(SparseMatrix<Number> & matrix,
                                    const NumericVector<Number> & /*solution*/,
                                    NumericVector<Number> & rhs)
{
  _problem.computeJacobian(*_nl_forward.currentSolution(), matrix, _forward_sys_num);

  _problem.setCurrentNonlinearSystem(_adjoint_sys_num);
  _problem.computeResidualTag(*_nl_adjoint.currentSolution(), rhs, _nl_adjoint.nonTimeVectorTag());
  rhs.scale(-1.0);
}

void
AdjointSolve::applyNodalBCs(SparseMatrix<Number> & matrix,
                            const NumericVector<Number> & solution,
                            NumericVector<Number> & rhs)
{
  std::vector<dof_id_type> nbc_dofs;
  auto & nbc_warehouse = _nl_forward.getNodalBCWarehouse();
  if (nbc_warehouse.hasActiveObjects())
  {
    for (const auto & bnode : *_mesh.getBoundaryNodeRange())
    {
      BoundaryID boundary_id = bnode->_bnd_id;
      Node * node = bnode->_node;

      if (!nbc_warehouse.hasActiveBoundaryObjects(boundary_id) ||
          node->processor_id() != processor_id())
        continue;

      for (const auto & bc : nbc_warehouse.getActiveBoundaryObjects(boundary_id))
        if (bc->shouldApply())
          for (unsigned int c = 0; c < bc->variable().count(); ++c)
            nbc_dofs.push_back(node->dof_number(_forward_sys_num, bc->variable().number() + c, 0));
    }

    // Petsc has a nice interface for zeroing rows and columns, so we'll use it
    auto petsc_matrix = dynamic_cast<PetscMatrix<Number> *>(&matrix);
    auto petsc_solution = dynamic_cast<const PetscVector<Number> *>(&solution);
    auto petsc_rhs = dynamic_cast<PetscVector<Number> *>(&rhs);
    if (petsc_matrix && petsc_solution && petsc_rhs)
      MatZeroRowsColumns(petsc_matrix->mat(),
                         cast_int<PetscInt>(nbc_dofs.size()),
                         numeric_petsc_cast(nbc_dofs.data()),
                         1.0,
                         petsc_solution->vec(),
                         petsc_rhs->vec());
    else
      mooseError("Using PETSc matrices and vectors is required for applying homogenized boundary "
                 "conditions.");
  }
}

void
AdjointSolve::checkIntegrity()
{
  // Main thing is that the number of dofs in each system is the same
  if (_nl_forward.system().n_dofs() != _nl_adjoint.system().n_dofs())
    mooseError(
        "The forward and adjoint systems do not seem to be the same size. This could be due to (1) "
        "the number of variables added to each system is not the same, (2) variables do not have "
        "consistent family/order, (3) variables do not have the same block restriction.");
}
