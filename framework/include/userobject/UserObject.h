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

#include "MooseObject.h"

class UserObject;

template<>
InputParameters validParams<UserObject>();


/**
 * Base class for user-specific data
 */
class UserObject : public MooseObject
{
public:
  UserObject(const std::string & name, InputParameters params);
  virtual ~UserObject();

  /**
   * Called before deleting the object. Free memory allocated by your derived classes, etc.
   */
  virtual void destroy() = 0;


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

};

#endif /* USEROBJECT_H */
