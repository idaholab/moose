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

defineLegacyParams(EigenProblem);

InputParameters
EigenProblem::validParams()
{
  InputParameters params = FEProblemBase::validParams();
  params.addParam<bool>("negative_sign_eigen_kernel",
                        true,
                        "Whether or not to use a negative sign for eigenvalue kernels. "
                        "Using a negative sign makes eigenvalue kernels consistent with "
                        "a nonlinear solver");

  params.addParam<unsigned int>(
      "active_eigen_index",
      0,
      "Which eigen vector is used to compute residual and also associateed to nonlinear variable");

  return params;
}

EigenProblem::EigenProblem(const InputParameters & parameters)
  : FEProblemBase(parameters),
    // By default, we want to compute an eigenvalue only (smallest or largest)
    _n_eigen_pairs_required(1),
    _generalized_eigenvalue_problem(false),
    _nl_eigen(std::make_shared<NonlinearEigenSystem>(*this, "eigen0")),
    _negative_sign_eigen_kernel(getParam<bool>("negative_sign_eigen_kernel")),
    _active_eigen_index(getParam<unsigned int>("active_eigen_index")),
    _compute_jacobian_tag_timer(registerTimedSection("computeJacobianTag", 3)),
    _compute_jacobian_ab_timer(registerTimedSection("computeJacobianAB", 3)),
    _compute_residual_tag_timer(registerTimedSection("computeResidualTag", 3)),
    _compute_residual_ab_timer(registerTimedSection("computeResidualAB", 3)),
    _solve_timer(registerTimedSection("solve", 1)),
    _compute_jacobian_blocks_timer(registerTimedSection("computeJacobianBlocks", 3))
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
EigenProblem::computeMatricesTags(
    const NumericVector<Number> & soln,
    const std::vector<std::unique_ptr<SparseMatrix<Number>>> & jacobians,
    const std::set<TagID> & tags)
{
  TIME_SECTION(_compute_jacobian_tag_timer);

  if (jacobians.size() != tags.size())
    mooseError("The number of matrices ",
               jacobians.size(),
               " does not equal the number of tags ",
               tags.size());

  _fe_matrix_tags.clear();

  _nl_eigen->setSolution(soln);

  _nl_eigen->disassociateAllTaggedMatrices();

  unsigned int i = 0;
  for (auto tag : tags)
    _nl_eigen->associateMatrixToTag(*(jacobians[i++]), tag);

  computeJacobianTags(tags);

  i = 0;
  for (auto tag : tags)
    _nl_eigen->disassociateMatrixFromTag(*(jacobians[i++]), tag);
}

void
EigenProblem::computeJacobianBlocks(std::vector<JacobianBlock *> & blocks)
{
  TIME_SECTION(_compute_jacobian_blocks_timer);

  if (_displaced_problem)
    _aux->compute(EXEC_PRE_DISPLACE);

  _aux->compute(EXEC_NONLINEAR);

  _currently_computing_jacobian = true;

  _nl->computeJacobianBlocks(blocks, {_nl_eigen->precondMatrixTag()});

  _currently_computing_jacobian = false;
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
  computeResidualAB(*_nl_eigen->currentSolution(),
                    _nl_eigen->residualVectorAX(),
                    _nl_eigen->residualVectorBX(),
                    _nl_eigen->nonEigenVectorTag(),
                    _nl_eigen->eigenVectorTag());

  Real eigenvalue = 1.0;

  if (_active_eigen_index < _nl_eigen->getNumConvergedEigenvalues())
    eigenvalue = _nl_eigen->getConvergedEigenvalue(_active_eigen_index).first;

  // Scale BX with eigenvalue
  _nl_eigen->residualVectorBX() *= eigenvalue;

  // Compute entire residual
  if (_negative_sign_eigen_kernel)
    _nl_eigen->residualVectorAX() += _nl_eigen->residualVectorBX();
  else
    _nl_eigen->residualVectorAX() -= _nl_eigen->residualVectorBX();

  return _nl_eigen->residualVectorAX().l2_norm();
}

void
EigenProblem::scaleEigenvector(const Real scaling_factor)
{
  std::vector<VariableName> var_names = getVariableNames();
  for (auto & vn : var_names)
  {
    MooseVariableFEBase & var = getVariable(0, vn);
    if (var.parameters().get<bool>("eigen"))
      for (unsigned int vc = 0; vc < var.count(); ++vc)
      {
        std::set<dof_id_type> var_indices;
        _nl_eigen->system().local_dof_indices(var.number() + vc, var_indices);
        for (const auto & dof : var_indices)
          _nl_eigen->solution().set(dof, _nl_eigen->solution()(dof) * scaling_factor);
      }
  }
  _nl_eigen->solution().close();
  _nl_eigen->update();
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
  // Set necessary slepc callbacks
  // We delay this call as much as possible because libmesh
  // could rebuild matrices due to mesh changes or something else.
  _nl_eigen->attachSLEPcCallbacks();
#endif

#if !PETSC_RELEASE_LESS_THAN(3, 12, 0)
  // Master has the default database
  if (!_app.isUltimateMaster())
    PetscOptionsPush(_petsc_option_data_base);
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

#if !PETSC_RELEASE_LESS_THAN(3, 12, 0)
  if (!_app.isUltimateMaster())
    PetscOptionsPop();
#endif
}

void
EigenProblem::init()
{
#if !PETSC_RELEASE_LESS_THAN(3, 13, 0) && LIBMESH_HAVE_SLEPC
  // If matrix_free=true, this tells Libmesh to use shell matrices
  _nl_eigen->sys().use_shell_matrices(solverParams()._eigen_matrix_free);
  // We need to tell libMesh if we are using a shell preconditioning matrix
  _nl_eigen->sys().use_shell_precond_matrix(solverParams()._precond_matrix_free);
#endif

  FEProblemBase::init();
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
         solverParams()._eigen_solve_type == Moose::EST_NEWTON;
}
