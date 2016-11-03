/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ACCUMULATEAUX_H
#define ACCUMULATEAUX_H

#include "AuxKernel.h"

//Forward Declarations
class AccumulateAux;

template<>
InputParameters validParams<AccumulateAux>();

/**
 * Accumulate values from one auxiliary variable into another
 */
class AccumulateAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  AccumulateAux(const InputParameters & parameters);

  virtual ~AccumulateAux() {}

protected:
  virtual Real computeValue();

  const VariableValue & _accumulate_from;
};

#endif //ACCUMULATEAUX_H
