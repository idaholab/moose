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

// libMesh includes
#include "libmesh/system.h"
#include "libmesh/eigen_solver.h"
#include "libmesh/enum_eigen_solver_type.h"

registerMooseObject("MooseApp", EigenProblem);

template <>
InputParameters
validParams<EigenProblem>()
{
  InputParameters params = validParams<FEProblemBase>();
  params.addParam<unsigned int>(
      "active_eigen_index", 0, "Which eigen vector is used to compute residual and also associateed to nonlinear variable");

  // Grab parameters from eigenvalue excutioner
#ifdef LIBMESH_HAVE_SLEPC
  Moose::SlepcSupport::getSlepcValidParams(params);

  params += Moose::SlepcSupport::getSlepcEigenProblemValidParams();
#endif

  return params;
}

EigenProblem::EigenProblem(const InputParameters & parameters)
  : FEProblemBase(parameters),
    // By default, we want to compute an eigenvalue only (smallest or largest)
    _n_eigen_pairs_required(1),
    _generalized_eigenvalue_problem(false),
    _nl_eigen(std::make_shared<NonlinearEigenSystem>(*this, "eigen0")),
    _active_eigen_index(getParam<unsigned int>("active_eigen_index")),
    _compute_jacobian_tag_timer(registerTimedSection("computeJacobianTag", 3)),
    _compute_jacobian_ab_timer(registerTimedSection("computeJacobianAB", 3)),
    _compute_residual_tag_timer(registerTimedSection("computeResidualTag", 3)),
    _compute_residual_ab_timer(registerTimedSection("computeResidualAB", 3)),
    _solve_timer(registerTimedSection("solve", 1))
{
#if LIBMESH_HAVE_SLEPC
  _nl = _nl_eigen;
  _aux = std::make_shared<AuxiliarySystem>(*this, "aux0");

  newAssemblyArray(*_nl_eigen);

  FEProblemBase::initNullSpaceVectors(parameters, *_nl_eigen);

  _eq.parameters.set<EigenProblem *>("_eigen_problem") = this;

  Moose::SlepcSupport::storeSlepcOptions(*this, parameters);
#else
  mooseError("Need to install SLEPc to solve eigenvalue problems, please reconfigure\n");
#endif /* LIBMESH_HAVE_SLEPC */

  // Create extra vectors and matrices if any
  createTagVectors();
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

void
EigenProblem::computeJacobianTag(const NumericVector<Number> & soln,
                                 SparseMatrix<Number> & jacobian,
                                 TagID tag)
{
  TIME_SECTION(_compute_jacobian_tag_timer);

  _fe_matrix_tags.clear();

  _fe_matrix_tags.insert(tag);

  _nl_eigen->setSolution(soln);

  _nl_eigen->disassociateAllTaggedMatrices();

  _nl_eigen->associateMatrixToTag(jacobian, tag);

  computeJacobianTags(_fe_matrix_tags);

  _nl_eigen->disassociateMatrixFromTag(jacobian, tag);
}

void
EigenProblem::computeJacobianAB(const NumericVector<Number> & soln,
                                SparseMatrix<Number> & jacobianA,
                                SparseMatrix<Number> & jacobianB,
                                TagID tagA,
                                TagID tagB)
{
  TIME_SECTION(_compute_jacobian_ab_timer);

  _fe_matrix_tags.clear();

  _fe_matrix_tags.insert(tagA);
  _fe_matrix_tags.insert(tagB);

  _nl_eigen->setSolution(soln);

  _nl_eigen->disassociateAllTaggedMatrices();
  _nl_eigen->associateMatrixToTag(jacobianA, tagA);
  _nl_eigen->associateMatrixToTag(jacobianB, tagB);

  computeJacobianTags(_fe_matrix_tags);

  _nl_eigen->disassociateMatrixFromTag(jacobianA, tagA);
  _nl_eigen->disassociateMatrixFromTag(jacobianB, tagB);
}

void
EigenProblem::computeResidualTag(const NumericVector<Number> & soln,
                                 NumericVector<Number> & residual,
                                 TagID tag)
{
  TIME_SECTION(_compute_residual_tag_timer);

  _fe_vector_tags.clear();

  _fe_vector_tags.insert(tag);

  _nl_eigen->setSolution(soln);

  _nl_eigen->disassociateAllTaggedVectors();

  _nl_eigen->associateVectorToTag(residual, tag);

  computeResidualTags(_fe_vector_tags);

  _nl_eigen->disassociateVectorFromTag(residual, tag);
}

void
EigenProblem::computeResidualAB(const NumericVector<Number> & soln,
                                NumericVector<Number> & residualA,
                                NumericVector<Number> & residualB,
                                TagID tagA,
                                TagID tagB)
{
  TIME_SECTION(_compute_residual_ab_timer);

  _fe_vector_tags.clear();

  _fe_vector_tags.insert(tagA);

  _fe_vector_tags.insert(tagB);

  _nl_eigen->setSolution(soln);

  _nl_eigen->disassociateAllTaggedVectors();

  _nl_eigen->associateVectorToTag(residualA, tagA);

  _nl_eigen->associateVectorToTag(residualB, tagB);

  computeResidualTags(_fe_vector_tags);

  _nl_eigen->disassociateVectorFromTag(residualA, tagA);

  _nl_eigen->disassociateVectorFromTag(residualB, tagB);
}

Real
EigenProblem::computeResidualL2Norm()
{
  computeResidualAB(*_nl_eigen->currentSolution(), _nl_eigen->ResidualVectorAX(), _nl_eigen->ResidualVectorBX(), _nl_eigen->nonEigenVectorTag(), _nl_eigen->eigenVectorTag());

  Real eigenvalue = 1.0;

  if (_active_eigen_index<_nl_eigen->getNumConvergedEigenvalues())
    eigenvalue = _nl_eigen->getNthConvergedEigenvalue(_active_eigen_index).first;

  // Scale BX with eigenvalue
  _nl_eigen->ResidualVectorBX() *= eigenvalue;

  // Compute entire residual
  _nl_eigen->ResidualVectorAX() -= _nl_eigen->ResidualVectorBX();

  return _nl_eigen->ResidualVectorAX().l2_norm();
}

#endif

void
EigenProblem::checkProblemIntegrity()
{
  FEProblemBase::checkProblemIntegrity();
  _nl_eigen->checkIntegrity();
}

void
EigenProblem::solve()
{
#if LIBMESH_HAVE_SLEPC
  Moose::SlepcSupport::slepcSetOptions(*this, _pars);
#endif
  if (_solve)
  {
    TIME_SECTION(_solve_timer);

    _nl->solve();
    _nl->update();
  }

  // sync solutions in displaced problem
  if (_displaced_problem)
    _displaced_problem->syncSolutions();
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
