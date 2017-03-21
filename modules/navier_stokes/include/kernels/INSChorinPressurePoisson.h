/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSCHORINPRESSUREPOISSON_H
#define INSCHORINPRESSUREPOISSON_H

#include "Kernel.h"

// Forward Declarations
class INSChorinPressurePoisson;

template <>
InputParameters validParams<INSChorinPressurePoisson>();

/**
 * This class computes the pressure Poisson solve which is part of
 * the "split" scheme used for solving the incompressible Navier-Stokes
 * equations.
 */
class INSChorinPressurePoisson : public Kernel
{
public:
  INSChorinPressurePoisson(const InputParameters & parameters);

  virtual ~INSChorinPressurePoisson() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Gradients of the "star" velocity
  const VariableGradient & _grad_u_star;
  const VariableGradient & _grad_v_star;
  const VariableGradient & _grad_w_star;

  // Variable numberings
  unsigned _u_vel_star_var_number;
  unsigned _v_vel_star_var_number;
  unsigned _w_vel_star_var_number;

  // Material properties
  Real _rho;
};

#endif // INSCHORINPRESSUREPOISSON_H
