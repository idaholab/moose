/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSDIVERGENCEAUX_H
#define INSDIVERGENCEAUX_H

#include "AuxKernel.h"

//Forward Declarations
class INSDivergenceAux;

template<>
InputParameters validParams<INSDivergenceAux>();

/**
 * Computes h_min / |u|
 */
class INSDivergenceAux : public AuxKernel
{
public:
  INSDivergenceAux(const std::string & name, InputParameters parameters);

  virtual ~INSDivergenceAux() {}

protected:
  virtual Real computeValue();

  // Velocity gradients
  VariableGradient& _grad_u_vel;
  VariableGradient& _grad_v_vel;
  VariableGradient& _grad_w_vel;
};

#endif //VELOCITYAUX_H
