//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SELFAUX_H
#define SELFAUX_H

#include "AuxKernel.h"

// Forward Declarations
class SelfAux;

template <>
InputParameters validParams<SelfAux>();

/**
 * Self auxiliary value
 */
class SelfAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  SelfAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;
};

#endif // SELFAUX_H
