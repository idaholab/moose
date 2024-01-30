//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearSystem.h"
#include "AuxiliarySystem.h"
#include "Problem.h"
#include "FEProblem.h"
#include "PetscSupport.h"
#include "Factory.h"
#include "ParallelUniqueId.h"
#include "ComputeLinearFVGreenGaussGradientFaceThread.h"
#include "ComputeLinearFVGreenGaussGradientVolumeThread.h"
#include "ComputeLinearFVElementalThread.h"
#include "ComputeLinearFVFaceThread.h"
#include "DisplacedProblem.h"
#include "Parser.h"
#include "MooseMesh.h"
#include "MooseUtils.h"
#include "MooseApp.h"
#include "TimeIntegrator.h"
#include "Assembly.h"
#include "FloatingPointExceptionGuard.h"
#include "Moose.h"
#include "ConsoleStream.h"
#include "MooseError.h"
#include "LinearFVKernel.h"
#include "UserObject.h"
#include "SolutionInvalidity.h"

// libMesh
#include "libmesh/linear_solver.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/dense_vector.h"
#include "libmesh/boundary_info.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_nonlinear_solver.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/mesh.h"
#include "libmesh/dense_subvector.h"
#include "libmesh/dense_submatrix.h"
#include "libmesh/dof_map.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/default_coupling.h"
#include "libmesh/diagonal_matrix.h"

#include <ios>

namespace Moose
{
void
compute_linear_system(libMesh::EquationSystems & es, const std::string & system_name)
{
  FEProblemBase * p = es.parameters.get<FEProblemBase *>("_fe_problem_base");
  auto & sys = p->getLinearSystem(p->linearSysNum(system_name));
  auto & lin_sys = sys.linearImplicitSystem();
  auto & matrix = *(sys.linearImplicitSystem().matrix);
  auto & rhs = *(sys.linearImplicitSystem().rhs);
  p->computeLinearSystemSys(lin_sys, matrix, rhs);
}
}

LinearSystem::LinearSystem(FEProblemBase & fe_problem, const std::string & name)
  : SystemBase(fe_problem, name, Moose::VAR_LINEAR),
    PerfGraphInterface(fe_problem.getMooseApp().perfGraph(), "LinearSystem"),
    _fe_problem(fe_problem),
    _sys(fe_problem.es().add_system<LinearImplicitSystem>(name)),
    _current_solution(NULL),
    _rhs_time_tag(-1),
    _rhs_time(NULL),
    _rhs_non_time_tag(-1),
    _rhs_non_time(NULL),
    _pc_side(Moose::PCS_DEFAULT),
    _ksp_norm(Moose::KSPN_UNPRECONDITIONED),
    _n_linear_iters(0),
    _linear_implicit_system(fe_problem.es().get_system<LinearImplicitSystem>(name)),
    _solution_is_invalid(false)
{
  getRightHandSideNonTimeVector();
  // Don't need to add the matrix - it already exists (for now)
  _system_matrix_tag = _fe_problem.addMatrixTag("SYSTEM");

  // We create a tag for the right hand side, the vector is already in the libmesh system
  _rhs_tag = _fe_problem.addVectorTag("RHS");

  _linear_implicit_system.attach_assemble_function(Moose::compute_linear_system);
}

LinearSystem::~LinearSystem() = default;

void
LinearSystem::init()
{
  SystemBase::init();

  _current_solution = _sys.current_local_solution.get();
}

void
LinearSystem::restoreSolutions()
{
  // call parent
  SystemBase::restoreSolutions();
  // and update _current_solution
  _current_solution = _sys.current_local_solution.get();
}

void
LinearSystem::addTimeIntegrator(const std::string & /*type*/,
                                const std::string & /*name*/,
                                InputParameters & /*parameters*/)
{
  mooseError("LinearSystem doesn ot support timeintegrators yet!");
}

void
LinearSystem::computeRightHandSideTags(const std::set<TagID> & tags)
{
  parallel_object_only();

  TIME_SECTION("nl::computeRightHandSideTags", 5);

  _fe_problem.setCurrentLinearSystem(_fe_problem.linearSysNum(name()));

  // not suppose to do anythin on matrix
  deactiveAllMatrixTags();

  FloatingPointExceptionGuard fpe_guard(_app);

  try
  {
    zeroTaggedVectors(tags);
    computeRightHandSideInternal(tags);
    closeTaggedVectors(tags);
  }
  catch (MooseException & e)
  {
    // The buck stops here, we have already handled the exception by
    // calling stopSolve(), it is now up to PETSc to return a
    // "diverged" reason during the next solve.
  }

  // not supposed to do anything on matrix
  activeAllMatrixTags();
}

