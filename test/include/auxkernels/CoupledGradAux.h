//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COUPLEDGRADAUX_H
#define COUPLEDGRADAUX_H

#include "AuxKernel.h"

// Forward Declarations
class CoupledGradAux;

template <>
InputParameters validParams<CoupledGradAux>();

/**
 * Coupled auxiliary gradient
 */
class CoupledGradAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  CoupledGradAux(const InputParameters & parameters);

  virtual ~CoupledGradAux();

protected:
  virtual Real computeValue();

  /// Gradient being set by this kernel
  RealGradient _grad;
  /// The number of coupled variable
  int _coupled;
  /// The value of coupled gradient
  const VariableGradient & _coupled_grad;
};

#endif // COUPLEDGRADAUX_H
