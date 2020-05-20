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
InputParameters getSlepcEigenProblemValidParams();
void storeSlepcOptions(FEProblemBase & fe_problem, const InputParameters & params);
void storeSlepcEigenProblemOptions(EigenProblem & eigen_problem, const InputParameters & params);
void slepcSetOptions(EigenProblem & eigen_problem, const InputParameters & params);
void setSlepcEigenSolverTolerances(EigenProblem & eigen_problem, const InputParameters & params);
void setSlepcOutputOptions(EigenProblem & eigen_problem);

void moosePetscSNESFormMatrixTag(SNES snes, Vec x, Mat mat, void * ctx, TagID tag);
void moosePetscSNESFormMatricesTags(
    SNES snes, Vec x, std::vector<Mat> & mats, void * ctx, const std::set<TagID> & tags);
PetscErrorCode mooseSlepcEigenFormJacobianA(SNES snes, Vec x, Mat jac, Mat pc, void * ctx);
PetscErrorCode mooseSlepcEigenFormJacobianB(SNES snes, Vec x, Mat jac, Mat pc, void * ctx);
PetscErrorCode mooseSlepcEigenFormFunctionA(SNES snes, Vec x, Vec r, void * ctx);
PetscErrorCode mooseSlepcEigenFormFunctionB(SNES snes, Vec x, Vec r, void * ctx);
PetscErrorCode mooseSlepcEigenFormFunctionAB(SNES snes, Vec x, Vec Ax, Vec Bx, void * ctx);

void attachCallbacksToMat(EigenProblem & eigen_problem, Mat mat, bool eigen);

PetscErrorCode mooseMatMult_Eigen(Mat mat, Vec x, Vec y);
PetscErrorCode mooseMatMult_NonEigen(Mat mat, Vec x, Vec y);
void setOperationsForShellMat(EigenProblem & eigen_problem, Mat mat, bool eigen);

PETSC_EXTERN PetscErrorCode PCCreate_MoosePC(PC pc);
PETSC_EXTERN PetscErrorCode registerPCToPETSc();
PetscErrorCode PCDestroy_MoosePC(PC pc);
PetscErrorCode PCView_MoosePC(PC pc, PetscViewer viewer);
PetscErrorCode PCApply_MoosePC(PC pc, Vec x, Vec y);
PetscErrorCode PCSetUp_MoosePC(PC pc);

} // namespace SlepcSupport
} // namespace moose

#endif // LIBMESH_HAVE_SLEPC
