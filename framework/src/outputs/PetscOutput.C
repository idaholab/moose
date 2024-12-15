//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "PetscOutput.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"

#include "libmesh/libmesh_common.h"
#include "libmesh/petsc_nonlinear_solver.h"

PetscOutputInterface::PetscOutputInterface(PetscOutput * obj) : _petsc_output(obj)
{
  // register petsc output interface
  obj->getMooseApp().registerInterfaceObject(*this);
}

PetscErrorCode
PetscOutputInterface::petscNonlinearOutput(SNES, PetscInt its, PetscReal norm, void * void_ptr)
{
  // Get the primary outputter object
  PetscOutput * primary_ptr = static_cast<PetscOutput *>(void_ptr);

  // loop over all interface objects
  for (const auto poi_ptr : primary_ptr->getMooseApp().getInterfaceObjects<PetscOutputInterface>())
  {
    auto ptr = poi_ptr->_petsc_output;

    // check time
    if (!ptr->inNonlinearTimeWindow())
      continue;

    // Update the pseudo times
    ptr->_nonlinear_time += ptr->_nonlinear_dt;
    ptr->_linear_time = ptr->_nonlinear_time;

    // Set the current norm and iteration number
    ptr->_norm = norm;
    ptr->_nonlinear_iter = its;

    // Set the flag indicating that output is occurring on the non-linear residual
    ptr->_on_nonlinear_residual = true;

    // Perform the output
    ptr->outputStep(EXEC_NONLINEAR);

    /**
     * This is one of three locations where we explicitly flush the output buffers during a
     * simulation:
     * PetscOutput::petscNonlinearOutput()
     * PetscOutput::petscLinearOutput()
     * OutputWarehouse::outputStep()
     *
     * All other Console output _should_ be using newlines to avoid covering buffer errors
     * and to avoid excessive I/O. This call is necessary. In the PETSc callback the
     * context bypasses the OutputWarehouse.
     */
    ptr->_app.getOutputWarehouse().flushConsoleBuffer();

    // Reset the non-linear output flag and the simulation time
    ptr->_on_nonlinear_residual = false;
  }

  // Done
  return (PetscErrorCode)0;
}

PetscErrorCode
PetscOutputInterface::petscLinearOutput(KSP, PetscInt its, PetscReal norm, void * void_ptr)
{
  // Get the primary outputter object
  PetscOutput * primary_ptr = static_cast<PetscOutput *>(void_ptr);

  // loop over all interface objects
  for (const auto poi_ptr : primary_ptr->getMooseApp().getInterfaceObjects<PetscOutputInterface>())
  {
    auto ptr = poi_ptr->_petsc_output;

    // check time
    if (!ptr->inLinearTimeWindow())
      continue;

    // Update the pseudo time
    ptr->_linear_time += ptr->_linear_dt;

    // Set the current norm and iteration number
    ptr->_norm = norm;
    ptr->_linear_iter = its;

    // Set the flag indicating that output is occurring on the non-linear residual
    ptr->_on_linear_residual = true;

    // Perform the output
    ptr->outputStep(EXEC_LINEAR);

    /**
     * This is one of three locations where we explicitly flush the output buffers during a
     * simulation:
     * PetscOutput::petscNonlinearOutput()
     * PetscOutput::petscLinearOutput()
     * OutputWarehouse::outputStep()
     *
     * All other Console output _should_ be using newlines to avoid covering buffer errors
     * and to avoid excessive I/O. This call is necessary. In the PETSc callback the
     * context bypasses the OutputWarehouse.
     */
    ptr->_app.getOutputWarehouse().flushConsoleBuffer();

    // Reset the linear output flag and the simulation time
    ptr->_on_linear_residual = false;
  }

  // Done
  return (PetscErrorCode)0;
}

InputParameters
PetscOutput::validParams()
{
  InputParameters params = Output::validParams();

  // Toggled for outputting nonlinear and linear residuals, only if we have PETSc
  params.addParam<bool>("output_linear",
                        false,
                        "Specifies whether output occurs on each PETSc linear residual evaluation");
  params.addParam<bool>(
      "output_nonlinear",
      false,
      "Specifies whether output occurs on each PETSc nonlinear residual evaluation");
  params.addParamNamesToGroup("output_linear output_nonlinear", "Execution scheduling");

  // Pseudo time step divisors
  params.addParam<Real>(
      "nonlinear_residual_dt_divisor",
      1000,
      "Number of divisions applied to time step when outputting non-linear residuals");
  params.addParam<Real>(
      "linear_residual_dt_divisor",
      1000,
      "Number of divisions applied to time step when outputting linear residuals");

  // Start times for residual output
  params.addParam<Real>(
      "linear_residual_start_time",
      "Specifies a start time to begin output on each linear residual evaluation");
  params.addParam<Real>(
      "nonlinear_residual_start_time",
      "Specifies a start time to begin output on each nonlinear residual evaluation");

  // End time for residual output
  /* Note, No default is given here so that in Peacock giant numbers do not show up by default, the
   * defaults are set in the initialization list */
  params.addParam<Real>("linear_residual_end_time",
                        "Specifies an end time to begin output on each linear residual evaluation");
  params.addParam<Real>(
      "nonlinear_residual_end_time",
      "Specifies an end time to begin output on each nonlinear residual evaluation");

  params.addParamNamesToGroup("linear_residual_start_time "
                              "nonlinear_residual_start_time linear_residual_end_time "
                              "nonlinear_residual_end_time  nonlinear_residual_dt_divisor "
                              "linear_residual_dt_divisor",
                              "PETSc linear/nonlinear output");
  return params;
}

