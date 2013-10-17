/****************************************************************/
/*               Do NOT MODIFY THIS HEADER                      */
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

#ifndef RANDOMHITUSEROBJECT_H
#define RANDOMHITUSEROBJECT_H

#include "GeneralUserObject.h"

#include "MooseRandom.h"

//Forward Declarations
class RandomHitUserObject;

template<>
InputParameters validParams<RandomHitUserObject>();

class RandomHitUserObject :
  public GeneralUserObject
{
public:
  RandomHitUserObject(const std::string & name, InputParameters parameters);

  virtual ~RandomHitUserObject() {}

  /**
   * Returns true if the element was hit.
   * @param elem The element to be checked.
   */
  bool elementWasHit(const Elem * elem) const;

  /**
   * Returns the hits for this timestep.
   */
  const std::vector<Point> & hits() const { return _hit_positions; }

  /**
   * Called before execute() is ever called so that data can be cleared.
   */
  virtual void initialize(){}

  /**
   * Compute the hit positions for this timestep
   */
  virtual void execute();

  virtual void finalize(){}

protected:
  unsigned int _num_hits;
  std::vector<Point> _hit_positions;
  std::vector<unsigned int> _closest_node;
  MooseRandom _random;
};

#endif
