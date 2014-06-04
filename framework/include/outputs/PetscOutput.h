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

#ifndef PETSCOUTPUT_H
#define PETSCOUTPUT_H

// MOOSE includes
#include "FileOutput.h"

// Forward declerations
class PetscOutput;

template<>
InputParameters validParams<PetscOutput>();

/**
 * Adds the ability to output on every nonlinear and/or linear residual
 */
class PetscOutput : public FileOutput
{
public:

  /**
   * Class constructor
   * @param name Outputter name
   * @param parameters Outputter input file parameters
   */
  PetscOutput(const std::string & name, InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~PetscOutput();

  /**
   * Linear residual output status
   * @return True if the output is currently being called on a linear residual evaluation
   */
  bool onLinearResidual(){ return _on_linear_residual; }

  /**
   * Non-Linear residual output status
   * @return True if the output is currently being called on a non-linear residual evaluation
   */
  bool onNonlinearResidual() { return _on_nonlinear_residual; }

  /**
   * Get the output time.
   * This outputter enables the ability to perform output on the nonlinear and linear iterations performed
   * by PETSc. To separate theses outputs within the output a pseudo time is defined, this function provides
   * this time and it should be used in place of _time from Outputter.
   */
  virtual Real time();

protected:

  /// Current norm returned from PETSc
  Real _norm;

  /// Current non-linear iteration returned from PETSc
  int _nonlinear_iter;

  /// Current linear iteration returned from PETSc
  int _linear_iter;

  /// True when the user desires output on non-linear iterations
  bool _output_nonlinear;

  /// True when the user desires output on linear-iterations
  bool _output_linear;

private:

  /**
   * Internal setup function that executes at the beginning of the time step
   */
  void timestepSetupInternal();

#ifdef LIBMESH_HAVE_PETSC
  /**
   * Performs the output on non-linear iterations
   *
   * This is the monitor method that PETSc will call on non-linear iterations
   */
  static PetscErrorCode petscNonlinearOutput(SNES, PetscInt its, PetscReal fnorm, void * void_ptr);

  /**
   * Performs the output onlinear iterations
   *
   * This is the monitor method that PETSc will call on linear iterations
   */
  static PetscErrorCode petscLinearOutput(KSP, PetscInt its, PetscReal fnorm, void * void_ptr);
#endif

  /// The psuedo non-linear time
  Real _nonlinear_time;

  /// The pseuedo non-linear time step
  Real _nonlinear_dt;

  /// Psuedo linear time
  Real _linear_time;

  /// Psuedo linear time step
  Real _linear_dt;

  /// True if current output calls is on the linear residual
  bool _on_linear_residual;

  /// True if current output call is on the non-linear residual
  bool _on_nonlinear_residual;

  /// Pseudo non-linear timestep divisor
  Real _nonlinear_dt_divisor;

  /// Pseudo linear timestep divisor
  Real _linear_dt_divisor;

  /// Non-linear residual output start time
  Real _nonlinear_start_time;

  /// Linear residual output start time
  Real _linear_start_time;

  /// Non-linear residual output end time
  Real _nonlinear_end_time;

  /// Linear residual output end time
  Real _linear_end_time;
};
#endif //PETSCOUTPUT_H
