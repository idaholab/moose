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

/* This class is here to combine the Postprocessor interface and the
 * base class Postprocessor object along with adding MooseObject to the inheritance tree*/
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
  virtual void initialize(){};

  /**
   * Compute the hit positions for this timestep
   */
  virtual void execute();

  /**
   * Called before deleting the object. Free memory allocated by your derived classes, etc.
   */
  virtual void destroy(){}

  /**
   * Finalize.  This is called _after_ execute() and _after_ threadJoin()!  This is probably where you want to do MPI communication!
   */
  virtual void finalize(){}

protected:
  unsigned int _num_hits;
  std::vector<Point> _hit_positions;
  std::vector<unsigned int> _closest_node;
  MooseRandom _random;
};

#endif
