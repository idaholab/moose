/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMOMENTUMSUPG_H
#define INSMOMENTUMSUPG_H

#include "Kernel.h"

// Forward Declarations
class INSMomentumSUPG;

template <>
InputParameters validParams<INSMomentumSUPG>();

/**
 * This class computes the spatial part of the momentum equation
 * residual and Jacobian for the incompressible Navier-Stokes momentum
 * equation, calling a virtual function to get the viscous
 * contribution.  You do not use this class directly, instead use one of:
 * .) INSMomentumLaplaceForm
 * .) INSMomentumTractionForm
 * depending on the application.  For "open" flow boundary conditions,
 * the INSMomentumLaplaceForm seems to give better results.  If you
 * have traction BCs, i.e. BCs where the normal traction is specified
 * on part of the boundary, you should use the INSMomentumTractionForm
 * instead.
 */
class INSMomentumSUPG : public Kernel
{
public:
  INSMomentumSUPG(const InputParameters & parameters);

  virtual ~INSMomentumSUPG() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Coupled variables
  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;

  // Gradients
  const VariableGradient & _grad_u_vel;
  const VariableGradient & _grad_v_vel;
  const VariableGradient & _grad_w_vel;

  // Variable numberings
  unsigned _u_vel_var_number;
  unsigned _v_vel_var_number;
  unsigned _w_vel_var_number;

  // Parameters
  unsigned _component;

  // Material properties
  const MaterialProperty<Real> & _mu;
  const MaterialProperty<Real> & _rho;
};

#endif
