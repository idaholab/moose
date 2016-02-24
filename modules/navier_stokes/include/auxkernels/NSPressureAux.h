/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSPRESSUREAUX_H
#define NSPRESSUREAUX_H

#include "AuxKernel.h"

//Forward Declarations
class NSPressureAux;

template<>
InputParameters validParams<NSPressureAux>();

/**
 * Nodal auxiliary variable, for computing pressure at the nodes
 */
class NSPressureAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  NSPressureAux(const InputParameters & parameters);

  virtual ~NSPressureAux() {}

protected:
  virtual Real computeValue();

  const VariableValue & _rho;
  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;
  const VariableValue & _rhoe;

  Real _gamma;
};

#endif //VELOCITYAUX_H
