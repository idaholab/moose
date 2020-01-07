//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RandomHitMarker.h"

#include "RandomHitUserObject.h"

registerMooseObject("MooseTestApp", RandomHitMarker);

InputParameters
RandomHitMarker::validParams()
{
  InputParameters params = Marker::validParams();
  params.addRequiredParam<UserObjectName>(
      "random_hits", "The name of the UserObject to use for the positions of the random hits");
  return params;
}

RandomHitMarker::RandomHitMarker(const InputParameters & parameters)
  : Marker(parameters), _random_hits(getUserObject<RandomHitUserObject>("random_hits"))
{
}

Marker::MarkerValue
RandomHitMarker::computeElementMarker()
{
  if (_random_hits.elementWasHit(_current_elem))
    return REFINE;

  return DONT_MARK;
}
