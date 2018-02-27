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

#ifndef NODALTRANSLATIONALINERTIA_H
#define NODALTRANSLATIONALINERTIA_H

#include "NodalKernel.h"

// Forward Declarations
class NodalTranslationalInertia;

template <>
InputParameters validParams<NodalTranslationalInertia>();

/**
 * Calculates the inertial force and mass proportional damping for a nodal mass
 */
class NodalTranslationalInertia : public NodalKernel
{
public:
  NodalTranslationalInertia(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  /// Mass associated with the node
  const Real & _mass;

  /// Old value of displacement
  const VariableValue & _u_old;

  /// Newmark time integration parameter
  const Real & _beta;

  /// Newmark time integration parameter
  const Real & _gamma;

  /// Mass proportional Rayliegh damping
  const Real & _eta;

  /// HHT time integration parameter
  const Real & _alpha;

  /// Auxiliary system object
  AuxiliarySystem & _aux_sys;

  /// Variable number corresponding to the velocity aux variable
  unsigned int _vel_num;

  /// Variable number corresponding to the acceleration aux variable
  unsigned int _accel_num;
};

#endif /* NODALTRANSLATIONALINERTIA_H */
