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

#include "RandomHitUserObject.h"

template<>
InputParameters validParams<RandomHitUserObject>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addRequiredParam<unsigned int>("num_hits", "The number of 'hits' every timestep");
  params.addParam<unsigned int>("seed", 1, "The seed for the random number generator");
  return params;
}

RandomHitUserObject::RandomHitUserObject(const std::string & name, InputParameters parameters) :
    GeneralUserObject(name, parameters),
    _num_hits(parameters.get<unsigned int>("num_hits")),
    _hit_positions(_num_hits)
{
  _random.seed(parameters.get<unsigned int>("seed"));
}

bool
RandomHitUserObject::elementWasHit(const Elem * elem) const
{
  bool was_hit = false;
  for(unsigned int i=0; i<_num_hits; i++)
  {
    was_hit = elem->contains_point(_hit_positions[i]);
    if(was_hit)
      break;
  }

  return was_hit;
}

void
RandomHitUserObject::execute()
{
  for(unsigned int i=0; i<_num_hits; i++)
    _hit_positions[i] = Point(_random.rand(),_random.rand(),0);
}
