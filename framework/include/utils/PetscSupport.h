//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh.h"

// MOOSE includes
#include "SolverParams.h"
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
class ParallelParamObject;

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
  PetscOptions()
    : flags("", "", true), dont_add_these_options("", "", true), user_set_options("", "", true)
  {
  }

  /// PETSc key-value pairs
  std::vector<std::pair<std::string, std::string>> pairs;

  /// Single value PETSc options (flags)
  MultiMooseEnum flags;

  /// Flags to explicitly not set, even if they are specified programmatically
  MultiMooseEnum dont_add_these_options;

  /// Options that are set by the user at the input level
  MultiMooseEnum user_set_options;

  /// Preconditioner description
  std::string pc_description;
};

/**
 * A function for setting the PETSc options in PETSc from the options supplied to MOOSE. This
 * interface function should be used when setting options on a per-system basis
 */
void petscSetOptions(const PetscOptions & po,
                     const SolverParams & solver_params,
                     FEProblemBase * const problem = nullptr);

/**
 * A function for setting the PETSc options in PETSc from the options supplied to MOOSE. This
 * interface function should be used for setting options all at once for all systems in a
 * multi-system context. Note that PetscOptions is not a vector because the options database has
 * prefixes for the different systems
 */
void petscSetOptions(const PetscOptions & po,
                     const std::vector<SolverParams> & solver_params,
                     FEProblemBase * problem);

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
setLinearSolverDefaults(FEProblemBase & problem, libMesh::LinearSolver<T> & linear_solver)
{
  petscSetKSPDefaults(problem,
                      libMesh::cast_ref<libMesh::PetscLinearSolver<T> &>(linear_solver).ksp());
}

/**
 * Sets the default options for PETSc
 */
void petscSetDefaults(FEProblemBase & problem);

/**
 * Setup the PETSc DM object
 */
void petscSetupDM(NonlinearSystemBase & nl, const std::string & dm_name);

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
 * Process some MOOSE-wrapped PETSc options. These options have no support for multi-system as
 * indicated by the fact that this function takes no prefix nor solver system argument
 */
void processSingletonMooseWrappedOptions(FEProblemBase & fe_problem,
                                         const InputParameters & params);

/**
 * Stores the PETSc options supplied from the parameter object on the problem
 * @param fe_problem The problem on which we will store the parameters
 * @param prefix A prefix to apply to all the parameter object's PETSc options. This should either
 * be a single character '-' or a string like "-foo_" where the trailing '_' is required
 * @param param_object The parameter object potentially holding PETSc options
 * String prefixes may be used to select the system the parameters is applied to
 */
void storePetscOptions(FEProblemBase & fe_problem,
                       const std::string & prefix,
                       const ParallelParamObject & param_object);

/**
 * Set flags that will instruct the user on the reason their simulation diverged from PETSc's
 * perspective
 */
void setConvergedReasonFlags(FEProblemBase & fe_problem, const std::string & prefix);

/**
 * Sets the FE problem's solve type from the input params.
 */
void setSolveTypeFromParams(FEProblemBase & fe_problem, const InputParameters & params);

/**
 * Sets the FE problem's line search from the input params.
 */
void setLineSearchFromParams(FEProblemBase & fe_problem, const InputParameters & params);

/**
 *  Sets the FE problem's matrix-free finite difference type from the input params.
 */
void setMFFDTypeFromParams(FEProblemBase & fe_problem, const InputParameters & params);

/**
 * Stores the Petsc flags and pair options fron the input params in the given PetscOptions object.
 */
void storePetscOptionsFromParams(FEProblemBase & fe_problem, const InputParameters & params);

/**
 * Populate flags in a given PetscOptions object using a vector of input arguments
 * @param petsc_flags Container holding the flags of the petsc options
 * @param prefix The prefix to add to the user provided \p petsc_flags
 * @param param_object The \p ParallelParamObject adding the PETSc options
 * @param petsc_options Data structure which handles petsc options within moose
 */
