/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMOMENTUM_H
#define INSMOMENTUM_H

#include "Kernel.h"

// Forward Declarations
class INSMomentum;

template<>
InputParameters validParams<INSMomentum>();

/**
 * This class computes momentum equation residual and Jacobian
 * contributions for the incompressible Navier-Stokes momentum
 * equation.
 */
class INSMomentum : public Kernel
{
public:
  INSMomentum(const InputParameters & parameters);

  virtual ~INSMomentum(){}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Coupled variables
  VariableValue& _u_vel;
  VariableValue& _v_vel;
  VariableValue& _w_vel;
  VariableValue& _p;

  // Gradients
  VariableGradient& _grad_u_vel;
  VariableGradient& _grad_v_vel;
  VariableGradient& _grad_w_vel;

  // Variable numberings
  unsigned _u_vel_var_number;
  unsigned _v_vel_var_number;
  unsigned _w_vel_var_number;
  unsigned _p_var_number;

  // Material properties
  // MaterialProperty<Real> & _dynamic_viscosity;
  Real _mu;
  Real _rho;
  RealVectorValue _gravity;

  // Parameters
  unsigned _component;
};


#endif // INSMOMENTUM_H
