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
  AccumulateAux(const std::string & name, InputParameters parameters);

  virtual ~AccumulateAux() {}

protected:
  virtual Real computeValue();

  VariableValue & _accumulate_from;
};

#endif //ACCUMULATEAUX_H
