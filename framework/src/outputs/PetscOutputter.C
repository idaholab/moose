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

// MOOSE includes
#include "PetscOutputter.h"
#include "FEProblem.h"

// libMesh includes
#include "libmesh/libmesh_common.h"
#include "libmesh/petsc_nonlinear_solver.h"

template<>
InputParameters validParams<PetscOutputter>()
{
  InputParameters params = validParams<FileOutputter>();

  // Toggled for outputting nonlinear and linear residuals, only if we have PETSc
#ifdef LIBMESH_HAVE_PETSC
  params.addParam<bool>("linear_residuals", false, "Specifies whether output occurs on each linear residual evaluation");
  params.addParam<bool>("nonlinear_residuals", false, "Specifies whether output occurs on each nonlinear residual evaluation");

  // Psuedo time step divisors
  params.addParam<Real>("nonlinear_residual_dt_divisor", 1000, "Number of divisions applied to time step when outtputting non-linear residuals");
  params.addParam<Real>("linear_residual_dt_divisor", 1000, "Number of divisions applied to time step when outtputting linear residuals");
  params.addParamNamesToGroup("nonlinear_residual_dt_divisor linear_residual_dt_divisor", "Advanced");
#endif

  return params;
}

PetscOutputter::PetscOutputter(const std::string & name, InputParameters & parameters) :
    FileOutputter(name, parameters),
    _nonlinear_iter(0),
    _linear_iter(0),
    _output_nonlinear(getParam<bool>("nonlinear_residuals")),
    _output_linear(getParam<bool>("linear_residuals")),
    _on_linear_residual(false),
    _on_nonlinear_residual(false),
    _nonlinear_dt_divisor(getParam<Real>("nonlinear_residual_dt_divisor")),
    _linear_dt_divisor(getParam<Real>("linear_residual_dt_divisor"))
{
}

PetscOutputter::~PetscOutputter()
{
}

void
PetscOutputter::timestepSetupInternal()
{
// Only execute if PETSc exists
#ifdef LIBMESH_HAVE_PETSC

  // Extract the non-linear and linear solvers from PETSc
  NonlinearSystem & nl = _problem_ptr->getNonlinearSystem();
  PetscNonlinearSolver<Number> * petsc_solver = dynamic_cast<PetscNonlinearSolver<Number> *>(nl.sys().nonlinear_solver.get());
  SNES snes = petsc_solver->snes();
  KSP ksp;
  SNESGetKSP(snes, &ksp);

  // Update the pseudo times
  _nonlinear_time = _time_old;                   // non-linear time starts with the previous time step
  _nonlinear_dt = _dt/_nonlinear_dt_divisor;     // set the pseudo non-linear timestep
  _linear_dt = _nonlinear_dt/_linear_dt_divisor; // set the pseudo linear timestep

  // Set the PETSc monitor functions
  if (_output_nonlinear)
  {
    PetscErrorCode ierr = SNESMonitorSet(snes, petscNonlinearOutput, this, PETSC_NULL);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }

  if (_output_linear)
  {
    PetscErrorCode ierr = KSPMonitorSet(ksp, petscLinearOutput, this, PETSC_NULL);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }
#endif
}

// Only define the montior functions if PETSc exists
#ifdef LIBMESH_HAVE_PETSC
PetscErrorCode
PetscOutputter::petscNonlinearOutput(SNES, PetscInt its, PetscReal norm, void * void_ptr)
{
  // Get the outputter object
  PetscOutputter * ptr = static_cast<PetscOutputter *>(void_ptr);

  // Update the psuedo times
  ptr->_nonlinear_time += ptr->_nonlinear_dt;
  ptr->_linear_time = ptr->_nonlinear_time;

  // Set the current norm and iteration number
  ptr->_norm = norm;
  ptr->_nonlinear_iter = its;

  // Set the flag indicating that output is occuriong on the non-linear residual
  ptr->_on_nonlinear_residual = true;

  // Perform the output
  ptr->outputStep();

  // Reset the non-linear output flag and the simulation time
  ptr->_on_nonlinear_residual = false;

  // Done
  return 0;
}

PetscErrorCode
PetscOutputter::petscLinearOutput(KSP, PetscInt its, PetscReal norm, void * void_ptr)
{
  // Get the Outputter object
  PetscOutputter * ptr = static_cast<PetscOutputter *>(void_ptr);

  // Update the psuedo time
  ptr->_linear_time += ptr->_linear_dt;

  // Set the current norm and iteration number
  ptr->_norm = norm;
  ptr->_linear_iter = its;

  // Set the flag indicating that output is occuriong on the non-linear residual
  ptr->_on_linear_residual = true;

  // Perform the output
  ptr->outputStep();

  // Reset the linear output flag and the simulation time
  ptr->_on_linear_residual = false;

  // Done
  return 0;
}
#endif

Real
PetscOutputter::time()
{
  if (_on_nonlinear_residual)
    return _nonlinear_time;
  else if (_on_linear_residual)
    return _linear_time;
  else
    return _time;
}