void addPetscFlagsToPetscOptions(const MultiMooseEnum & petsc_flags,
                                 const std::string & prefix,
                                 const ParallelParamObject & param_object,
                                 PetscOptions & petsc_options);

/**
 * Populate name and value pairs in a given PetscOptions object using vectors of input arguments
 * @param petsc_pair_options Option-value pairs of petsc settings
 * @param mesh_dimension The mesh dimension, needed for multigrid settings
 * @param prefix The prefix to add to the user provided \p petsc_flags
 * @param param_object The \p ParallelParamObject adding the PETSc options
 * @param petsc_options Data structure which handles petsc options within moose
 */
void addPetscPairsToPetscOptions(
    const std::vector<std::pair<MooseEnumItem, std::string>> & petsc_pair_options,
    const unsigned int mesh_dimension,
    const std::string & prefix,
    const ParallelParamObject & param_object,
    PetscOptions & petsc_options);

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

/// A helper function to produce a MultiMooseEnum with commonly used PETSc snes single options (flags)
MultiMooseEnum getCommonSNESFlags();

/// A helper function to produce a MultiMooseEnum with commonly used PETSc ksp single options (flags)
MultiMooseEnum getCommonKSPFlags();

/// A helper function to produce a MultiMooseEnum with commonly used PETSc iname options (keys in key-value pairs)
MultiMooseEnum getCommonPetscKeys();

/// A helper function to produce a MultiMooseEnum with commonly used PETSc snes option names (keys)
MultiMooseEnum getCommonSNESKeys();

/// A helper function to produce a MultiMooseEnum with commonly used PETSc ksp option names (keys)
MultiMooseEnum getCommonKSPKeys();

/// check if SNES type is variational inequalities (VI) solver
bool isSNESVI(FEProblemBase & fe_problem);

/**
 * A wrapper function for dealing with different versions of
 * PetscOptionsSetValue.  This is not generally called from
 * MOOSE code, it is instead intended to be called by stuff in
 * MOOSE::PetscSupport.
 */
void setSinglePetscOption(const std::string & name,
                          const std::string & value = "",
                          FEProblemBase * const problem = nullptr);

/**
 * Same as setSinglePetscOption, but does not set the option if it doesn't make sense for the
 * current simulation type, e.g. if \p name is contained within \p dont_add_these_options
 */
void setSinglePetscOptionIfAppropriate(const MultiMooseEnum & dont_add_these_options,
                                       const std::string & name,
                                       const std::string & value = "",
                                       FEProblemBase * const problem = nullptr);

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
 * Function to ensure that a particular petsc option is not added to the PetscOptions
 * storage object to be later set unless explicitly specified in input or on the command line.
 */
void dontAddPetscFlag(const std::string & flag, PetscOptions & petsc_options);

/**
 * Function to ensure that -snes_converged_reason is not added to the PetscOptions storage
 * object to be later set unless explicitly specified in input or on the command line.
 */
void dontAddNonlinearConvergedReason(FEProblemBase & fe_problem);

/**
 * Function to ensure that -ksp_converged_reason is not added to the PetscOptions storage
 * object to be later set unless explicitly specified in input or on the command line.
 */
void dontAddLinearConvergedReason(FEProblemBase & fe_problem);

/**
 * Function to ensure that common KSP options are not added to the PetscOptions storage
 * object to be later set unless explicitly specified in input or on the command line.
 */
void dontAddCommonKSPOptions(FEProblemBase & fe_problem);

/**
 * Function to ensure that common SNES options are not added to the PetscOptions storage
 * object to be later set unless explicitly specified in input or on the command line.
 */
void dontAddCommonSNESOptions(FEProblemBase & fe_problem);

/**
 * Create a matrix from a binary file. Note that the returned libMesh matrix wrapper will not
 * destroy the created matrix on destruction. \p petsc_mat must be destroyed manually via \p
 * MatDestroy
 */
std::unique_ptr<PetscMatrix<Number>>
createMatrixFromFile(const libMesh::Parallel::Communicator & comm,
                     Mat & petsc_mat,
                     const std::string & binary_mat_file);

#define SNESGETLINESEARCH SNESGetLineSearch
}
}
