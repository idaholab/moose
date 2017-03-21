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

#ifndef MOOSEOBJECTACTION_H
#define MOOSEOBJECTACTION_H

#include "Action.h"

#include <string>

class MooseObjectAction;

template <>
InputParameters validParams<MooseObjectAction>();

class MooseObjectAction : public Action
{
public:
  MooseObjectAction(InputParameters params);

  /**
   * Retreive the parameters of the object to be created by this action
   */
  InputParameters & getObjectParams() { return _moose_object_pars; }

  /**
   * Return the object type to be created
   */
  const std::string & getMooseObjectType() const { return _type; }

protected:
  /// The Object type that is being created
  std::string _type;

  /// The parameters for the object to be created
  InputParameters _moose_object_pars;
};

#endif // MOOSEOBJECTACTION_H
