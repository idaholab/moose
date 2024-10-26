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
#include "MooseVariableScalar.h"
#include "UserObject.h"

// libMesh includes
#include "libmesh/system.h"
#include "libmesh/eigen_solver.h"
#include "libmesh/enum_eigen_solver_type.h"

// Needed for LIBMESH_CHECK_ERR
using libMesh::PetscSolverException;

registerMooseObject("MooseApp", EigenProblem);

InputParameters
EigenProblem::validParams()
{
  InputParameters params = FEProblemBase::validParams();
  params.addClassDescription("Problem object for solving an eigenvalue problem.");
  params.addParam<bool>("negative_sign_eigen_kernel",
                        true,
                        "Whether or not to use a negative sign for eigenvalue kernels. "
                        "Using a negative sign makes eigenvalue kernels consistent with "
                        "a nonlinear solver");

  params.addParam<unsigned int>(
      "active_eigen_index",
      0,
      "Which eigenvector is used to compute residual and also associated to nonlinear variable");
  params.addParam<PostprocessorName>("bx_norm", "A postprocessor describing the norm of Bx");

  params.addParamNamesToGroup("negative_sign_eigen_kernel active_eigen_index bx_norm",
                              "Eigenvalue solve");

  return params;
}

EigenProblem::EigenProblem(const InputParameters & parameters)
  : FEProblemBase(parameters)
#ifdef LIBMESH_HAVE_SLEPC
    ,
    // By default, we want to compute an eigenvalue only (smallest or largest)
    _n_eigen_pairs_required(1),
    _generalized_eigenvalue_problem(false),
    _negative_sign_eigen_kernel(getParam<bool>("negative_sign_eigen_kernel")),
    _active_eigen_index(getParam<unsigned int>("active_eigen_index")),
    _do_free_power_iteration(false),
    _output_inverse_eigenvalue(false),
    _on_linear_solver(false),
    _matrices_formed(false),
    _constant_matrices(false),
    _has_normalization(false),
    _normal_factor(1.0),
    _first_solve(declareRestartableData<bool>("first_solve", true)),
    _bx_norm_name(isParamValid("bx_norm")
                      ? std::make_optional(getParam<PostprocessorName>("bx_norm"))
                      : std::nullopt)
#endif
{
#ifdef LIBMESH_HAVE_SLEPC
  if (_nl_sys_names.size() > 1)
    paramError("nl_sys_names",
               "eigen problems do not currently support multiple nonlinear eigen systems");

  for (const auto i : index_range(_nl_sys_names))
  {
    const auto & sys_name = _nl_sys_names[i];
    auto & nl = _nl[i];
    nl = std::make_shared<NonlinearEigenSystem>(*this, sys_name);
    _nl_eigen = std::dynamic_pointer_cast<NonlinearEigenSystem>(nl);
    _current_nl_sys = nl.get();
    _solver_systems[i] = std::dynamic_pointer_cast<SolverSystem>(nl);
  }

  _aux = std::make_shared<AuxiliarySystem>(*this, "aux0");

  newAssemblyArray(_solver_systems);

  FEProblemBase::initNullSpaceVectors(parameters, _nl);

  es().parameters.set<EigenProblem *>("_eigen_problem") = this;
#else
  mooseError("Need to install SLEPc to solve eigenvalue problems, please reconfigure\n");
#endif /* LIBMESH_HAVE_SLEPC */

  // SLEPc older than 3.13.0 can not take initial guess from moose
#if PETSC_RELEASE_LESS_THAN(3, 13, 0)
  mooseDeprecated(
      "Please use SLEPc-3.13.0 or higher. Old versions of SLEPc likely produce bad convergence");
#endif
  // Create extra vectors and matrices if any
  createTagVectors();

  // Create extra solution vectors if any
  createTagSolutions();
}

#ifdef LIBMESH_HAVE_SLEPC
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
EigenProblem::execute(const ExecFlagType & exec_type)
{
  if (exec_type == EXEC_INITIAL && !_app.isRestarting())
    // we need to scale the solution properly and we can do this only all initial setup of
    // depending objects by the residual evaluations has been done to this point.
    preScaleEigenVector(std::pair<Real, Real>(_initial_eigenvalue, 0));

  FEProblemBase::execute(exec_type);
}

