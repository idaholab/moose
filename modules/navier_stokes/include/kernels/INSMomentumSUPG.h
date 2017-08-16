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
 * This class adds streamline upwind modifications to the test function,
 * important in advection dominated problems, e.g. when the grid Peclet
 * number is greater than 1. The alpha parameter is chosen in an optimal
 * way such that at least for a single-component, one dimensional advection-
 * diffusion equation, exact solutions at the nodes are achieved for the
 * entire range of grid Peclet numbers (0 - infinity). For references on
 * the subject, please see Zienkiewicz, et. al., FEM for Fluid Dynamics, 7th
 * ed., Chapter 2, as well as Computer Methods in Applied Mechanics and Engineering
 * 32 (1982) 199-259 by Hughes et. al.
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

  // Parameters
  RealVectorValue _gravity;
  unsigned _component;

  // Material properties
  const MaterialProperty<Real> & _mu;
  const MaterialProperty<Real> & _rho;
};

#endif
