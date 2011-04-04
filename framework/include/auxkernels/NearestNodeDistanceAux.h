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
#include "NearestNodeLocator.h"


//Forward Declarations
class NearestNodeDistanceAux;

template<>
InputParameters validParams<NearestNodeDistanceAux>();

/**
 * Constant auxiliary value
 */
class NearestNodeDistanceAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  NearestNodeDistanceAux(const std::string & name, InputParameters parameters);

  virtual ~NearestNodeDistanceAux() {}

protected:
  virtual Real computeValue();

  NearestNodeLocator & _nearest_node;
};

#endif //NEARESTNODEDISTANCEAUX_H
