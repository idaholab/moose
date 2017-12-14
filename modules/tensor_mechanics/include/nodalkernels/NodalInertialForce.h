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

#ifndef NODALINERTIALFORCE_H
#define NODALINERTIALFORCE_H

#include "NodalKernel.h"

// Forward Declarations
class NodalInertialForce;

template <>
InputParameters validParams<NodalInertialForce>();

/**
 * Calculates the inertial force and mass proportional damping for a nodal mass
 */
class NodalInertialForce : public NodalKernel
{
public:
  NodalInertialForce(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  /// Mass associated with the node
  const Real & _mass;

  /// Old value of displacement
  const VariableValue & _u_old;

  /// Newmark beta time integration parameters - beta and gamma
  const Real & _beta;
  const Real & _gamma;

  /// Mass proportional Rayliegh damping
  const Real & _eta;

  /// HHT time integration parameter
  const Real & _alpha;

  AuxiliarySystem & _aux_sys;

  unsigned int _vel_num;

  unsigned int _accel_num;
};

#endif /* NODALINERTIALFORCE_H */
