/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSVELOCITYAUX_H
#define NSVELOCITYAUX_H

#include "AuxKernel.h"

// Forward Declarations
class NSVelocityAux;

template <>
InputParameters validParams<NSVelocityAux>();

/**
 * Velocity auxiliary value
 */
class NSVelocityAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  NSVelocityAux(const InputParameters & parameters);

  virtual ~NSVelocityAux() {}

protected:
  virtual Real computeValue();

  const VariableValue & _rho;
  const VariableValue & _momentum;
};

#endif // VELOCITYAUX_H