void
EigenProblem::computeJacobianTag(const NumericVector<Number> & soln,
                                 SparseMatrix<Number> & jacobian,
                                 TagID tag)
{
  TIME_SECTION("computeJacobianTag", 3);

  // Disassociate the default tags because we will associate vectors with only the
  // specific system tags that we need for this instance
  _nl_eigen->disassociateDefaultMatrixTags();

  // Clear FE tags and first add the specific tag associated with the Jacobian
  _fe_matrix_tags.clear();
  _fe_matrix_tags.insert(tag);

  // Add any other user-added matrix tags if they have associated matrices
  const auto & matrix_tags = getMatrixTags();
  for (const auto & matrix_tag : matrix_tags)
    if (_nl_eigen->hasMatrix(matrix_tag.second))
      _fe_matrix_tags.insert(matrix_tag.second);

  _nl_eigen->setSolution(soln);

  _nl_eigen->associateMatrixToTag(jacobian, tag);

  setCurrentNonlinearSystem(_nl_eigen->number());
  computeJacobianTags(_fe_matrix_tags);

  _nl_eigen->disassociateMatrixFromTag(jacobian, tag);
}

void
EigenProblem::computeMatricesTags(const NumericVector<Number> & soln,
                                  const std::vector<SparseMatrix<Number> *> & jacobians,
                                  const std::set<TagID> & tags)
{
  TIME_SECTION("computeMatricesTags", 3);

  if (jacobians.size() != tags.size())
    mooseError("The number of matrices ",
               jacobians.size(),
               " does not equal the number of tags ",
               tags.size());

  // Disassociate the default tags because we will associate vectors with only the
  // specific system tags that we need for this instance
  _nl_eigen->disassociateDefaultMatrixTags();

  _fe_matrix_tags.clear();

  _nl_eigen->setSolution(soln);

  unsigned int i = 0;
  for (auto tag : tags)
    _nl_eigen->associateMatrixToTag(*(jacobians[i++]), tag);

  setCurrentNonlinearSystem(_nl_eigen->number());
  computeJacobianTags(tags);

  i = 0;
  for (auto tag : tags)
    _nl_eigen->disassociateMatrixFromTag(*(jacobians[i++]), tag);
}

void
EigenProblem::computeJacobianBlocks(std::vector<JacobianBlock *> & blocks,
                                    const unsigned int nl_sys_num)
{
  TIME_SECTION("computeJacobianBlocks", 3);
  setCurrentNonlinearSystem(nl_sys_num);

  if (_displaced_problem)
    computeSystems(EXEC_PRE_DISPLACE);

  computeSystems(EXEC_NONLINEAR);

  _currently_computing_jacobian = true;

  _current_nl_sys->computeJacobianBlocks(blocks, {_nl_eigen->precondMatrixTag()});

  _currently_computing_jacobian = false;
}

void
EigenProblem::computeJacobianAB(const NumericVector<Number> & soln,
                                SparseMatrix<Number> & jacobianA,
                                SparseMatrix<Number> & jacobianB,
                                TagID tagA,
                                TagID tagB)
{
  TIME_SECTION("computeJacobianAB", 3);

  // Disassociate the default tags because we will associate vectors with only the
  // specific system tags that we need for this instance
  _nl_eigen->disassociateDefaultMatrixTags();

  // Clear FE tags and first add the specific tags associated with the Jacobian
  _fe_matrix_tags.clear();
  _fe_matrix_tags.insert(tagA);
  _fe_matrix_tags.insert(tagB);

  // Add any other user-added matrix tags if they have associated matrices
  const auto & matrix_tags = getMatrixTags();
  for (const auto & matrix_tag : matrix_tags)
    if (_nl_eigen->hasMatrix(matrix_tag.second))
      _fe_matrix_tags.insert(matrix_tag.second);

  _nl_eigen->setSolution(soln);

  _nl_eigen->associateMatrixToTag(jacobianA, tagA);
  _nl_eigen->associateMatrixToTag(jacobianB, tagB);

  setCurrentNonlinearSystem(_nl_eigen->number());
  computeJacobianTags(_fe_matrix_tags);

  _nl_eigen->disassociateMatrixFromTag(jacobianA, tagA);
  _nl_eigen->disassociateMatrixFromTag(jacobianB, tagB);
}

