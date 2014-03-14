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

#ifndef PETSCOUTPUTTER_H
#define PETSCOUTPUTTER_H

// MOOSE includes
#include "FileOutputter.h"

// Forward declerations
class PetscOutputter;

template<>
InputParameters validParams<PetscOutputter>();

/**
 * Adds the ability to output on every nonlinear and/or linear residual
 */
class PetscOutputter : public FileOutputter
{
public:

  /**
   * Class constructor
   * @param name Outputter name
   * @param parameters Outputter input file parameters
   */
  PetscOutputter(const std::string & name, InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~PetscOutputter();

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
  Real time();
  
protected:

  /// Current norm returned from PETSc
  PetscReal _norm;

  /// Current non-linear iteration returned from PETSc
  PetscInt _nonlinear_iter;

  /// Current linear iteration returned from PETSc
  PetscInt _linear_iter;

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

  /// True when the user desires output on non-linear iterations
  bool _output_nonlinear;

  /// True when the user desires output on linear-iterations
  bool _output_linear;

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
};

#endif //PETSCOUTPUTTER_H
