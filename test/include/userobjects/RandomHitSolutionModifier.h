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

#ifndef RANDOMHITSOLUTIONMODIFIER_H
#define RANDOMHITSOLUTIONMODIFIER_H

// Moose
#include "GeneralUserObject.h"
#include "MooseMesh.h"

//Forward Declarations
class RandomHitUserObject;
class RandomHitSolutionModifier;

template<>
InputParameters validParams<RandomHitSolutionModifier>();

/* This class is here to combine the Postprocessor interface and the
 * base class Postprocessor object along with adding MooseObject to the inheritance tree*/
class RandomHitSolutionModifier :
  public GeneralUserObject
{
public:
  RandomHitSolutionModifier(const std::string & name, InputParameters parameters);

  virtual ~RandomHitSolutionModifier() {}

  /**
   * Called before execute() is ever called so that data can be cleared.
   */
  virtual void initialize(){}

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
  const RandomHitUserObject & _random_hits;

  std::vector<unsigned int> _nodes_that_were_hit;

  MooseMesh & _mesh;

  const PointLocatorBase & _point_locator;

  MooseVariable & _variable;

  Real _amount;
};

#endif
