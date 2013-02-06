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

#ifndef PERIODICDISTANCEAUX_H
#define PERIODICDISTANCEAUX_H

#include "AuxKernel.h"

class PeriodicDistanceAux;
class GeneratedMesh;

template<>
InputParameters validParams<PeriodicDistanceAux>();

/**
 * Aux kernel that tests periodic distance functions in GeneratedMesh
 */
class PeriodicDistanceAux : public AuxKernel
{
public:
  PeriodicDistanceAux(const std::string & name, InputParameters parameters);
  virtual ~PeriodicDistanceAux();

protected:
  virtual Real computeValue();

  /// A point of interest in the domain
  Point _point;
};


#endif /* PERIODICDISTANCEAUX_H */
