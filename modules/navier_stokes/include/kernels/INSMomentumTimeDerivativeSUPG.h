/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMOMENTUMTIMEDERIVATIVESUPG_H
#define INSMOMENTUMTIMEDERIVATIVESUPG_H

#include "INSMomentumTimeDerivative.h"

// Forward Declarations
class INSMomentumTimeDerivativeSUPG;

template <>
InputParameters validParams<INSMomentumTimeDerivativeSUPG>();

/**
 * This class computes the time derivative for the incompressible
 * Navier-Stokes momentum equation with a perturbation to the test
 * function as described in Computer Methods in Applied Mechanics and Engineering
 * 32 (1982) 199-259 by Hughes et. al. This kernel must be used in conjunction
 * with INSTimeDerivative to get the contribution from the standard Galerkin
 * test function
 */
class INSMomentumTimeDerivativeSUPG : public INSMomentumTimeDerivative
{
public:
  INSMomentumTimeDerivativeSUPG(const InputParameters & parameters);

  virtual ~INSMomentumTimeDerivativeSUPG() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  // Coupled vars
  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;

  // Mat props
  const MaterialProperty<Real> & _mu;
};

#endif // INSMOMENTUMTIMEDERIVATIVESUPG_H
