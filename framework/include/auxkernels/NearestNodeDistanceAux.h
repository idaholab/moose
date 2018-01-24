//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NEARESTNODEDISTANCEAUX_H
#define NEARESTNODEDISTANCEAUX_H

#include "AuxKernel.h"

// Forward Declarations
class NearestNodeDistanceAux;
class NearestNodeLocator;

template <>
InputParameters validParams<NearestNodeDistanceAux>();

/**
 * Computes the distance from a block or boundary to another boundary.
 */
class NearestNodeDistanceAux : public AuxKernel
{
public:
  NearestNodeDistanceAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  NearestNodeLocator & _nearest_node;
};

#endif // NEARESTNODEDISTANCEAUX_H