void
LinearSystem::computeRightHandSideInternal(const std::set<TagID> & tags)
{
  parallel_object_only();

  TIME_SECTION("computeRightHandSideInternal", 3);

  const auto vector_tag_data = _fe_problem.getVectorTags(tags);

  // residual contributions from the domain
  PARALLEL_TRY
  {
    TIME_SECTION("LinearFVKernels_RightHandSide", 3 /*, "Computing LinearFVKernels"*/);

    using ElemInfoRange = StoredRange<MooseMesh::const_elem_info_iterator, const ElemInfo *>;
    ElemInfoRange elem_info_range(_fe_problem.mesh().ownedElemInfoBegin(),
                                  _fe_problem.mesh().ownedElemInfoEnd());

    using FaceInfoRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
    FaceInfoRange face_info_range(_fe_problem.mesh().ownedFaceInfoBegin(),
                                  _fe_problem.mesh().ownedFaceInfoEnd());

    ComputeLinearFVGreenGaussGradientFaceThread gradient_face_thread(
        _fe_problem, _fe_problem.linearSysNum(name()));
    Threads::parallel_reduce(face_info_range, gradient_face_thread);

    ComputeLinearFVGreenGaussGradientVolumeThread gradient_volume_thread(
        _fe_problem, _fe_problem.linearSysNum(name()));
    Threads::parallel_reduce(elem_info_range, gradient_volume_thread);

    ComputeLinearFVElementalThread elem_thread(_fe_problem,
                                               _fe_problem.linearSysNum(name()),
                                               Moose::FV::LinearFVComputationMode::RHS,
                                               tags);
    Threads::parallel_reduce(elem_info_range, elem_thread);

    ComputeLinearFVFaceThread face_thread(_fe_problem,
                                          _fe_problem.linearSysNum(name()),
                                          Moose::FV::LinearFVComputationMode::RHS,
                                          tags);
    Threads::parallel_reduce(face_info_range, face_thread);
  }
  PARALLEL_CATCH;

  // Accumulate the occurrence of solution invalid warnings for the current iteration cumulative
  // counters
  _app.solutionInvalidity().solutionInvalidAccumulation();
}

void
LinearSystem::computeSystemMatrixTags(const std::set<TagID> & tags)
{
  parallel_object_only();

  TIME_SECTION("LinearSystem::computeJacobianTags", 5);

  _fe_problem.setCurrentLinearSystem(_fe_problem.linearSysNum(name()));

  FloatingPointExceptionGuard fpe_guard(_app);

  try
  {
    computeSystemMatrixInternal(tags);
  }
  catch (MooseException & e)
  {
    // The buck stops here, we have already handled the exception by
    // calling stopSolve(), it is now up to PETSc to return a
    // "diverged" reason during the next solve.
  }
}

void
LinearSystem::computeSystemMatrixInternal(const std::set<TagID> & tags)
{
  parallel_object_only();

  TIME_SECTION("LinearSystem::computeSystemMatrixInternal", 3);

  // Make matrix ready to use
  activeAllMatrixTags();

  for (auto tag : tags)
  {
    if (!hasMatrix(tag))
      continue;

    auto & tagged_matrix = getMatrix(tag);
    // Necessary for speed
    if (auto petsc_matrix = dynamic_cast<PetscMatrix<Number> *>(&tagged_matrix))
    {
      MatSetOption(petsc_matrix->mat(),
                   MAT_KEEP_NONZERO_PATTERN, // This is changed in 3.1
                   PETSC_TRUE);
      if (!_fe_problem.errorOnJacobianNonzeroReallocation())
        MatSetOption(petsc_matrix->mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE);
    }
  }

  PARALLEL_TRY
  {
    TIME_SECTION("LinearFVKernels_systemMatrix", 3 /*, "Computing LinearFVKernels"*/);

    using ElemInfoRange = StoredRange<MooseMesh::const_elem_info_iterator, const ElemInfo *>;
    ElemInfoRange elem_info_range(_fe_problem.mesh().ownedElemInfoBegin(),
                                  _fe_problem.mesh().ownedElemInfoEnd());

    using FaceInfoRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
    FaceInfoRange face_info_range(_fe_problem.mesh().ownedFaceInfoBegin(),
                                  _fe_problem.mesh().ownedFaceInfoEnd());

    ComputeLinearFVGreenGaussGradientFaceThread gradient_face_thread(
        _fe_problem, _fe_problem.linearSysNum(name()));
    Threads::parallel_reduce(face_info_range, gradient_face_thread);

    ComputeLinearFVGreenGaussGradientVolumeThread gradient_volume_thread(
        _fe_problem, _fe_problem.linearSysNum(name()));
    Threads::parallel_reduce(elem_info_range, gradient_volume_thread);

    ComputeLinearFVElementalThread elem_thread(_fe_problem,
                                               _fe_problem.linearSysNum(name()),
                                               Moose::FV::LinearFVComputationMode::Matrix,
                                               tags);
    Threads::parallel_reduce(elem_info_range, elem_thread);

    ComputeLinearFVFaceThread face_thread(_fe_problem,
                                          _fe_problem.linearSysNum(name()),
                                          Moose::FV::LinearFVComputationMode::Matrix,
                                          tags);
    Threads::parallel_reduce(face_info_range, face_thread);
  }
  PARALLEL_CATCH;

  closeTaggedMatrices(tags);

  // Accumulate the occurrence of solution invalid warnings for the current iteration cumulative
  // counters
  _app.solutionInvalidity().solutionInvalidAccumulation();
}

