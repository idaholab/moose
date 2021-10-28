//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh.h"

// MOOSE includes
#include "MultiMooseEnum.h"

#include "libmesh/petsc_macro.h"
#include "libmesh/linear_solver.h"
#include "libmesh/petsc_linear_solver.h"

#include <petscksp.h>

// Forward declarations
class FEProblemBase;
class NonlinearSystemBase;
class CommandLine;
class InputParameters;

namespace Moose
{
namespace PetscSupport
{

/**
 * A struct for storing the various types of petsc options and values
 */
class PetscOptions
{
public:
  PetscOptions() : flags("", "", true) {}

  /// PETSc key-value pairs
  std::vector<std::pair<std::string, std::string>> pairs;

  /// Single value PETSc options (flags)
  MultiMooseEnum flags;

  /// Preconditioner description
  std::string pc_description;
};

/**
 * A function for setting the PETSc options in PETSc from the options supplied to MOOSE
 */
void petscSetOptions(FEProblemBase & problem);

/**
 * Set the default options for a KSP
 */
void petscSetKSPDefaults(FEProblemBase & problem, KSP ksp);

/**
 * Set the defaults for a libMesh LinearSolver
 *
 * Used in explicit solves
 */
template <typename T>
void
setLinearSolverDefaults(FEProblemBase & problem, LinearSolver<T> & linear_solver)
{
  petscSetKSPDefaults(problem, libMesh::cast_ref<PetscLinearSolver<T> &>(linear_solver).ksp());
}

/**
 * Sets the default options for PETSc
 */
void petscSetDefaults(FEProblemBase & problem);

void petscSetupDM(NonlinearSystemBase & nl);

PetscErrorCode petscSetupOutput(CommandLine * cmd_line);

/**
 * Helper function for outputting the norm values with/without color
 */
void outputNorm(libMesh::Real old_norm, libMesh::Real norm, bool use_color = false);

/**
 * Helper function for displaying the linear residual during PETSC solve
 */
PetscErrorCode petscLinearMonitor(KSP /*ksp*/, PetscInt its, PetscReal rnorm, void * void_ptr);

/**
 * Stores the PETSc options supplied from the InputParameters with MOOSE
 */
void storePetscOptions(FEProblemBase & fe_problem, const InputParameters & params);

/**
 * Returns the valid petsc line search options as a set of strings
 */
std::set<std::string> getPetscValidLineSearches();

/**
 * Returns the PETSc options that are common between Executioners and Preconditioners
 * @return InputParameters object containing the PETSc related parameters
 *
 * The output of this function should be added to the the parameters object of the overarching class
 * @see CreateExecutionerAction
 */
InputParameters getPetscValidParams();

/// A helper function to produce a MultiMooseEnum with commonly used PETSc single options (flags)
MultiMooseEnum getCommonPetscFlags();

/// A helper function to produce a MultiMooseEnum with commonly used PETSc iname options (keys in key-value pairs)
MultiMooseEnum getCommonPetscKeys();

/// check if SNES type is variational inequalities (VI) solver
bool isSNESVI(FEProblemBase & fe_problem);

/**
 * A wrapper function for dealing with different versions of
 * PetscOptionsSetValue.  This is not generally called from
 * MOOSE code, it is instead intended to be called by stuff in
 * MOOSE::PetscSupport.
 */
void setSinglePetscOption(const std::string & name, const std::string & value = "");

void addPetscOptionsFromCommandline();

/**
 * Setup which side we want to apply preconditioner
 */
void petscSetDefaultPCSide(FEProblemBase & problem, KSP ksp);

/**
 * Set norm type
 */
void petscSetDefaultKSPNormType(FEProblemBase & problem, KSP ksp);

/**
 * This method takes an adjacency matrix, and a desired number of colors and applies
 * a graph coloring algorithm to produce a coloring. The coloring is returned as a vector
 * of unsigned integers indicating which color or group each vextex in the adjacency matrix
 * belongs to.
 */
void colorAdjacencyMatrix(PetscScalar * adjacency_matrix,
                          unsigned int size,
                          unsigned int colors,
                          std::vector<unsigned int> & vertex_colors,
                          const char * coloring_algorithm);

/**
 * disable printing of the nonlinear convergence reason
 */
void disableNonlinearConvergedReason(FEProblemBase & fe_problem);

/**
 * disable printing of the linear convergence reason
 */
void disableLinearConvergedReason(FEProblemBase & fe_problem);

#define SNESGETLINESEARCH SNESGetLineSearch
}
}
