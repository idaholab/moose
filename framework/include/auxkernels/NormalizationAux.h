//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NORMALIZATIONAUX_H
#define NORMALIZATIONAUX_H

#include "AuxKernel.h"

// Forward Declarations
class NormalizationAux;

template <>
InputParameters validParams<NormalizationAux>();

/**
 * This auxiliary kernel normalizes a variable based on a postprocessor.
 * Typically this postprocessor is a norm of the variable to be normalized.
 */
class NormalizationAux : public AuxKernel
{
public:
  NormalizationAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const VariableValue & _src;
  const Real & _pp_on_source;
  Real _normal_factor;
};

#endif // NORMALIZATIONAUX_H
