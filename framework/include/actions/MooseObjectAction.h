//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

#include <string>

class MooseObjectAction : public Action
{
public:
  static InputParameters validParams();

  MooseObjectAction(const InputParameters & params);

  using Action::addRelationshipManagers;
  virtual void addRelationshipManagers(Moose::RelationshipManagerType when_type) override;

  /**
   * Retreive the parameters of the object to be created by this action
   */
  InputParameters & getObjectParams() { return _moose_object_pars; }

  /**
   * Constant version of retreiving the parameters of the object to be created by this action
   */
  const InputParameters & getObjectParams() const { return _moose_object_pars; }

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