void
LinearSystem::computeLinearSystemTags(const std::set<TagID> & vector_tags,
                                      const std::set<TagID> & matrix_tags)
{
  parallel_object_only();

  TIME_SECTION("LinearSystem::computeLinearSystemTags", 5);

  _fe_problem.setCurrentLinearSystem(_fe_problem.linearSysNum(name()));

  FloatingPointExceptionGuard fpe_guard(_app);

  try
  {
    computeLinearSystemInternal(vector_tags, matrix_tags);
  }
  catch (MooseException & e)
  {
    // The buck stops here, we have already handled the exception by
    // calling stopSolve(), it is now up to PETSc to return a
    // "diverged" reason during the next solve.
  }
}

void
LinearSystem::computeLinearSystemInternal(const std::set<TagID> & vector_tags,
                                          const std::set<TagID> & matrix_tags)
{
  TIME_SECTION("computeLinearSystemInternal", 3);

  // Make matrix ready to use
  activeAllMatrixTags();

  for (auto tag : matrix_tags)
  {
    auto & matrix = getMatrix(tag);
    // Necessary for speed
    if (auto petsc_matrix = dynamic_cast<PetscMatrix<Number> *>(&matrix))
    {
      MatSetOption(petsc_matrix->mat(),
                   MAT_KEEP_NONZERO_PATTERN, // This is changed in 3.1
                   PETSC_TRUE);
      if (!_fe_problem.errorOnJacobianNonzeroReallocation())
        MatSetOption(petsc_matrix->mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE);
    }
  }

  // residual contributions from the domain
  PARALLEL_TRY
  {
    TIME_SECTION("LinearFVKernels_FullSystem", 3 /*, "Computing LinearFVKernels"*/);

    using ElemInfoRange = StoredRange<MooseMesh::const_elem_info_iterator, const ElemInfo *>;
    ElemInfoRange elem_info_range(_fe_problem.mesh().ownedElemInfoBegin(),
                                  _fe_problem.mesh().ownedElemInfoEnd());

    using FaceInfoRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
    FaceInfoRange face_info_range(_fe_problem.mesh().ownedFaceInfoBegin(),
                                  _fe_problem.mesh().ownedFaceInfoEnd());

    ComputeLinearFVGreenGaussGradientFaceThread gradient_face_thread(
        _fe_problem, _fe_problem.linearSysNum(name()));
    Threads::parallel_reduce(face_info_range, gradient_face_thread);

    ComputeLinearFVGreenGaussGradientVolumeThread gradient_volume_thread(
        _fe_problem, _fe_problem.linearSysNum(name()));
    Threads::parallel_reduce(elem_info_range, gradient_volume_thread);

    ComputeLinearFVElementalThread elem_thread(_fe_problem,
                                               _fe_problem.linearSysNum(name()),
                                               Moose::FV::LinearFVComputationMode::FullSystem,
                                               vector_tags);
    Threads::parallel_reduce(elem_info_range, elem_thread);

    ComputeLinearFVFaceThread face_thread(_fe_problem,
                                          _fe_problem.linearSysNum(name()),
                                          Moose::FV::LinearFVComputationMode::FullSystem,
                                          vector_tags);
    Threads::parallel_reduce(face_info_range, face_thread);
  }
  PARALLEL_CATCH;

  closeTaggedMatrices(matrix_tags);

  _linear_implicit_system.matrix->close();
  _linear_implicit_system.rhs->close();

  // _linear_implicit_system.matrix->print();
  // _linear_implicit_system.rhs->print();

  // Accumulate the occurrence of solution invalid warnings for the current iteration cumulative
  // counters
  _app.solutionInvalidity().solutionInvalidAccumulation();
}

