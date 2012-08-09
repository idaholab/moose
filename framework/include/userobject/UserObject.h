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

#ifndef USEROBJECT_H
#define USEROBJECT_H

//MOOSE includes
#include "Moose.h"
#include "MooseObject.h"
#include "SetupInterface.h"
#include "ParallelUniqueId.h"

//libMesh includes
#include "libmesh_common.h"
#include "parallel.h"

class UserObject;
class Problem;
class SubProblem;

template<>
InputParameters validParams<UserObject>();

/**
 * Base class for user-specific data
 */
class UserObject :
  public MooseObject,
  public SetupInterface
{
public:
  UserObject(const std::string & name, InputParameters params);
  virtual ~UserObject();

  /**
   * Called before execute() is ever called so that data can be cleared.
   */
  virtual void initialize() = 0;

  /**
   * Called before deleting the object. Free memory allocated by your derived classes, etc.
   */
  virtual void destroy() = 0;

  /**
   * Finalize.  This is called _after_ execute() and _after_ threadJoin()!  This is probably where you want to do MPI communication!
   */
  virtual void finalize() = 0;

  /**
   * Load user data object from a stream
   * @param stream Stream to load from
   */
  virtual void load(std::ifstream & stream);

  /**
   * Store user data object to a stream
   * @param stream Stream to store to
   */
  virtual void store(std::ofstream & stream);

  /**
   * Returns a reference to the subproblem that
   * this postprocessor is tied to
   */
  SubProblem & getSubProblem() const { return _subproblem; }

protected:
  SubProblem & _subproblem;
  FEProblem & _fe_problem;

  /// Thread ID of this postprocessor
  THREAD_ID _tid;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;
};

#endif /* USEROBJECT_H */
