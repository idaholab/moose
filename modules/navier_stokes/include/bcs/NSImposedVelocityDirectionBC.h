/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSIMPOSEDVELOCITYDIRECTIONBC_H
#define NSIMPOSEDVELOCITYDIRECTIONBC_H

#include "NodalBC.h"

// Forward Declarations
class NSImposedVelocityDirectionBC;

// Specialization required of all user-level Moose objects
template <>
InputParameters validParams<NSImposedVelocityDirectionBC>();

/**
 * This class imposes a velocity direction component as a
 * Dirichlet condition on the appropriate momentum equation.
 * For example, in the x-direction, the residual equation becomes:
 *
 * u1/|u| - u1_hat_desired = 0
 *
 * or
 *
 * u1 - u1_hat_desired*|u| = 0
 *
 * or
 *
 * rho*u1 - rho*u1_hat_desired*|u| = 0
 *
 * where:
 * u1  = the x-momentum component
 * |u| = velocity magnitude
 * u1_hat_desired = The desired velocity direction, \f$ \in (0,1) \f$
 */
class NSImposedVelocityDirectionBC : public NodalBC
{
public:
  NSImposedVelocityDirectionBC(const InputParameters & parameters);

protected:
  // NodalBC's can (currently) only specialize the computeQpResidual function,
  // the computeQpJacobian() function automatically assembles a "1" onto the main
  // diagonal for this DoF.
  virtual Real computeQpResidual();

  // Coupled variables
  const VariableValue & _rho;
  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;

  // The desired value for the unit velocity component
  Real _desired_unit_velocity_component;
};

#endif // NSIMPOSEDVELOCITYDIRECTIONBC_H
