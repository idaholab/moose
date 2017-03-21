/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSCHORINCORRECTOR_H
#define INSCHORINCORRECTOR_H

#include "Kernel.h"

// Forward Declarations
class INSChorinCorrector;

template <>
InputParameters validParams<INSChorinCorrector>();

/**
 * This class computes the "Chorin" Corrector equation in fully-discrete
 * (both time and space) form.
 */
class INSChorinCorrector : public Kernel
{
public:
  INSChorinCorrector(const InputParameters & parameters);

  virtual ~INSChorinCorrector() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // "Star" velocity components
  const VariableValue & _u_vel_star;
  const VariableValue & _v_vel_star;
  const VariableValue & _w_vel_star;

  // Pressure gradients
  const VariableGradient & _grad_p;

  // Variable numberings
  unsigned _u_vel_star_var_number;
  unsigned _v_vel_star_var_number;
  unsigned _w_vel_star_var_number;
  unsigned _p_var_number;

  // Material properties
  Real _rho;

  // Parameters
  unsigned _component;
};

#endif // INSCHORINCORRECTOR_H
