//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "libmesh/libmesh_config.h"

#include "EigenProblem.h"

#include "Assembly.h"
#include "AuxiliarySystem.h"
#include "DisplacedProblem.h"
#include "NonlinearEigenSystem.h"
#include "SlepcSupport.h"
#include "RandomData.h"
#include "OutputWarehouse.h"
#include "Function.h"

#include "libmesh/system.h"
#include "libmesh/eigen_solver.h"

template <>
InputParameters
validParams<EigenProblem>()
{
  InputParameters params = validParams<FEProblemBase>();
  return params;
}

EigenProblem::EigenProblem(const InputParameters & parameters)
  : FEProblemBase(parameters),
    // By default, we want to compute an eigenvalue only (smallest or largest)
    _n_eigen_pairs_required(1),
    _generalized_eigenvalue_problem(false),
    _nl_eigen(std::make_shared<NonlinearEigenSystem>(*this, "eigen0")),
    _is_residual_initialed(false)
{
#if LIBMESH_HAVE_SLEPC
  _nl = _nl_eigen;
  _aux = std::make_shared<AuxiliarySystem>(*this, "aux0");

  newAssemblyArray(*_nl_eigen);

  FEProblemBase::initNullSpaceVectors(parameters, *_nl_eigen);

  _eq.parameters.set<EigenProblem *>("_eigen_problem") = this;

#else
  mooseError("Need to install SLEPc to solve eigenvalue problems, please reconfigure\n");
#endif /* LIBMESH_HAVE_SLEPC */
}

EigenProblem::~EigenProblem()
{
#if LIBMESH_HAVE_SLEPC
  FEProblemBase::deleteAssemblyArray();
#endif /* LIBMESH_HAVE_SLEPC */
}

#if LIBMESH_HAVE_SLEPC
void
EigenProblem::setEigenproblemType(Moose::EigenProblemType eigen_problem_type)
{
  switch (eigen_problem_type)
  {
    case Moose::EPT_HERMITIAN:
      _nl_eigen->sys().set_eigenproblem_type(libMesh::HEP);
      _generalized_eigenvalue_problem = false;
      break;

    case Moose::EPT_NON_HERMITIAN:
      _nl_eigen->sys().set_eigenproblem_type(libMesh::NHEP);
      _generalized_eigenvalue_problem = false;
      break;

    case Moose::EPT_GEN_HERMITIAN:
      _nl_eigen->sys().set_eigenproblem_type(libMesh::GHEP);
      _generalized_eigenvalue_problem = true;
      break;

    case Moose::EPT_GEN_INDEFINITE:
      _nl_eigen->sys().set_eigenproblem_type(libMesh::GHIEP);
      _generalized_eigenvalue_problem = true;
      break;

    case Moose::EPT_GEN_NON_HERMITIAN:
      _nl_eigen->sys().set_eigenproblem_type(libMesh::GNHEP);
      _generalized_eigenvalue_problem = true;
      break;

    case Moose::EPT_POS_GEN_NON_HERMITIAN:
      mooseError("libMesh does not support EPT_POS_GEN_NON_HERMITIAN currently \n");
      break;

    case Moose::EPT_SLEPC_DEFAULT:
      _generalized_eigenvalue_problem = false;
      break;

    default:
      mooseError("Unknown eigen solver type \n");
  }
}
#endif

void
EigenProblem::computeJacobian(const NumericVector<Number> & soln,
                              SparseMatrix<Number> & jacobian,
                              Moose::KernelType kernel_type)
{
  // to avoid computing residual
  solverParams()._type = Moose::ST_NEWTON;
  FEProblemBase::computeJacobian(soln, jacobian, kernel_type);
}

void
EigenProblem::checkProblemIntegrity()
{
  FEProblemBase::checkProblemIntegrity();
  _nl_eigen->checkIntegrity();
}

void
EigenProblem::solve()
{
  Moose::perf_log.push("Eigen_solve()", "Execution");
  if (_solve)
  {
    _nl->solve();
    _nl->update();
  }

  // sync solutions in displaced problem
  if (_displaced_problem)
    _displaced_problem->syncSolutions();

  Moose::perf_log.pop("Eigen_solve()", "Execution");
}

bool
EigenProblem::converged()
{
  return _nl_eigen->converged();
}

bool
EigenProblem::isNonlinearEigenvalueSolver()
{
  return solverParams()._eigen_solve_type == Moose::EST_NONLINEAR_POWER ||
         solverParams()._eigen_solve_type == Moose::EST_MF_NONLINEAR_POWER ||
         solverParams()._eigen_solve_type == Moose::EST_MONOLITH_NEWTON ||
         solverParams()._eigen_solve_type == Moose::EST_MF_MONOLITH_NEWTON;
}

void
EigenProblem::computeResidualTypeBx(const NumericVector<Number> & soln,
                                    NumericVector<Number> & residual,
                                    Moose::KernelType type)
{
  _nl->setSolution(soln);

  _nl->zeroVariablesForResidual();

  _nl->computeResidual(residual, type);
}

void
EigenProblem::computeResidualType(const NumericVector<Number> & soln,
                                  NumericVector<Number> & residual,
                                  Moose::KernelType type)
{
  // if Ax is just compputed, we do not do extra computation such as Transfer
  if (type == Moose::KT_EIGEN && _is_residual_initialed)
  {
    computeResidualTypeBx(soln, residual, type);
    _is_residual_initialed = false;
  }
  else
  {
    FEProblemBase::computeResidualType(soln, residual, type);
    if (type == Moose::KT_NONEIGEN)
      _is_residual_initialed = true;
  }
}
