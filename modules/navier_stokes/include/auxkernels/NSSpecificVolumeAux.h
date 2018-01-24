//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
