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
#include "MooseLinearVariableFV.h"
#include "LinearFVTimeDerivative.h"

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
#include "libmesh/petsc_solver_exception.h"

#include <ios>

using namespace libMesh;

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
  : SolverSystem(fe_problem, fe_problem, name, Moose::VAR_SOLVER),
    PerfGraphInterface(fe_problem.getMooseApp().perfGraph(), "LinearSystem"),
    _sys(fe_problem.es().add_system<LinearImplicitSystem>(name)),
    _rhs_time_tag(-1),
    _rhs_time(NULL),
    _rhs_non_time_tag(-1),
    _rhs_non_time(NULL),
    _n_linear_iters(0),
    _converged(false),
    _linear_implicit_system(fe_problem.es().get_system<LinearImplicitSystem>(name))
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
LinearSystem::initialSetup()
{
  SystemBase::initialSetup();
  // Checking if somebody accidentally assigned nonlinear variables to this system
  const auto & var_names = _vars[0].names();
  for (const auto & name : var_names)
    if (!dynamic_cast<MooseLinearVariableFVReal *>(_vars[0].getVariable(name)))
      mooseError("You are trying to add a nonlinear variable to a linear system! The variable "
                 "which is assigned to the wrong system: ",
                 name);
}

void
LinearSystem::computeLinearSystemTags(const std::set<TagID> & vector_tags,
                                      const std::set<TagID> & matrix_tags,
                                      const bool compute_gradients)
{
  parallel_object_only();

  TIME_SECTION("LinearSystem::computeLinearSystemTags", 5);

  _fe_problem.setCurrentLinearSystem(_fe_problem.linearSysNum(name()));

  FloatingPointExceptionGuard fpe_guard(_app);

  try
  {
    computeLinearSystemInternal(vector_tags, matrix_tags, compute_gradients);
  }
  catch (MooseException & e)
  {
    // The buck stops here, we have already handled the exception by
    // calling stopSolve(), it is now up to PETSc to return a
    // "diverged" reason during the next solve.
  }
}

void
LinearSystem::computeGradients()
{
  _new_gradient.clear();
  for (auto & vec : _raw_grad_container)
    _new_gradient.push_back(vec->zero_clone());

  TIME_SECTION("LinearVariableFV_Gradients", 3 /*, "Computing Linear FV variable gradients"*/);

  PARALLEL_TRY
  {
    using FaceInfoRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
    FaceInfoRange face_info_range(_fe_problem.mesh().ownedFaceInfoBegin(),
                                  _fe_problem.mesh().ownedFaceInfoEnd());

    ComputeLinearFVGreenGaussGradientFaceThread gradient_face_thread(
        _fe_problem, _fe_problem.linearSysNum(name()));
    Threads::parallel_reduce(face_info_range, gradient_face_thread);
  }
  PARALLEL_CATCH;

  for (auto & vec : _new_gradient)
    vec->close();

  PARALLEL_TRY
  {
    using ElemInfoRange = StoredRange<MooseMesh::const_elem_info_iterator, const ElemInfo *>;
    ElemInfoRange elem_info_range(_fe_problem.mesh().ownedElemInfoBegin(),
                                  _fe_problem.mesh().ownedElemInfoEnd());

    ComputeLinearFVGreenGaussGradientVolumeThread gradient_volume_thread(
        _fe_problem, _fe_problem.linearSysNum(name()));
    Threads::parallel_reduce(elem_info_range, gradient_volume_thread);
  }
  PARALLEL_CATCH;

  for (const auto i : index_range(_raw_grad_container))
  {
    _raw_grad_container[i] = std::move(_new_gradient[i]);
    _raw_grad_container[i]->close();
  }
}

void
LinearSystem::computeLinearSystemInternal(const std::set<TagID> & vector_tags,
                                          const std::set<TagID> & matrix_tags,
                                          const bool compute_gradients)
{
  TIME_SECTION("computeLinearSystemInternal", 3);

  // Before we assemble we clear up the matrix and the vector
  _linear_implicit_system.matrix->zero();
  _linear_implicit_system.rhs->zero();

  // Make matrix ready to use
  activeAllMatrixTags();

  for (auto tag : matrix_tags)
  {
    auto & matrix = getMatrix(tag);
    // Necessary for speed
    if (auto petsc_matrix = dynamic_cast<PetscMatrix<Number> *>(&matrix))
    {
      LibmeshPetscCall(MatSetOption(petsc_matrix->mat(),
                                    MAT_KEEP_NONZERO_PATTERN, // This is changed in 3.1
                                    PETSC_TRUE));
      if (!_fe_problem.errorOnJacobianNonzeroReallocation())
        LibmeshPetscCall(
            MatSetOption(petsc_matrix->mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE));
    }
  }

  if (compute_gradients)
    computeGradients();

  // linear contributions from the domain
  PARALLEL_TRY
  {
    TIME_SECTION("LinearFVKernels_FullSystem", 3 /*, "Computing LinearFVKernels"*/);

    using ElemInfoRange = StoredRange<MooseMesh::const_elem_info_iterator, const ElemInfo *>;
    ElemInfoRange elem_info_range(_fe_problem.mesh().ownedElemInfoBegin(),
                                  _fe_problem.mesh().ownedElemInfoEnd());

    using FaceInfoRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
    FaceInfoRange face_info_range(_fe_problem.mesh().ownedFaceInfoBegin(),
                                  _fe_problem.mesh().ownedFaceInfoEnd());

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
LinearSystem::solve()
{
  TIME_SECTION("LinearSystem::solve", 2, "Solving linear system");

  // Clear the iteration counters
  _current_l_its = 0;

  system().solve();

  // store info about the solve
  _n_linear_iters = _linear_implicit_system.n_linear_iterations();

  auto & linear_solver =
      libMesh::cast_ref<PetscLinearSolver<Real> &>(*_linear_implicit_system.get_linear_solver());
  _initial_linear_residual = linear_solver.get_initial_residual();
  _final_linear_residual = _linear_implicit_system.final_linear_residual();
  _converged = linear_solver.get_converged_reason() > 0;

  _console << "System: " << this->name() << " Initial residual: " << _initial_linear_residual
           << " Final residual: " << _final_linear_residual << " Num. of Iter. " << _n_linear_iters
           << std::endl;

  // determine whether solution invalid occurs in the converged solution
  checkInvalidSolution();
}

void
LinearSystem::stopSolve(const ExecFlagType & /*exec_flag*/,
                        const std::set<TagID> & vector_tags_to_close)
{
  // We close the containers in case the solve restarts from a failed iteration
  closeTaggedVectors(vector_tags_to_close);
  _linear_implicit_system.matrix->close();
}

bool
LinearSystem::containsTimeKernel()
{
  // Right now, FV kernels are in TheWarehouse so we have to use that.
  std::vector<LinearFVKernel *> kernels;
  auto base_query = _fe_problem.theWarehouse()
                        .query()
                        .template condition<AttribSysNum>(this->number())
                        .template condition<AttribSystem>("LinearFVKernel")
                        .queryInto(kernels);

  bool contains_time_kernel = false;
  for (const auto kernel : kernels)
  {
    contains_time_kernel = dynamic_cast<LinearFVTimeDerivative *>(kernel);
    if (contains_time_kernel)
      break;
  }

  return contains_time_kernel;
}
