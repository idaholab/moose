//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RandomHitUserObject.h"

registerMooseObject("MooseTestApp", RandomHitUserObject);

InputParameters
RandomHitUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<unsigned int>("num_hits", "The number of 'hits' every timestep");
  params.addParam<unsigned int>("seed", 1, "The seed for the random number generator");
  return params;
}

RandomHitUserObject::RandomHitUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _num_hits(parameters.get<unsigned int>("num_hits")),
    _hit_positions(_num_hits)
{
  _random.seed(parameters.get<unsigned int>("seed"));
}

bool
RandomHitUserObject::elementWasHit(const Elem * elem) const
{
  bool was_hit = false;
  for (unsigned int i = 0; i < _num_hits; i++)
  {
    was_hit = elem->contains_point(_hit_positions[i]);
    if (was_hit)
      break;
  }

  return was_hit;
}

void
RandomHitUserObject::execute()
{
  for (unsigned int i = 0; i < _num_hits; i++)
  {
    Real rand_x = _random.rand();
    Real rand_y = _random.rand();
    _hit_positions[i] = Point(rand_x, rand_y, 0);
  }
}
