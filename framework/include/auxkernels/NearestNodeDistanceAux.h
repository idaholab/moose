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
