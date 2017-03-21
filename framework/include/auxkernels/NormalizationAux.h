/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
