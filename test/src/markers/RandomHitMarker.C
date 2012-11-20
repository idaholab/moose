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

#include "RandomHitMarker.h"

#include "RandomHitUserObject.h"

template<>
InputParameters validParams<RandomHitMarker>()
{
  InputParameters params = validParams<Marker>();
  params.addRequiredParam<UserObjectName>("random_hits", "The name of the UserObject to use for the positions of the random hits");
  return params;
}


RandomHitMarker::RandomHitMarker(const std::string & name, InputParameters parameters) :
    Marker(name, parameters),
    _random_hits(getUserObject<RandomHitUserObject>("random_hits"))
{
}

Marker::MarkerValue
RandomHitMarker::computeElementMarker()
{
  if(_random_hits.elementWasHit(_current_elem))
    return REFINE;

  return DONT_MARK;
}