PetscOutput::PetscOutput(const InputParameters & parameters)
  : Output(parameters),
    PetscOutputInterface(this),
    _nonlinear_iter(0),
    _linear_iter(0),
    _on_linear_residual(false),
    _on_nonlinear_residual(false),
    _nonlinear_dt_divisor(getParam<Real>("nonlinear_residual_dt_divisor")),
    _linear_dt_divisor(getParam<Real>("linear_residual_dt_divisor")),
    _nonlinear_start_time(-std::numeric_limits<Real>::max()),
    _linear_start_time(-std::numeric_limits<Real>::max()),
    _nonlinear_end_time(std::numeric_limits<Real>::max()),
    _linear_end_time(std::numeric_limits<Real>::max())
{
  // Output toggle support
  if (getParam<bool>("output_linear"))
    _execute_on.setAdditionalValue("linear");
  if (getParam<bool>("output_nonlinear"))
    _execute_on.setAdditionalValue("nonlinear");

  // Nonlinear residual start-time supplied by user
  if (isParamValid("nonlinear_residual_start_time"))
  {
    _nonlinear_start_time = getParam<Real>("nonlinear_residual_start_time");
    _execute_on.setAdditionalValue("nonlinear");
  }

  // Nonlinear residual end-time supplied by user
  if (isParamValid("nonlinear_residual_end_time"))
    _nonlinear_end_time = getParam<Real>("nonlinear_residual_end_time");

  // Linear residual start-time supplied by user
  if (isParamValid("linear_residual_start_time"))
  {
    _linear_start_time = getParam<Real>("linear_residual_start_time");
    _execute_on.setAdditionalValue("linear");
  }

  // Linear residual end-time supplied by user
  if (isParamValid("linear_residual_end_time"))
    _linear_end_time = getParam<Real>("linear_residual_end_time");
}

void
PetscOutput::solveSetup()
{
  // Update the pseudo times
  _nonlinear_time = _time_old; // non-linear time starts with the previous time step
  if (_dt != 0)
    // set the pseudo non-linear timestep as fraction
    // of real timestep for transient executioners
    _nonlinear_dt = _dt / _nonlinear_dt_divisor;
  else
    // set the pseudo non-linear timestep for steady
    // executioners (here _dt==0)
    _nonlinear_dt = 1. / _nonlinear_dt_divisor;

  _linear_dt = _nonlinear_dt / _linear_dt_divisor; // set the pseudo linear timestep

  // only the first PetscOutput object registers a DM monitor for linear and nonlinear
  // all other PetscOutput objects are dispatched from that one monitor (per app).
  if (_app.getInterfaceObjects<PetscOutputInterface>()[0] != this)
    return;

  // Extract the non-linear and linear solvers from PETSc
  NonlinearSystemBase & nl = _problem_ptr->currentNonlinearSystem();
  SNES snes = nl.getSNES();
  KSP ksp;
  LibmeshPetscCallA(_communicator.get(), SNESGetKSP(snes, &ksp));

  // Set the PETSc monitor functions (register the nonlinear callback so that linear outputs
  // get an updated nonlinear iteration number)
  // Not every Output should register its own DM monitor! Just register one each of nonlinear
  // and linear and dispatch all Outputs from there!
  LibmeshPetscCallA(_communicator.get(),
                    SNESMonitorSet(snes, petscNonlinearOutput, this, LIBMESH_PETSC_NULLPTR));
  LibmeshPetscCallA(_communicator.get(),
                    KSPMonitorSet(ksp, petscLinearOutput, this, LIBMESH_PETSC_NULLPTR));
}

Real
PetscOutput::time()
{
  if (_on_nonlinear_residual)
    return _nonlinear_time;
  else if (_on_linear_residual)
    return _linear_time;
  else
    return Output::time();
}

Real
PetscOutput::getOutputTime()
{
  return time();
}
