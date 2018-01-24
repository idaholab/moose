//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NEARESTNODEVALUEAUX_H
#define NEARESTNODEVALUEAUX_H

#include "AuxKernel.h"

// Forward Declarations
class NearestNodeValueAux;
class NearestNodeLocator;

template <>
InputParameters validParams<NearestNodeValueAux>();

/**
 * Finds the closest node on a paired boundary to the current node or element and stores a
 * corresponding field value.
 */
class NearestNodeValueAux : public AuxKernel
{
public:
  NearestNodeValueAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  NearestNodeLocator & _nearest_node;

  const NumericVector<Number> *& _serialized_solution;

  unsigned int _paired_variable;
};

#endif // NEARESTNODEVALUEAUX_H
