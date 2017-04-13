/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSSPECIFICVOLUMEAUX_H
#define NSSPECIFICVOLUMEAUX_H

#include "AuxKernel.h"

// Forward Declarations
class NSSpecificVolumeAux;

template <>
InputParameters validParams<NSSpecificVolumeAux>();

/**
 * Auxiliary kernel for computing the specific volume (1/rho) of the fluid.
 */
class NSSpecificVolumeAux : public AuxKernel
{
public:
  NSSpecificVolumeAux(const InputParameters & parameters);

  virtual ~NSSpecificVolumeAux() {}

protected:
  virtual Real computeValue();

  const VariableValue & _rho;
};

#endif
