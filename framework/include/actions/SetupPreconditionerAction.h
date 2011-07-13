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

#ifndef SETUPPRECONDITIONERACTION_H
#define SETUPPRECONDITIONERACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "MooseObjectAction.h"

#include <string>

/**
 * Preconditioners are not normal "Moose Objects" but we are going to inherit from the
 * MooseObjectAction class to take advantage of the second set of parser filled parameters
 * afforded by that class.  We will then build the right Preconditioner "Action" to
 * setup the preconditioner using that second set of parameters
 */
class SetupPreconditionerAction : public MooseObjectAction
{
public:
  SetupPreconditionerAction(const std::string & name, InputParameters params);

  virtual void act();

  static std::string getTypeString(const std::string & action);
  
protected:
  static unsigned int _count;
  static std::map<std::string, std::string> _type_to_action;
  
  /**
   * This parameter tells us whether or not this instance is actually of this type or
   * not (one of its derived classes).  We do not need RTTI for this as this class is
   * responsible for creating instances of its derived classes so we can keep track of
   * it through a parameter.
   */
  bool _is_base_instance;
};

template<>
InputParameters validParams<SetupPreconditionerAction>();

#endif // SETUPPRECONDITIONERACTION_H
