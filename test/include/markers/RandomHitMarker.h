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

#ifndef RANDOMHITMARKER_H
#define RANDOMHITMARKER_H

#include "Marker.h"

class RandomHitUserObject;

class RandomHitMarker;
template<>
InputParameters validParams<RandomHitMarker>();

class RandomHitMarker : public Marker
{
public:
  RandomHitMarker(const std::string & name, InputParameters parameters);
  virtual ~RandomHitMarker(){};

protected:
  virtual MarkerValue computeElementMarker();

  const RandomHitUserObject & _random_hits;
};

#endif /* RANDOMHITMARKER_H */
