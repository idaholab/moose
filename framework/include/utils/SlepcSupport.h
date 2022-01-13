//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_SLEPC

#include "libmesh/petsc_macro.h"

#include "Moose.h"
#include "SystemBase.h"

/* We need this in order to implement moose PC */
#include <petsc/private/pcimpl.h>
#include <slepceps.h>
#include <slepc/private/epsimpl.h>
/* In order to use libMesh preconditioner */
#include "libmesh/linear_solver.h"
#include "libmesh/preconditioner.h"

class EigenProblem;
class InputParameters;

namespace Moose
{
namespace SlepcSupport
{
/**
 * @return InputParameters object containing the SLEPC related parameters
 *
 * The output of this function should be added to the the parameters object of the overarching class
 * @see EigenProblem
 */
InputParameters getSlepcValidParams(InputParameters & params);

/**
 * Retrieve valid params that allow users to specify eigen problem configuration
 */
InputParameters getSlepcEigenProblemValidParams();

/**
 * Set solve type into eigen problem (solverParams)
 */
void storeSolveType(FEProblemBase & fe_problem, const InputParameters & params);

/**
 * Retrieve eigen problem params from 'params', and then set these params into SolverParams
 */
void setEigenProblemSolverParams(EigenProblem & eigen_problem, const InputParameters & params);

/**
 * Push all SLEPc/PETSc options into SLEPc/PETSc side. Options could come from commandline,
 * SolverParams, params, etc.
 */
void slepcSetOptions(EigenProblem & eigen_problem, const InputParameters & params);

/**
 * Control eigen solver tolerances via SLEPc options
 */
void setSlepcEigenSolverTolerances(EigenProblem & eigen_problem, const InputParameters & params);

/**
 * Set SLEPc/PETSc options to trigger free power iteration
 */
void setFreeNonlinearPowerIterations(unsigned int free_power_iterations);

/*
 * Set SLEPc/PETSc options to turn the eigen-solver back to a regular Newton solver
 */
void clearFreeNonlinearPowerIterations(const InputParameters & params);

/**
 * Form matrix according to tag
 */
void moosePetscSNESFormMatrixTag(SNES snes, Vec x, Mat mat, void * ctx, TagID tag);

/**
 * Form multiple matrices for multiple tags
 */
void moosePetscSNESFormMatricesTags(
    SNES snes, Vec x, std::vector<Mat> & mats, void * ctx, const std::set<TagID> & tags);

/**
 * Form Jacobian matrix A. It is a SLEPc callback
 */
PetscErrorCode mooseSlepcEigenFormJacobianA(SNES snes, Vec x, Mat jac, Mat pc, void * ctx);

/**
 * Form Jacobian matrix B. It is a SLEPc callback
 */
PetscErrorCode mooseSlepcEigenFormJacobianB(SNES snes, Vec x, Mat jac, Mat pc, void * ctx);

/**
 * Form function residual Ax. It is a SLEPc callback
 */
PetscErrorCode mooseSlepcEigenFormFunctionA(SNES snes, Vec x, Vec r, void * ctx);

/**
 * Form function residual Bx. It is a SLEPc callback
 */
PetscErrorCode mooseSlepcEigenFormFunctionB(SNES snes, Vec x, Vec r, void * ctx);

/**
 * Form function residual Ax-Bx. It is a SLEPc callback
 */
PetscErrorCode mooseSlepcEigenFormFunctionAB(SNES snes, Vec x, Vec Ax, Vec Bx, void * ctx);

/**
 * A customized convergence checker. We need to make solver as converged when
 * doing free power iteration.
 */
PetscErrorCode mooseSlepcStoppingTest(EPS eps,
                                      PetscInt its,
                                      PetscInt max_it,
                                      PetscInt nconv,
                                      PetscInt nev,
                                      EPSConvergedReason * reason,
                                      void * ctx);

/**
 * Retrieve SNES from EPS. It makes sense only for Newton eigenvalue solvers
 */
PetscErrorCode mooseSlepcEPSGetSNES(EPS eps, SNES * snes);

/**
 * A customized solver monitor to print out eigenvalue
 */
PetscErrorCode mooseSlepcEPSMonitor(EPS eps,
                                    PetscInt its,
                                    PetscInt nconv,
                                    PetscScalar * eigr,
                                    PetscScalar * eigi,
                                    PetscReal * errest,
                                    PetscInt nest,
                                    void * mctx);

/**
 * Get rid of prefix "-eps_power" for SNES, KSP, PC, etc.
 */
PetscErrorCode mooseSlepcEPSSNESSetUpOptionPrefix(EPS eps);

/**
 * Attach a customized PC. It is useful when you want to use PBP or other customized
 * preconditioners
 */
PetscErrorCode mooseSlepcEPSSNESSetCustomizePC(EPS eps);

/**
 * Allow users to specify PC side. By default, we apply PC from the right side.
 * It is consistent with the nonlinear solver.
 */
PetscErrorCode mooseSlepcEPSSNESKSPSetPCSide(FEProblemBase & problem, EPS eps);

/**
 * Attach call backs to mat. SLEPc solver will retrieve callbacks we attached here
 */
void attachCallbacksToMat(EigenProblem & eigen_problem, Mat mat, bool eigen);

/**
 * Implement MatMult via function evaluation for Bx
 */
PetscErrorCode mooseMatMult_Eigen(Mat mat, Vec x, Vec y);

/**
 * Implement MatMult via function evaluation for Ax
 */
PetscErrorCode mooseMatMult_NonEigen(Mat mat, Vec x, Vec y);

/**
 * Set operations to shell mat
 */
void setOperationsForShellMat(EigenProblem & eigen_problem, Mat mat, bool eigen);

/**
 * Create a preconditioner from moose side. It is used to attach moose preconditioner
 */
PETSC_EXTERN PetscErrorCode PCCreate_MoosePC(PC pc);

/**
 * Let PETSc know there is a preconditioner
 */
PETSC_EXTERN PetscErrorCode registerPCToPETSc();

/**
 * Destroy preconditioner
 */
PetscErrorCode PCDestroy_MoosePC(PC pc);

/**
 * View preconditioner
 */
PetscErrorCode PCView_MoosePC(PC pc, PetscViewer viewer);

/**
 * Preconditioner application. It call libMesh preconditioner to implement this.
 */
PetscErrorCode PCApply_MoosePC(PC pc, Vec x, Vec y);

/**
 * Setup preconditioner
 */
PetscErrorCode PCSetUp_MoosePC(PC pc);

/**
 * Function call for MFFD
 */
PetscErrorCode mooseSlepcEigenFormFunctionMFFD(void * ctx, Vec x, Vec r);

} // namespace SlepcSupport
} // namespace moose

#endif // LIBMESH_HAVE_SLEPC
