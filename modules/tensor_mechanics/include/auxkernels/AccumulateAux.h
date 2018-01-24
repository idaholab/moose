/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ACCUMULATEAUX_H
#define ACCUMULATEAUX_H

#include "AuxKernel.h"

// Forward Declarations
class AccumulateAux;

template <>
InputParameters validParams<AccumulateAux>();

/**
 * Accumulate values from one auxiliary variable into another
 */
class AccumulateAux : public AuxKernel
{
public:
  AccumulateAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  // coupled variable values to be aggregated
  std::vector<const VariableValue *> _values;
};

#endif // ACCUMULATEAUX_H
