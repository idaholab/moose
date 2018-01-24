//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