NumericVector<Number> &
LinearSystem::getRightHandSideTimeVector()
{
  return *_rhs_time;
}

NumericVector<Number> &
LinearSystem::getRightHandSideNonTimeVector()
{
  return *_rhs_non_time;
}

void
LinearSystem::augmentSparsity(SparsityPattern::Graph & /*sparsity*/,
                              std::vector<dof_id_type> & /*n_nz*/,
                              std::vector<dof_id_type> & /*n_oz*/)
{
  mooseError("LinearSystem does not support AugmentSparsity!");
}

void
LinearSystem::serializeSolution()
{
  if (_serialized_solution.get())
  {
    if (!_serialized_solution->initialized() || _serialized_solution->size() != _sys.n_dofs())
    {
      _serialized_solution->clear();
      _serialized_solution->init(_sys.n_dofs(), false, SERIAL);
    }

    _current_solution->localize(*_serialized_solution);
  }
}

NumericVector<Number> &
LinearSystem::serializedSolution()
{
  if (!_serialized_solution.get())
  {
    _serialized_solution = NumericVector<Number>::build(_communicator);
    _serialized_solution->init(_sys.n_dofs(), false, SERIAL);
  }

  return *_serialized_solution;
}

void
LinearSystem::setSolution(const NumericVector<Number> & soln)
{
  _current_solution = &soln;

  auto tag = _subproblem.getVectorTagID(Moose::SOLUTION_TAG);
  associateVectorToTag(const_cast<NumericVector<Number> &>(soln), tag);

  if (_serialized_solution.get())
    serializeSolution();
}

void
LinearSystem::checkKernelCoverage(const std::set<SubdomainID> & /*mesh_subdomains*/) const
{
}

void
LinearSystem::setPCSide(MooseEnum pcs)
{
  if (pcs == "left")
    _pc_side = Moose::PCS_LEFT;
  else if (pcs == "right")
    _pc_side = Moose::PCS_RIGHT;
  else if (pcs == "symmetric")
    _pc_side = Moose::PCS_SYMMETRIC;
  else if (pcs == "default")
    _pc_side = Moose::PCS_DEFAULT;
  else
    mooseError("Unknown PC side specified.");
}

void
LinearSystem::setMooseKSPNormType(MooseEnum kspnorm)
{
  if (kspnorm == "none")
    _ksp_norm = Moose::KSPN_NONE;
  else if (kspnorm == "preconditioned")
    _ksp_norm = Moose::KSPN_PRECONDITIONED;
  else if (kspnorm == "unpreconditioned")
    _ksp_norm = Moose::KSPN_UNPRECONDITIONED;
  else if (kspnorm == "natural")
    _ksp_norm = Moose::KSPN_NATURAL;
  else if (kspnorm == "default")
    _ksp_norm = Moose::KSPN_DEFAULT;
  else
    mooseError("Unknown ksp norm type specified.");
}

void
LinearSystem::solve()
{
  TIME_SECTION("LinearSystem::solve", 2, "Solving linear system");

  // Clear the iteration counters
  _current_l_its = 0;

  system().solve();

  _n_linear_iters = _linear_implicit_system.n_linear_iterations();

  // store info about the solve
  _final_linear_residual = _linear_implicit_system.final_linear_residual();

  // determine whether solution invalid occurs in the converged solution
  _solution_is_invalid = _app.solutionInvalidity().solutionInvalid();

  // output the solution invalid summary
  if (_solution_is_invalid)
  {
    // sync all solution invalid counts to rank 0 process
    _app.solutionInvalidity().sync();

    if (_fe_problem.allowInvalidSolution())
      mooseWarning("The Solution Invalidity warnings are detected but silenced! "
                   "Use Problem/allow_invalid_solution=false to activate ");
    else
      // output the occurrence of solution invalid in a summary table
      _app.solutionInvalidity().print(_console);
  }
}
