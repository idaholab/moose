/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMOMENTUMBASE_H
#define INSMOMENTUMBASE_H

#include "Kernel.h"

// Forward Declarations
class INSMomentumBase;

template <>
InputParameters validParams<INSMomentumBase>();

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
class INSMomentumBase : public Kernel
{
public:
  INSMomentumBase(const InputParameters & parameters);

  virtual ~INSMomentumBase() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Must be defined by derived classes at every qp.
  virtual Real computeQpResidualViscousPart() = 0;
  virtual Real computeQpJacobianViscousPart() = 0;
  virtual Real computeQpOffDiagJacobianViscousPart(unsigned jvar) = 0;

  // Coupled variables
  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;
  const VariableValue & _p;

  // Gradients
  const VariableGradient & _grad_u_vel;
  const VariableGradient & _grad_v_vel;
  const VariableGradient & _grad_w_vel;
  const VariableGradient & _grad_p;

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
  bool _integrate_p_by_parts;
};

#endif
