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

#ifndef USERDATA_H
#define USERDATA_H

#include "MooseObject.h"

class UserData;

template<>
InputParameters validParams<UserData>();


/**
 * Base class for user-specific data
 */
class UserData : public MooseObject
{
public:
  UserData(const std::string & name, InputParameters params);
  virtual ~UserData();

  /**
   * Called before deleting the object. Free memory allocated by your derived classes, etc.
   */
  virtual void destroy() = 0;
};

#endif /* USERDATA_H */
