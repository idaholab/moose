/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef PETSCSUPPORT_H
#define PETSCSUPPORT_H

#include "libmesh/libmesh.h"

#ifdef LIBMESH_HAVE_PETSC

// Moose includes
#include "Problem.h"
#include "NonlinearSystem.h"
#include "CommandLine.h"
#include "Console.h"

// libMesh
#include "libmesh/petsc_nonlinear_solver.h"

class FEProblem;

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
  PetscOptions() :
      flags("", "", true)
  {}

  /// Keys for PETSc key-value pairs
  std::vector<std::string> inames;

  /// Values for PETSc key-value pairs
  std::vector<std::string> values;

  /// Single value PETSc options (flags)
  MultiMooseEnum flags;

  /// Preconditioner description
  std::string pc_description;
};

/**
 * A function for setting the PETSc options in PETSc from the options supplied to MOOSE
 */
void petscSetOptions(FEProblem & problem);

/**
 * Sets the default options for PETSc
 */
void petscSetDefaults(FEProblem & problem);

void petscSetupDampers(NonlinearImplicitSystem& sys);

void petscSetupDM(NonlinearSystem & nl);

PetscErrorCode petscSetupOutput(CommandLine * cmd_line);

/**
 * Helper function for outputing the norm values with/without color
 */
void outputNorm(Real old_norm, Real norm, bool use_color = false);

/**
 * Helper function for displaying the linear residual during PETSC solve
 */
PetscErrorCode petscLinearMonitor(KSP /*ksp*/, PetscInt its, PetscReal rnorm, void *void_ptr);

/**
 * Stores the PETSc options supplied from the InputParameters with MOOSE
 */
void storePetscOptions(FEProblem & fe_problem, const InputParameters & params);

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
MultiMooseEnum getCommonPetscOptionsKeys();
}
}

#endif //LIBMESH_HAVE_PETSC

#endif //PETSCSUPPORT_H
