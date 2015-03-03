/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSSPLITMOMENTUM_H
#define INSSPLITMOMENTUM_H

#include "Kernel.h"

// Forward Declarations
class INSSplitMomentum;

template<>
InputParameters validParams<INSSplitMomentum>();

/**
 * This class computes the "split" momentum equation residual.  In the
 * split method, this is a time-independent vector equation for "a",
 * an intermediate "acceleration" vector.  The pressure is not coupled
 * directly to momentum in the split method.  Note: this equation is
 * divided through by the density, so "nu" appears rather than "mu", for
 * instance.
 *
 * Do not use, USE INSChorinPredictor and related classes instead.
 */
class INSSplitMomentum : public Kernel
{
public:
  INSSplitMomentum(const std::string & name, InputParameters parameters);

  virtual ~INSSplitMomentum(){}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Coupled variables
  VariableValue& _u_vel;
  VariableValue& _v_vel;
  VariableValue& _w_vel;

  // Acceleration vector components
  VariableValue& _a1;
  VariableValue& _a2;
  VariableValue& _a3;

  // Gradients
  VariableGradient& _grad_u_vel;
  VariableGradient& _grad_v_vel;
  VariableGradient& _grad_w_vel;

  // Variable numberings
  unsigned _u_vel_var_number;
  unsigned _v_vel_var_number;
  unsigned _w_vel_var_number;

  unsigned _a1_var_number;
  unsigned _a2_var_number;
  unsigned _a3_var_number;

  // Material properties
  Real _mu;
  Real _rho;
  RealVectorValue _gravity;

  // Parameters
  unsigned _component;
};


#endif // INSSPLITMOMENTUM_H
