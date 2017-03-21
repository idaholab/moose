/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSPRESSUREPOISSON_H
#define INSPRESSUREPOISSON_H

#include "Kernel.h"

// Forward Declarations
class INSPressurePoisson;

template <>
InputParameters validParams<INSPressurePoisson>();

/**
 * This class computes the pressure Poisson solve which is part of
 * the "split" scheme used for solving the incompressible Navier-Stokes
 * equations.
 *
 * Do not use, USE INSChorinPressurePoisson and related classes instead.
 */
class INSPressurePoisson : public Kernel
{
public:
  INSPressurePoisson(const InputParameters & parameters);

  virtual ~INSPressurePoisson() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Gradients of the accleration vector, 'a'
  const VariableGradient & _grad_a1;
  const VariableGradient & _grad_a2;
  const VariableGradient & _grad_a3;

  // Variable numberings
  unsigned _a1_var_number;
  unsigned _a2_var_number;
  unsigned _a3_var_number;

  // Material properties
  Real _rho;
};

#endif // INSPRESSUREPOISSON_H