void
EigenProblem::computeResidualTag(const NumericVector<Number> & soln,
                                 NumericVector<Number> & residual,
                                 TagID tag)
{
  TIME_SECTION("computeResidualTag", 3);

  // Disassociate the default tags because we will associate vectors with only the
  // specific system tags that we need for this instance
  _nl_eigen->disassociateDefaultVectorTags();

  // add the specific tag associated with the residual
  mooseAssert(_fe_vector_tags.empty(), "This should be empty indicating a clean starting state");
  _fe_vector_tags.insert(tag);

  // Add any other user-added vector residual tags if they have associated vectors
  const auto & residual_vector_tags = getVectorTags(Moose::VECTOR_TAG_RESIDUAL);
  for (const auto & vector_tag : residual_vector_tags)
    if (_nl_eigen->hasVector(vector_tag._id))
      _fe_vector_tags.insert(vector_tag._id);

  _nl_eigen->associateVectorToTag(residual, tag);

  _nl_eigen->setSolution(soln);

  setCurrentNonlinearSystem(_nl_eigen->number());
  computeResidualTags(_fe_vector_tags);
  _fe_vector_tags.clear();

  _nl_eigen->disassociateVectorFromTag(residual, tag);
}

void
EigenProblem::computeResidualAB(const NumericVector<Number> & soln,
                                NumericVector<Number> & residualA,
                                NumericVector<Number> & residualB,
                                TagID tagA,
                                TagID tagB)
{
  TIME_SECTION("computeResidualAB", 3);

  // Disassociate the default tags because we will associate vectors with only the
  // specific system tags that we need for this instance
  _nl_eigen->disassociateDefaultVectorTags();

  // add the specific tags associated with the residual
  mooseAssert(_fe_vector_tags.empty(), "This should be empty indicating a clean starting state");
  _fe_vector_tags.insert(tagA);
  _fe_vector_tags.insert(tagB);

  // Add any other user-added vector residual tags if they have associated vectors
  const auto & residual_vector_tags = getVectorTags(Moose::VECTOR_TAG_RESIDUAL);
  for (const auto & vector_tag : residual_vector_tags)
    if (_nl_eigen->hasVector(vector_tag._id))
      _fe_vector_tags.insert(vector_tag._id);

  _nl_eigen->associateVectorToTag(residualA, tagA);
  _nl_eigen->associateVectorToTag(residualB, tagB);

  _nl_eigen->setSolution(soln);

  computeResidualTags(_fe_vector_tags);
  _fe_vector_tags.clear();

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
EigenProblem::adjustEigenVector(const Real value, bool scaling)
{
  std::vector<VariableName> var_names = getVariableNames();
  for (auto & vn : var_names)
  {
    MooseVariableBase * var = nullptr;
    if (hasScalarVariable(vn))
      var = &getScalarVariable(0, vn);
    else
      var = &getVariable(0, vn);
    // Do operations for only eigen variable
    if (var->eigen())
      for (unsigned int vc = 0; vc < var->count(); ++vc)
      {
        std::set<dof_id_type> var_indices;
        _nl_eigen->system().local_dof_indices(var->number() + vc, var_indices);
        for (const auto & dof : var_indices)
          _nl_eigen->solution().set(dof, scaling ? (_nl_eigen->solution()(dof) * value) : value);
      }
  }

  _nl_eigen->solution().close();
  _nl_eigen->update();
}

void
EigenProblem::scaleEigenvector(const Real scaling_factor)
{
  adjustEigenVector(scaling_factor, true);
}

void
EigenProblem::initEigenvector(const Real initial_value)
{
  // Yaqi's note: the following code will set a flat solution for lagrange and
  // constant monomial variables. For the first or higher order elemental variables,
  // the solution is not flat. Fortunately, the initial guess does not affect
  // the final solution as long as it is not perpendicular to the true solution.
  // We, in general, do not need to worry about that.

  adjustEigenVector(initial_value, false);
}

void
EigenProblem::preScaleEigenVector(const std::pair<Real, Real> & eig)
{
  // pre-scale the solution to make sure ||Bx||_2 is equal to inverse of eigenvalue
  computeResidualTag(
      *_nl_eigen->currentSolution(), _nl_eigen->residualVectorBX(), _nl_eigen->eigenVectorTag());

  // Eigenvalue magnitude
  Real v = std::sqrt(eig.first * eig.first + eig.second * eig.second);
  // Scaling factor
  Real factor = 1 / v / (bxNormProvided() ? formNorm() : _nl_eigen->residualVectorBX().l2_norm());
  // Scale eigenvector
  if (!MooseUtils::absoluteFuzzyEqual(factor, 1))
    scaleEigenvector(factor);
}

void
EigenProblem::postScaleEigenVector()
{
  if (_has_normalization)
  {
    Real v;
    if (_normal_factor == std::numeric_limits<Real>::max())
    {
      if (_active_eigen_index >= _nl_eigen->getNumConvergedEigenvalues())
        mooseError("Number of converged eigenvalues ",
                   _nl_eigen->getNumConvergedEigenvalues(),
                   " but you required eigenvalue ",
                   _active_eigen_index);

      // when normal factor is not provided, we use the inverse of the norm of
      // the active eigenvalue for normalization
      auto eig = _nl_eigen->getAllConvergedEigenvalues()[_active_eigen_index];
      v = 1 / std::sqrt(eig.first * eig.first + eig.second * eig.second);
    }
    else
      v = _normal_factor;

    Real c = getPostprocessorValueByName(_normalization);

    // We scale SLEPc eigen vector here, so we need to scale it back for optimal
    // convergence if we call EPS solver again
    mooseAssert(v != 0., "normal factor can not be zero");

    unsigned int itr = 0;

    while (!MooseUtils::relativeFuzzyEqual(v, c))
    {
      // If postprocessor is not defined on eigen variables, scaling might not work
      if (itr > 10)
        mooseError("Can not scale eigenvector to the required factor ",
                   v,
                   " please check if postprocessor is defined on only eigen variables");

      mooseAssert(c != 0., "postprocessor value used for scaling can not be zero");

      scaleEigenvector(v / c);

      // update all aux variables and user objects on linear
      execute(EXEC_LINEAR);

      c = getPostprocessorValueByName(_normalization);

      itr++;
    }
  }
}

void
EigenProblem::checkProblemIntegrity()
{
  FEProblemBase::checkProblemIntegrity();
  _nl_eigen->checkIntegrity();
  if (_bx_norm_name)
  {
    if (!isNonlinearEigenvalueSolver())
      paramWarning("bx_norm", "This parameter is only used for nonlinear solve types");
    else if (auto & pp = getUserObjectBase(_bx_norm_name.value());
             !pp.getExecuteOnEnum().contains(EXEC_LINEAR))
      pp.paramError("execute_on",
                    "If providing the Bx norm, this postprocessor must execute on linear e.g. "
                    "during residual evaluations");
  }
}

void
EigenProblem::doFreeNonlinearPowerIterations(unsigned int free_power_iterations)
{
  mooseAssert(_current_nl_sys, "This needs to be non-null");

  doFreePowerIteration(true);
  // Set free power iterations
  Moose::SlepcSupport::setFreeNonlinearPowerIterations(free_power_iterations);

  // Call solver
  _current_nl_sys->solve();
  _current_nl_sys->update();

  // Clear free power iterations
  auto executioner = getMooseApp().getExecutioner();
  if (executioner)
    Moose::SlepcSupport::clearFreeNonlinearPowerIterations(executioner->parameters());
  else
    mooseError("There is no executioner for this moose app");

  doFreePowerIteration(false);
}

void
EigenProblem::solve(const unsigned int nl_sys_num)
{
#if !PETSC_RELEASE_LESS_THAN(3, 12, 0)
  // Master has the default database
  if (!_app.isUltimateMaster())
    LibmeshPetscCall(PetscOptionsPush(_petsc_option_data_base));
#endif

  setCurrentNonlinearSystem(nl_sys_num);

  if (_solve)
  {
    TIME_SECTION("solve", 1);

    // Set necessary slepc callbacks
    // We delay this call as much as possible because libmesh
    // could rebuild matrices due to mesh changes or something else.
    _nl_eigen->attachSLEPcCallbacks();

    // If there is an eigenvalue, we scale 1/|Bx| to eigenvalue
    if (_active_eigen_index < _nl_eigen->getNumConvergedEigenvalues())
    {
      std::pair<Real, Real> eig = _nl_eigen->getConvergedEigenvalue(_active_eigen_index);
      preScaleEigenVector(eig);
    }

    if (isNonlinearEigenvalueSolver() &&
        solverParams()._eigen_solve_type != Moose::EST_NONLINEAR_POWER)
    {
      // Let do an initial solve if a nonlinear eigen solver but not power is used.
      // The initial solver is a Inverse Power, and it is used to compute a good initial
      // guess for Newton
      if (solverParams()._free_power_iterations && _first_solve)
      {
        _console << std::endl << " -------------------------------" << std::endl;
        _console << " Free power iteration starts ..." << std::endl;
        _console << " -------------------------------" << std::endl << std::endl;
        doFreeNonlinearPowerIterations(solverParams()._free_power_iterations);
        _first_solve = false;
      }

      // Let us do extra power iterations here if necessary
      if (solverParams()._extra_power_iterations)
      {
        _console << std::endl << " --------------------------------------" << std::endl;
        _console << " Extra Free power iteration starts ..." << std::endl;
        _console << " --------------------------------------" << std::endl << std::endl;
        doFreeNonlinearPowerIterations(solverParams()._extra_power_iterations);
      }
    }

    // We print this for only nonlinear solver
    if (isNonlinearEigenvalueSolver())
    {
      _console << std::endl << " -------------------------------------" << std::endl;

      if (solverParams()._eigen_solve_type != Moose::EST_NONLINEAR_POWER)
        _console << " Nonlinear Newton iteration starts ..." << std::endl;
      else
        _console << " Nonlinear power iteration starts ..." << std::endl;

      _console << " -------------------------------------" << std::endl << std::endl;
    }

    _current_nl_sys->solve();
    _current_nl_sys->update();

    // with PJFNKMO solve type, we need to evaluate the linear objects to bring them up-to-date
    if (solverParams()._eigen_solve_type == Moose::EST_PJFNKMO)
      execute(EXEC_LINEAR);

    // Scale eigen vector if users ask
    postScaleEigenVector();
  }

#if !PETSC_RELEASE_LESS_THAN(3, 12, 0)
  if (!_app.isUltimateMaster())
    LibmeshPetscCall(PetscOptionsPop());
#endif

  // sync solutions in displaced problem
  if (_displaced_problem)
    _displaced_problem->syncSolutions();

  // Reset the matrix flag, so that we reform matrix in next picard iteration
  _matrices_formed = false;
}

void
EigenProblem::setNormalization(const PostprocessorName & pp, const Real value)
{
  _has_normalization = true;
  _normalization = pp;
  _normal_factor = value;
}

void
EigenProblem::init()
{
#if PETSC_RELEASE_LESS_THAN(3, 13, 0)
  // Prior to Slepc 3.13 we did not have a nonlinear eigenvalue solver so we must always assemble
  // before the solve
  _nl_eigen->sys().attach_assemble_function(Moose::assemble_matrix);
#else
  if (isNonlinearEigenvalueSolver())
    // We don't need to assemble before the solve
    _nl_eigen->sys().assemble_before_solve = false;
  else
    _nl_eigen->sys().attach_assemble_function(Moose::assemble_matrix);

  // If matrix_free=true, this tells Libmesh to use shell matrices
  _nl_eigen->sys().use_shell_matrices(solverParams()._eigen_matrix_free &&
                                      !solverParams()._eigen_matrix_vector_mult);
  // We need to tell libMesh if we are using a shell preconditioning matrix
  _nl_eigen->sys().use_shell_precond_matrix(solverParams()._precond_matrix_free);
#endif

  FEProblemBase::init();
}

bool
EigenProblem::solverSystemConverged(unsigned int)
{
  if (_solve)
    return _nl_eigen->converged();
  else
    return true;
}

bool
EigenProblem::isNonlinearEigenvalueSolver() const
{
  return solverParams()._eigen_solve_type == Moose::EST_NONLINEAR_POWER ||
         solverParams()._eigen_solve_type == Moose::EST_NEWTON ||
         solverParams()._eigen_solve_type == Moose::EST_PJFNK ||
         solverParams()._eigen_solve_type == Moose::EST_JFNK ||
         solverParams()._eigen_solve_type == Moose::EST_PJFNKMO;
}

void
EigenProblem::initPetscOutputAndSomeSolverSettings()
{
  _app.getOutputWarehouse().solveSetup();
}

Real
EigenProblem::formNorm()
{
  mooseAssert(_bx_norm_name,
              "We should not get here unless a bx_norm postprocessor has been provided");
  return getPostprocessorValueByName(*_bx_norm_name);
}
#endif
