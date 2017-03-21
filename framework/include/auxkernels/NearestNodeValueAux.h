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

#ifndef NEARESTNODEVALUEAUX_H
#define NEARESTNODEVALUEAUX_H

#include "AuxKernel.h"

// Forward Declarations
class NearestNodeValueAux;
class NearestNodeLocator;

template <>
InputParameters validParams<NearestNodeValueAux>();

/**
 * Constant auxiliary value
 */
class NearestNodeValueAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  NearestNodeValueAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  NearestNodeLocator & _nearest_node;

  const NumericVector<Number> *& _serialized_solution;

  unsigned int _paired_variable;
};

#endif // NEARESTNODEVALUEAUX_H
